#ifndef __UART_H
#define __UART_H

#include <inttypes.h>

/* TRM p.4459 */
#define TX_FIFO_FULL	(1<<0)
#define TX_FIFO_E	(1<<5)
#define RX_FIFO_E	(1<<0)
#define RHR_IT		(1<<0)
#define IT_PENDING	(1<<0)
#define IT_TYPE_RHR	(2<<1)

/* TRM p.4444 */
struct uart {
	union {			/* 0x00 */
		u32 thr;
		u32 rhr;
		u32 dll;
	};
	union {			/* 0x04 */
		u32 ier;
		u32 dlh;
	};
	union {			/* 0x08 */
		u32 iir;
		u32 fcr;
		u32 efr;
	};
	u32 lcr;		/* 0x0c */
	union {			/* 0x10 */
		u32 mcr;
		u32 xon1_addr1;
	};
	union {			/* 0x14 */
		u32 lsr;
		u32 xon2_addr2;
	};
	union {			/* 0x18 */
		u32 tcr;
		u32 msr;
		u32 xoff1;
	};
	union {			/* 0x1c */
		u32 spr;
		u32 tlr;
		u32 xoff2;
	};
	u32 mdr1;		/* 0x20 */
	u32 mdr2;		/* 0x24 */
	union {			/* 0x28 */
		u32 sflsr;
		u32 txfll;
	};
	union {			/* 0x2c */
		u32 resume;
		u32 txflh;
	};
	union {			/* 0x30 */
		u32 sfregl;
		u32 rxfll;
	};
	union {			/* 0x34 */
		u32 sfregh;
		u32 rxflh;
	};
	union {			/* 0x38 */
		u32 blr;
		u32 uasr;
	};
	u32 acreg;		/* 0x3c */
	u32 scr;		/* 0x40 */
	u32 ssr;		/* 0x44 */
	u32 eblr;		/* 0x48 */
	u32 __pad;		/* 0x4c */
	u32 mvr;		/* 0x50 */
	u32 sysc;		/* 0x54 */
	u32 syss;		/* 0x58 */
	u32 wer;		/* 0x5c */
	u32 cfps;		/* 0x60 */
	u32 rxfifo_lvl;		/* 0x64 */
	u32 txfifo_lvl;		/* 0x68 */
	u32 ier2;		/* 0x6c */
	u32 isr2;		/* 0x70 */
	u32 freq_sel;		/* 0x74 */
	u32 __pad2[2];		/* 0x78 */
	u32 mdr3;		/* 0x80 */
	u32 tx_dma_threshold;	/* 0x84 */
};

#endif	/* __UART_H */
