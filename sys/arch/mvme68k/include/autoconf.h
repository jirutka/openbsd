/*	$OpenBSD: src/sys/arch/mvme68k/include/autoconf.h,v 1.3 1996/04/28 10:56:03 deraadt Exp $ */

/*
 * Copyright (c) 1995 Theo de Raadt
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
 *	This product includes software developed under OpenBSD by
 *	Theo de Raadt for Willowglen Singapore.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

struct confargs {
	int	ca_bustype;
	void	*ca_vaddr;
	void	*ca_paddr;
	int	ca_offset;
	int	ca_len;
	int	ca_ipl;
	int	ca_vec;
	char	*ca_name;

	void	*ca_master;	/* points to bus-dependent data */
};

#define BUS_MAIN	1
#define BUS_PCC		2	/* VME147 PCC chip */
#define BUS_MC		3	/* VME162 MC chip */
#define BUS_PCCTWO	4	/* VME166/167/177 PCC2 chip */
#define BUS_VMES	5	/* 16 bit VME access */
#define BUS_VMEL	6	/* 32 bit VME access */
#define BUS_IP		7	/* VME162 IP module bus */

caddr_t	mapiodev __P((caddr_t pa, int size));
void	unmapiodev __P((caddr_t kva, int size));
