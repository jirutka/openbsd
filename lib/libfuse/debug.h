/*	$OpenBSD: src/lib/libfuse/debug.h,v 1.1 2013/06/03 16:00:50 tedu Exp $	*/
/*
 * Copyright (c) 2008 Alexandre Ratchov <alex@caoua.org>
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
#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG
#include <stdio.h>

#define DPRINTFN(n, ...)					\
	do {							\
		if (ifuse_debug >= (n))				\
			fprintf(stderr, __VA_ARGS__);		\
	} while(0)

#define DPRINTF(...)						\
	do {							\
		if (ifuse_debug > 0)				\
			fprintf(stderr, __VA_ARGS__);		\
	} while(0)

#define DPERROR(s)						\
	do {							\
		if (ifuse_debug > 0)				\
			perror(s);				\
	} while(0)

void ifuse_debug_init(void);
extern int ifuse_debug;
#else
#define DPRINTF(...) do {} while(0)
#define DPRINTFN(...) do {} while(0)
#define DPERROR(s) do {} while(0)
#endif

#endif /* _DEBUG_H_ */
