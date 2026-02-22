// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_HASH_H__
#define __STM32_HASH_H__


class Stm32Hash: public Stm32Dma::Peripheral {

public:
    enum class Register {
        CR   = 0x00,
        DIN  = 0x04,             // Data in
        STR  = 0x08,             // Start
        HRA0 = 0x0c,             // HRx aliased to HRAx for x=0-4
        HRA1 = 0x10,           
        HRA2 = 0x14,
        HRA3 = 0x18,
        HRA4 = 0x1c,
        IMR  = 0x20,
        SR   = 0x24,
        CSR0 = 0xf8,              // Context swap, 0-53 (0xf8 + x * 4)
        HR0  = 0x310,             // Hash digest word 0-4
        HR1  = 0x314,             //   MD5: 0-3
        HR2  = 0x318,             //   SHA1: 0-4
        HR3  = 0x31c,             //   SHA2-224: 0-6
        HR4  = 0x320,             //   SHA2-256 0-7
        HR5  = 0x324,
        HR6  = 0x328,
        HR7  = 0x32c,
    };

    enum {
        // CR
        ALGO1 = 18,
        LKEY = 16,
        MDMAT = 13,
        DINNE = 12,
        NBW = 8,
        ALGO0 = 7,
        MODE = 6,               // 0=hash. 1=HMAC
        DATATYPE = 4,           // 0=uint32_t, 1=uint16_t, 2=uint8_t, 3=bit string (32-bit reverse bit order)
        DMAE = 3,
        INIT = 2,

        // STR
        DCAL = 8,               // Digest calc
        NBLW = 0,               // Number of valid bits in the last word (0=all 32)

        // IMR
        DCIE = 1,               // Digest calc complete IE
        DINIE = 0,              // Data input IE

        // SR
        BUSY = 3,               // Processing block of data
        DMAS = 2,
        DCIS = 1,               // Digest calc intr status
        DINIS = 0,              // Data input intr status
    };

    enum Algorithm {
        SHA1 =     0,
        MD5 =      1,
        SHA2_224 = 2,           // SHA2-224
        SHA2_256 = 3            // SHA2-256
    };

    volatile uint32_t& reg(const Register r) {
        return *((volatile uint32_t*)(BASE_HASH + (uint32_t)r)); 
    }


private:
    typedef Ring<SEND_BUF_SIZE, uint32_t> SendQ;

    mutable Mutex   _w_mutex;
    SendQ           _sendq;

    // For DMA
    Stm32Dma*       _dma;       // TX DMA is enabled if this != NULL
    uint16_t        _tx_size;   // Current DMA TX length
    bool            _ienable;   // Enable interrupts

public:
    // RX DMA simply copies TX as a placeholder since it's never used
    Stm32Hash(Stm32Dma::Target txtarg, uint32_t streams)
        : Peripheral(base + (uint32_t)Register::DIN, 
                     base + (uint32_t)Register::DIN,
                     txtarg, txtarg, streams),
          _dma(NULL) {
    }


    // Start digest.  If key is provided, calculate HMAC, otherwise hash
    // data only.  keylen is in uint32_t's.  Len should be 0 and key
    // NULL if no key is supplied.
    // Fills in digest of length len (words).
    void Calculate(uint32_t *digest, const int len,
                   Algorithm algo = MD5, const uint32_t* key = NULL, int keylen = 0) {
        assert(len >= 4 && len <= 8);

        volatile uint32_t& cr  = reg(Register::CR);
        volatile uint32_t& sr  = reg(Register::SR);
        volatile uint32_t& str = reg(Register::STR);

        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_HASH);

        cr = Bitfield(cr)
            .cbit(INIT)
            .bit(MODE, keylen != 0)
            .bit(LKEY, keylen && (keylen * 4 <= 64)) // Present and 64 bytes or less
            .bit(ALGO1, (uint32_t)algo & BIT(1))
            .bit(ALGO0, (uint32_t)algo & BIT(0))
            .f(2, DATATYPE, 0);          // Data is 32-bit words
        
        cr |= BIT(INIT);

