/*	$OpenBSD: src/lib/libc/gen/isnormal.c,v 1.4 2008/12/10 01:15:02 martynas Exp $	*/
/*
 * Copyright (c) 2008 Martynas Venckus <martynas@openbsd.org>
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

#include <sys/types.h>
#include <sys/cdefs.h>
#include <machine/ieee.h>
#include <float.h>
#include <math.h>

int
__isnormal(double d)
{
	struct ieee_double *p = (struct ieee_double *)&d;

	return (p->dbl_exp != 0 && p->dbl_exp != DBL_EXP_INFNAN);
}

int
__isnormalf(float f)
{
	struct ieee_single *p = (struct ieee_single *)&f;

	return (p->sng_exp != 0 && p->sng_exp != SNG_EXP_INFNAN);
}

#if LDBL_MANT_DIG == 53
#ifdef __weak_alias
__weak_alias(__isnormall, __isnormal);
#endif /* __weak_alias */
#endif /* LDBL_MANT_DIG == 53 */
