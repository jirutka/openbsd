/*	$OpenBSD: src/games/monop/roll.c,v 1.2 1998/09/20 23:36:56 pjanzen Exp $	*/
/*	$NetBSD: roll.c,v 1.5 1995/03/23 08:35:13 cgd Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)roll.c	8.1 (Berkeley) 5/31/93";
#else
static char rcsid[] = "$OpenBSD: src/games/monop/roll.c,v 1.2 1998/09/20 23:36:56 pjanzen Exp $";
#endif
#endif /* not lint */

#include <stdlib.h>

/*
 *	This routine rolls ndie nside-sided dice.
 */

#if defined(pdp11)
#define	MAXRAND	32767L

int
roll(ndie, nsides)
	int	ndie, nsides;
{
	long		tot;
	unsigned	n, r;

	tot = 0;
	n = ndie;
	while (n--)
		tot += random();
	return (int) ((tot * (long) nsides) / ((long) MAXRAND + 1)) + ndie;
}

#else

int
roll(ndie, nsides)
	int	ndie, nsides;
{
	int	tot, r;
	double	num_sides;

	num_sides = nsides;
	tot = 0;
	while (ndie--)
		tot += (r = random()) * (num_sides / RAND_MAX) + 1;
	return tot;
}
#endif
