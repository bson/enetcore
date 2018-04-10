// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "network.h"
#include "ip.h"
#include "dhcp.h"


EventObject _net_event;

namespace BufferPool {

static Vector<IOBuffer*> _pool;

void Initialize(uint num, uint size)
{
    Thread::IPL G(IPL_NETWORK);

	_pool.Reserve(num);
	for (uint i = 0; i < num; ++i) {
		IOBuffer* buf = AllocNetworkBuffer();
        uint8_t* mem = AllocNetworkData(size, 4);
		buf->SetMem(mem, size);
		buf->SetSize(size);
		buf->SetAutoCompact(false);
		buf->SetAutoResize(false); // Trap on attempts to grow buffer
		_pool.PushBack(buf);
	}
}


static IOBuffer* Alloc()
{
	if (_pool.Empty())
        return NULL;

	IOBuffer* buf = _pool.Back();
	_pool.PopBack();

	buf->SetHead(0);
	buf->SetTail(buf->GetReserve());

	return buf;
}


IOBuffer* AllocRx()
{
    Thread::IPL G(IPL_NETWORK);

	return Alloc();
}


IOBuffer* AllocTx()
{
    Thread::IPL G(IPL_NETWORK);

	if (_pool.Size() <= TX_MIN_POOL)
        return NULL;

	return Alloc();
}


void FreeBuffer(IOBuffer* buf)
{
    Thread::IPL G(IPL_NETWORK);

	_pool.PushBack(buf);
}


} // namespace BufferPool


// XXX move this stuff to IP

const Vector<InterfaceInfo*>& GetNetworkInterfaces()
{
	static Vector<InterfaceInfo*> iflist;

	if (iflist.Empty()) {
#if 0
		InterfaceInfo* info = Ip::GetIfInfo(STR("eth0"));
		assert(info);
		iflist.PushBack(info);
#endif
	}

	return iflist;
}


bool GetIfMacAddr(const String& interface, uint8_t macaddr[6])
{
	if (interface.Equals(STR("eth0"))) {
		memcpy(macaddr, _eth0.GetMacAddr(), 6);
		return true;
	}

	return false;
}


bool GetMacAddr(uint8_t macaddr[6]) { return GetIfMacAddr(STR("eth0"), macaddr); }


#ifdef SOFT_PHY_ADDR
static uint16_t _mac0;          // First 2 bytes
static uint32_t _mac1;          // Last 4 bytes
#endif

void InitEthernet() {
    uint8_t macaddr[8];
    Platform::GetMacAddr(macaddr+2);

	_eth0.Initialize(macaddr+2);
    _eth0.SetEventObject(&_net_event);

#ifdef SOFT_PHY_ADDR
    macaddr[0] = macaddr[1] = 0;
    _mac0 = *(uint16_t*)(macaddr + 2);
    _mac1 = *(uint32_t*)(macaddr + 4);
#endif
}

void* NetThread(void*)
{
    Thread::SetPriority(NET_THREAD_PRIORITY);

	BufferPool::Initialize(NumNetworkBuffers(_eth0.GetBufSize()), _eth0.GetBufSize());

    InitEthernet();

	_ip0.Initialize();
	_dhcp0.Reset();
    _dns0.Init();

	Time dhcp_next = _dhcp0.GetServiceTime();
	Time ip_next = _ip0.GetServiceTime();
	bool link = _eth0.GetLinkStatus();
    console("eth0: link %s", link ? "up" : "down");

	for (;;) {
		Time now = Time::Now();
		Time next = min(dhcp_next, ip_next);
		if (!link)
            next = min(next, now + Time::FromMsec(250));

		if (now < next)
			_net_event.Wait(next - now);

        if (_eth0.MISRAtten())
            _eth0.ServiceMISR();

        const bool newlink = _eth0.GetLinkStatus();
        if (newlink != link) {
            link = newlink;
            console("eth0: link %s", link ? "up" : "down");
        }

		IOBuffer* packet;
		uint16_t et;
		while ((packet = _eth0.Receive(et))) {
#ifdef SOFT_PHY_ADDR
            const uint16_t mac0 = *(uint16_t*)(*packet + 0);
            const uint32_t mac1 = *(uint32_t*)(*packet + 2);
            if ((mac0 == 0xffff && mac1 == 0xffffffff) || 
                (mac0 == _mac0 && mac1 == _mac1)) {
#endif
                switch (et) {
                case ETHERTYPE_IP:
                    if (!_dhcp0.Receive(packet))
                        _ip0.Receive(packet);
                    break;
                case ETHERTYPE_ARP:
                    _ip0.ArpReceive(packet);
                    break;
                default: ;
                }
#ifdef SOFT_PHY_ADDR
            }
#endif
			BufferPool::FreeBuffer(packet);
		}

		now = Time::Now();

		if (now >= dhcp_next)
            dhcp_next = _dhcp0.Service();
        
		if (now >= ip_next)
            ip_next = _ip0.Service();
	}
	// Notreached
	return NULL;
}
