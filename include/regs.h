#ifndef __REGS_H
#define __REGS_H

struct regs {
	u32 cpsr;
	union {
		u32 r[16];
		struct {
			u32 r0;
			u32 r1;
			u32 r2;
			u32 r3;
			u32 r4;
			u32 r5;
			u32 r6;
			u32 r7;
			u32 r8;
			u32 r9;
			u32 r10;
			u32 r11;
			u32 r12;
			u32 r13;
			u32 r14;
			u32 r15;
		};
		struct {
			u32 a1; /* r0 */
			u32 a2; /* r1 */
			u32 a3; /* r2 */
			u32 a4; /* r3 */
			u32 v1; /* r4 */
			u32 v2; /* r5 */
			u32 v3; /* r6 */
			u32 v4; /* r7 */
			u32 v5; /* r8 */
			union { /* r9 */
				u32 v6;
				u32 sb;
			};
			union {	/* r10  */
				u32 v7;
				u32 sl;
			};
			union {	/* r11  */
				u32 v8;
				u32 fp;
			};
			u32 ip; /* r12 */
			u32 sp; /* r13 */
			u32 lr; /* r14 */
			u32 pc; /* r15 */
		};
	};
};

#endif	/* __REGS_H */
