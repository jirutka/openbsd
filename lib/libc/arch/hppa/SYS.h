/*	$OpenBSD: src/lib/libc/arch/hppa/SYS.h,v 1.1 1998/08/28 20:59:41 mickey Exp $	*/

/*
 * Copyright (c) 1998 Michael Shalayeff
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
 *	This product includes software developed by Michael Shalayeff.
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

#include <sys/syscall.h>
#include <machine/asm.h>
#include <machine/vmparam.h>

#define	RSYSCALL(x)		!\
ENTRY(x)			!\
	stw rp, FM_ERP(sr0,sp)	!\
	ldil L%SYSCALLGATE, r1	!\
	ble 4(sr7, r1)		!\
	ldi __CONCAT(SYS_,x),r22!\
	or,<> r0,r22,r0		!\
	ldw FM_ERP(sr0,sp),rp	!\
EXIT(x)

#define	PSEUDO(x,y)		!\
ENTRY(x)			!\
	stw rp, FM_ERP(sr0,sp)	!\
	ldil L%SYSCALLGATE, r1	!\
	ble 4(sr7, r1)		!\
	ldi __CONCAT(SYS_,y),r22!\
	or,<> r0,r22,r0		!\
	ldw FM_ERP(sr0,sp),rp	!\
EXIT(x)

