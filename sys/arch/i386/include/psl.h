/*	$NetBSD: psl.h,v 1.22 1995/10/11 04:20:23 mycroft Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)psl.h	5.2 (Berkeley) 1/18/91
 */

#ifndef _I386_PSL_H_
#define _I386_PSL_H_

/*
 * 386 processor status longword.
 */
#define	PSL_C		0x00000001	/* carry flag */
#define	PSL_PF		0x00000004	/* parity flag */
#define	PSL_AF		0x00000010	/* auxiliary carry flag */
#define	PSL_Z		0x00000040	/* zero flag */
#define	PSL_N		0x00000080	/* sign flag */
#define	PSL_T		0x00000100	/* trap flag */
#define	PSL_I		0x00000200	/* interrupt enable flag */
#define	PSL_D		0x00000400	/* direction flag */
#define	PSL_V		0x00000800	/* overflow flag */
#define	PSL_IOPL	0x00003000	/* i/o privilege level */
#define	PSL_NT		0x00004000	/* nested task */
#define	PSL_RF		0x00010000	/* resume flag */
#define	PSL_VM		0x00020000	/* virtual 8086 mode */
#define	PSL_AC		0x00040000	/* alignment check flag */
#define	PSL_VIF		0x00080000	/* virtual interrupt enable flag */
#define	PSL_VIP		0x00100000	/* virtual interrupt pending flag */
#define	PSL_ID		0x00200000	/* identification flag */

#define	PSL_MBO		0x00000002	/* must be one bits */
#define	PSL_MBZ		0xffc08028	/* must be zero bits */

#define	PSL_USERSET	(PSL_MBO | PSL_I)
#ifdef VM86
#define	PSL_USERSTATIC	(PSL_MBO | PSL_MBZ | PSL_I | PSL_IOPL | PSL_VIF | PSL_VIP)
#else
#define	PSL_USERSTATIC	(PSL_MBO | PSL_MBZ | PSL_I | PSL_IOPL | PSL_VM | PSL_VIF | PSL_VIP)
#endif

#ifdef _KERNEL

/* Interrupt priority `levels'; not mutually exclusive. */
#define	IPL_NONE	-1
#define	IPL_BIO		0	/* block I/O */
#define	IPL_NET		1	/* network */
#define	IPL_TTY		2	/* terminal */
#define	IPL_CLOCK	3	/* clock */
#define	IPL_IMP		4	/* memory allocation */

/* Interrupt sharing types. */
#define	IST_NONE	0	/* none */
#define	IST_PULSE	1	/* pulsed */
#define	IST_EDGE	2	/* edge-triggered */
#define	IST_LEVEL	3	/* level-triggered */

/* Soft interrupt masks. */
#define	SIR_CLOCK	31
#define	SIR_CLOCKMASK	((1 << SIR_CLOCK))
#define	SIR_NET		30
#define	SIR_NETMASK	((1 << SIR_NET) | SIR_CLOCKMASK)
#define	SIR_TTY		29
#define	SIR_TTYMASK	((1 << SIR_TTY) | SIR_CLOCKMASK)
#define	SIR_ALLMASK	(SIR_CLOCKMASK | SIR_NETMASK | SIR_TTYMASK)

#ifndef LOCORE

int cpl, ipending, astpending, imask[5];

/*
 * Add a mask to cpl, and return the old value of cpl.
 */
static __inline int
splraise(ncpl)
	register int ncpl;
{
	register int ocpl = cpl;
	cpl |= ncpl;
	return (ocpl);
}

extern void spllower __P((void));

/*
 * Restore a value to cpl (unmasking interrupts).  If any unmasked
 * interrupts are pending, call spllower() to process them.
 *
 * NOTE: We go to the trouble of returning the old value of cpl for
 * the benefit of some splsoftclock() callers.  This extra work is
 * usually optimized away by the compiler.
 */
static __inline int
splx(ncpl)
	register int ncpl;
{
	register int ocpl = cpl;
	cpl = ncpl;
	if (ipending & ~ncpl)
		spllower();
	return (ocpl);
}

/*
 * Hardware interrupt masks
 */
#define	splbio()	splraise(imask[IPL_BIO])
#define	splnet()	splraise(imask[IPL_NET])
#define	spltty()	splraise(imask[IPL_TTY])
#define	splclock()	splraise(imask[IPL_CLOCK])
#define	splimp()	splraise(imask[IPL_IMP])
#define	splstatclock()	splclock()

/*
 * Software interrupt masks
 *
 * NOTE: splsoftclock() is used by hardclock() to lower the priority from
 * clock to softclock before it calls softclock().
 */
#define	splsoftclock()	splx(SIR_CLOCKMASK)
#define	splsoftnet()	splraise(SIR_NETMASK)
#define	splsofttty()	splraise(SIR_TTYMASK)

/*
 * Miscellaneous
 */
#define	splhigh()	splraise(-1)
#define	spl0()		splx(0)

/*
 * Software interrupt registration
 */
#define	softintr(n)	(ipending |= (1 << (n)))
#define	setsoftast()	(astpending = 1)
#define	setsoftclock()	softintr(SIR_CLOCK)
#define	setsoftnet()	softintr(SIR_NET)
#define	setsofttty()	softintr(SIR_TTY)

#endif /* !LOCORE */
#endif /* _KERNEL */

#endif /* !_I386_PSL_H_ */