        if (_dma && !_tx._active)
            _dma->AssignTx(this);

        StartTx();

        while (!_send.Empty())
            Thread::WaitFor((void*)&_sendq);

        // Calculate
        str |= BIT(DCAL);

        while ((sr & BIT(DCIS)) == 0)
            Thread::WaitFor((void*)&_sendq);

        const uint32_t* hr = (const uint32_t*)(BASE_HASH + HR0);
        while (len--)
            *digest++ = *hdr++;

        // If someone else is waiting, kick em off
        Thread::WakeSingle((void*)&_sendq);
    }

    
    // Write data.  len is the number of words.
    // Use this to fill up the buffer, followed by Calculate().
    void Write(const uint32_t* data, uint len) {
        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_HASH);
    
        for (const uint32_t *p = data; p < data + len; ) {
            assert(_sendq.Headroom());

            if (_sendq.Headroom()) {
                while (_sendq.Headroom() && (p < data + len))
                    _sendq.PushBack(*p++);
        }
    }


    void EnableDmaTx(Stm32Dma& dma, uint8_t stream, Stm32Dma::Priority prio) {
        assert(stream <= 7);

        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_HASH);

        _dma = &dma;
        Peripheral::_ipl = IPL_HASH;
        Peripheral::_prio = prio;
        Peripheral::_word_size = Stm32Dma::WordSize::WORD32;
        Peripheral::_tx._active = false;
        Peripheral::_trbuff = true;

        SetInterrupts(_ienable);
    }


	// Enable interrupts
    void SetInterrupts(bool enable) {
        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);

        _ienable = enable;

        volatile uint32_t& imr = reg(Register::IMR);
        if (enable) {
            imr = Bitfield(imr)
                .bit(DINIE, !_dma)
                .bit(DCIE);
        } else {
            imr = Bitfield(imr)
                .cbit(DINIE)
                .cbit(DCIE);
        }
    }

	// Interrupt handler
	static void Interrupt(void* token) {
        ((Stm32Hash*)token)->HandleInterrupt();
    }

    // DMA TX complete
    void DmaTxComplete() {
        _sendq.Clear();
        StartTx();              // Housekeeping
        Thread::WakeSingle((void*)&_sendq);
    }

    // NYI
    void DmaRxComplete() { }

    // Enable disable DMA (attach, detach trigger)
    void DmaEnableTx() {
        reg(Register::CR) |= BIT(DMAE);
    }

    void DmaDisableTx() {
        reg(Register::CR) &= ~BIT(DMAE);
    }

    // Dummy; RX DMA is not used for HASH
    void DmaEnableRx() { }
    void DmaDisableRx() { }

private:
    inline void StartTx() {
        volatile uint32_t& imr = reg(Register::IMR);
        volatile uint32_t& sr  = reg(Register::SR);
        volatile uint32_t& din = reg(Register::DIN);

        if (_dma) {
            if (!_sendq.Empty()) {
                if (!_tx._active)
                    _dma->Transmit(this, _sendq.Buffer(), (_tx_size = _sendq.Continuous()), true);
            } else {
                _sendq.Clear();     // Normalize
                _dma->ReleaseTx(this);
            }

            // Just in case we got here on a DIN interrupt, right after enabling DMA
            imr = Bitfield(imr)
                .cbit(DINIE);

            return;
        }
            
        if (sr & BIT(DINIS)) {
            if (!_sendq.Empty()) {
                while (!_send.Empty() && (((sr >> NBW) & 0xf) != 0xf))
                    din = _sendq.PopFront();
                    
                imr = Bitfield(imr)
                    .bit(DINIE, _enable)
                    .bit(DCIE, _enable);
            }
        }
    }


    inline void HandleInterrupt() {
        volatile uint32_t& sr  = reg(Register::SR);

        if (sr & BIT(DCIS)) {
            Thread::WakeSingle((void*)&_sendq);
        } else if (sr & BIT(DINIS)) {
            StartTx();
            if (_sendq.Empty())
                Thread::WakeSingle((void*)&_sendq);
        }
    }
};

#endif // __HASH_H__
