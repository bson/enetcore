	
/* ARM Core Cortex-M7 system control */
CPUID = 0xe000ed00;
ACTLR = 0xe000e008;
ICSR = 0xe000ed04;
VTOR = 0xe000ed08;
AIRCR = 0xe000ed0c;
SCR = 0xe000ed10;
CCR = 0xe000ed14;
SHPR1 = 0xe000ed18;
SHPR2 = 0xe000ed1c;
SHPR3 = 0xe000ed20;
SHCRS = 0xe000ed24;
CFSR = 0xe000ed28;
MMFSR = 0xe000ed28;
BFSR = 0xe000ed29;
UFSR = 0xe000ed2a;
HFSR = 0xe000ed2c;
MMAR = 0xe000ed34;
BFAR = 0xe000ed38;
AFSR = 0xe000ed3c;
CPACR = 0xe000ed88;
FPCCR = 0xe000ef34;

/* ARM Processor features
CLIDR = 0xe000ed78;
CTR = 0xe000ed7c;
CCSIDR = 0xe000ed80;
CCSELR = 0xe000ed84;

/* ARM cache maintenance and access control */
ICIALLU = 0xe000ef50;
ICIMVAU = 0xe000ef58;
DCIMVAC = 0xe000ef5c;
DCISW = 0xe000ef60;
DCCMVAU = 0xe000ef64;
DCCMVAC = 0xe000ef68;
DCCSW = 0xe000ef6c;
DCCIMVAC = 0xe000ef70;
DCCISW = 0xe000ef74;
ITCMCR = 0xe000ef90;
DTCMCR = 0xe000ef94;
AHBPCR = 0xe000ef98;
CACR = 0xe000ef9c;
AHBSCR = 0xe000efa0;
ABFSR = 0xe000efa8;

/* ARM Core NVIC */
ISER0 = 0xe000e100;
ISER1 = 0xe000e104;
ICER0 = 0xe000e180;
ICER1 = 0xe000e184;
ISPR0 = 0xe000e200;
ISPR1 = 0xe000e204;
ICPR0 = 0xe000e280;
ICPR1 = 0xe000e284;
IABR0 = 0xe000e300;
IABR1 = 0xe000e304;
IPR0 = 0xe000e400;
IPR1 = 0xe000e404;
IPR2 = 0xe000e408;
IPR3 = 0xe000e40c;
IPR4 = 0xe000e410;
IPR5 = 0xe000e414;
IPR6 = 0xe000e418;
IPR7 = 0xe000e41c;
IPR8 = 0xe000e420;
IPR9 = 0xe000e424;
IPR10 = 0xe000e428;
STIR = 0xe000ef00;

/* ARM Core MPU */
MPU_TYPE = 0xe000ed90;
MPU_CTRL =  0xe000ed94;
MPU_RNR =  0xe000ed98;
MPU_RBAR =  0xe000ed9c;
MPU_RASR =  0xe000eda0;
MPU_RBAR_A1 =  0xe000eda4;
MPU_RASR_A1 = 0xe000eda8;
MPU_RBAR_A2 = 0xe000edac;
MPU_RASR_A2 = 0xe000edb0;
MPU_RBAR_A3 = 0xe000edb4;
MPU_RASR_A3 = 0xe000edb8;

/* ARM Core SYSTICK */
SYST_CSR = 0xe000e010;
SYST_RVR = 0xe000e014;
SYST_CVR = 0xe000e018;
SYST_CALIB = 0xe000e01c;

/* ITM */
ITM_LAR  = 0xe0000fb0; /* LOCK, write 0xc5acce55 to unlock ITM regs */
ITM_TCR  = 0xe0000e80; /* Trace control */
ITM_TPR  = 0xe0000e40; /* Trace privilege */
ITM_TER  = 0xe0000e00; /* Trace enable */
ITM_STIM0 = 0xe0000000; /* 32x 32-bit trace ports 0-31 */

/* TPIU */
TPIU_SSPSR = 0xe0040000;
TPIU_CSPSR = 0xe0040004;
TPIU_ACPR  = 0xe0040010;
TPIU_SPPR  = 0xe00400f0;
TPIU_FFMT  = 0xe0040304;
TPIU_FFSR  = 0xe0040300; /* Formatter and flush status */
TPIU_FFCR  = 0xe0040304; /* Formatter and flush control */
TPIU_FSCR  = 0xe0040308; /* Formatter synchronization counter */

TPIU_TRIGGER     = 0xe0040ee8; /* TRIGGER register (RO, reset: 0x0) */
TPIU_FIFO_data_0 = 0xe0040eec; /* Integration ETM Data (RO, reset: 0x--000000) */
TPIU_ITATBCTR2   = 0xe0040ef0; /* ITATBCTR2 (RO, reset: 0x0) */
TPIU_FIFO_data_1 = 0xe0040efc; /* Integration ITM Data (RO, reset: 0x--000000) */
TPIU_ITATBCTR0   = 0xe0040ef8; /* ITATBCTR0 (RO, reset: 0x0) */
TPIU_ITCTRL      = 0xe0040f00; /* Integration Mode Control, TPIU_ITCTRL (RW, reset: 0x0) */
TPIU_CLAIMSET    = 0xe0040fa0; /* Claim tag set (RW, reset: 0xf) */
TPIU_CLAIMCLR    = 0xe0040fa4; /* Claim tag clear (RW, reset: 0x0) */
TPIU_DEVID       = 0xe0040fc8; /* TPIU_DEVID (RO, reset: 0xca0/0xca1) */
TPIU_DEVTYPE     = 0xe0040fcc; /* TPIU_DEVTYPE (RO, reset: 0x11) */

/* DEBUG */
DBG_DEMCR = 0xe000edfc;
DBG_CR    = 0xe0042004;
