/*	$OpenBSD: src/sys/arch/vax/mbus/fwioreg.h,v 1.1 2008/08/18 23:19:25 miod Exp $	*/

/*
 * Copyright (c) 2008 Miodrag Vallat.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Firefox Workstation I/O Module registers and memory layout
 */

#define	FWIO_SSC_REMAP_ADDR	0x21400000

#define	FWIO_SII_REG_OFFSET	0x000000
#define	FWIO_LANCE_REG_OFFSET	0x200000
#define	FWIO_SII_BUF_OFFSET	0x400000
#define	FWIO_SII_BUF_SIZE	0x020000
#define	FWIO_DZ_REG_OFFSET	0x600000
#define	FWIO_IOCSR_OFFSET	0x800000
#define	FWIO_ESAR_OFFSET	0x800000
#define	FWIO_LANCE_BUF_OFFSET	0xa00000
#define	FWIO_LANCE_BUF_SIZE	0x020000

/*
 * IOCSR bits
 */

#define	FWIO_IOCSR_CNSL		0x80000000 /* break on line 3 asserts MHALT */
#define	FWIO_IOCSR_MRUN		0x40000000 /* assert MRUN */
#define	FWIO_IOCSR_CLKIEN	0x20000000 /* drive MCLKI */
#define	FWIO_IOCSR_RSTWS	0x10000000 /* reset workstation */

#define	FWIO_ESAR_MASK		0x00ff0000
#define	FWIO_ESAR_SHIFT		16
