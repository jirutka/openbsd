/*	$OpenBSD: src/sys/arch/hp300/dev/hyperreg.h,v 1.2 2005/01/22 22:26:47 miod Exp $	*/
/*	$NetBSD: grf_hyreg.h,v 1.2 1994/10/26 07:23:57 cgd Exp $	*/

/*
 * Copyright (c) 1991 University of Utah.
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Mark Davies of the Department of Computer
 * Science, Victoria University of Wellington, New Zealand.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 * from: Utah $Hdr: grf_hyreg.h 1.1 92/01/22$
 *
 *	@(#)grf_hyreg.h	8.1 (Berkeley) 6/10/93
 */

struct hyboxfb {
	u_int8_t :8;
	u_int8_t reset;			/* reset register		0x01 */
	u_int8_t fb_address;		/* frame buffer address 	0x02 */
	u_int8_t interrupt;		/* interrupt register		0x03 */
	u_int8_t :8;
	u_int8_t fbwmsb;		/* frame buffer width MSB	0x05 */
	u_int8_t :8;
	u_int8_t fbwlsb;		/* frame buffer width MSB	0x07 */
	u_int8_t :8;
	u_int8_t fbhmsb;		/* frame buffer height MSB	0x09 */
	u_int8_t :8;
	u_int8_t fbhlsb;		/* frame buffer height MSB	0x0b */
	u_int8_t :8;
	u_int8_t dwmsb;			/* display width MSB		0x0d */
	u_int8_t :8;
	u_int8_t dwlsb;			/* display width MSB		0x0f */
	u_int8_t :8;
	u_int8_t dhmsb;			/* display height MSB		0x11 */
	u_int8_t :8;
	u_int8_t dhlsb;			/* display height MSB		0x13 */
	u_int8_t :8;
	u_int8_t fbid;			/* Scondary frame buffer id	0x15 */
	u_int8_t :8;
	u_int8_t bits;			/* square(0)/double-high(1) 	0x17 */
	u_int8_t f1[0x5b-0x17-1];
	u_int8_t num_planes;		/* number of color planes       0x5b */
	u_int8_t :8;
	u_int8_t fbomsb;		/* frame buffer offset MSB	0x5d */
	u_int8_t :8;
	u_int8_t fbolsb;		/* frame buffer offset LSB	0x5f */
	u_int8_t f2[0x4000-0x5f-1];
	u_int8_t nblank;		/* display enable planes      0x4000 */
};

/* Values for nblank */
#define	DISP_SYNC_ENABLE	0x01
#define	DISP_VIDEO_ENABLE	0x04
