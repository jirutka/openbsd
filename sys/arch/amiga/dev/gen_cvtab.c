/*	$OpenBSD: src/sys/arch/amiga/dev/Attic/gen_cvtab.c,v 1.1 1996/02/26 21:08:03 niklas Exp $	*/
/*	$NetBSD: gen_cvtab.c,v 1.1 1996/02/24 20:13:00 chopps Exp $	*/

/*
 * Copyright (c) 1996 Christian E. Hopps
 * All rights reserved.
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
 *      This product includes software developed by Christian E. Hopps.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/types.h>
#include <stdio.h>

/*
 * generate a conversion table to replace the use of the
 * floating point expression below in grf_cv.c
 */
int
main(int argc, char** argv)
{
	u_long vals[31];
	u_long clock;
	int m = 33;
	int lastm = -1;

	for (clock = 0; clock < 1080 && m > 0; ++clock) {
		m = ((55 * .72 + 16.867) * 89.736 / (clock + 39) - 21.1543);	
		m = m / 2 - 1;
		if (m > 31)
			continue;
		if (m != lastm)  {
			vals[m] = clock;
			lastm = m;
		}
	}
	printf("/* generated by gen_cvtab */\n");
	printf("static int cv_convtab[31] = {");
	for (m = 0; m < 31; ++m) {
		if (m % 5 == 0)
			printf("\n");
		printf("\t%ld,", vals[m]);
	}
	printf("\n};\n");
}
