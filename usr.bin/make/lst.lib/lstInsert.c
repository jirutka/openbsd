/*	$OpenPackages$ */
/*	$OpenBSD: src/usr.bin/make/lst.lib/lstInsert.c,v 1.14 2001/05/29 12:53:46 espie Exp $	*/
/*	$NetBSD: lstInsert.c,v 1.5 1996/11/06 17:59:44 christos Exp $	*/

/*
 * Copyright (c) 1988, 1989, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
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

/*-
 * LstInsert.c --
 *	Insert a new datum before an old one
 */

#include "lstInt.h"
#include <sys/types.h>
#include <stdlib.h>
#include "memory.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Insert --
 *	Insert a new node with the given piece of data before the given
 *	node in the given list.
 *
 * Side Effects:
 *	the firstPtr field will be changed if ln is the first node in the
 *	list.
 *
 *-----------------------------------------------------------------------
 */
void
Lst_Insert(l, ln, d)
    Lst 		l;	/* list to manipulate */
    LstNode		ln;	/* node before which to insert d */
    void		*d;	/* datum to be inserted */
{
    LstNode		nLNode; /* new lnode for d */


    if (ln == NULL && !Lst_IsEmpty(l))
	return;

    if (ln != NULL && Lst_IsEmpty(l))
	return;

    PAlloc(nLNode, LstNode);

    nLNode->datum = d;

    if (ln == NULL) {
	nLNode->prevPtr = nLNode->nextPtr = NULL;
	l->firstPtr = l->lastPtr = nLNode;
    } else {
	nLNode->prevPtr = ln->prevPtr;
	nLNode->nextPtr = ln;

	if (nLNode->prevPtr != NULL)
	    nLNode->prevPtr->nextPtr = nLNode;
	ln->prevPtr = nLNode;

	if (ln == l->firstPtr)
	    l->firstPtr = nLNode;
    }
}

void
Lst_AtFront(l, d)
    Lst 	l;
    void	*d;
{
    LstNode	ln;

    PAlloc(ln, LstNode);
    ln->datum = d;

    ln->nextPtr = l->firstPtr;
    ln->prevPtr = NULL;
    if (l->firstPtr == NULL)
	l->lastPtr = ln;
    else
	l->firstPtr->prevPtr = ln;
    l->firstPtr = ln;
}
