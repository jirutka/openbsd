/*
 * Copyright (c) 1997-2003 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

#include "der_locl.h"

RCSID("$KTH: der_length.c,v 1.16 2004/02/07 14:27:59 lha Exp $");

size_t
_heim_len_unsigned (unsigned val)
{
    size_t ret = 0;

    do {
	++ret;
	val /= 256;
    } while (val);
    return ret;
}

size_t
_heim_len_int (int val)
{
    unsigned char q;
    size_t ret = 0;

    if (val >= 0) {
	do {
	    q = val % 256;
	    ret++;
	    val /= 256;
	} while(val);
	if(q >= 128)
	    ret++;
    } else {
	val = ~val;
	do {
	    q = ~(val % 256);
	    ret++;
	    val /= 256;
	} while(val);
	if(q < 128)
	    ret++;
    }
    return ret;
}

static size_t
len_oid (const heim_oid *oid)
{
    size_t ret = 1;
    int n;

    for (n = 2; n < oid->length; ++n) {
	unsigned u = oid->components[n];

	++ret;
	u /= 128;
	while (u > 0) {
	    ++ret;
	    u /= 128;
	}
    }
    return ret;
}

size_t
length_len (size_t len)
{
    if (len < 128)
	return 1;
    else
	return _heim_len_unsigned (len) + 1;
}

size_t
length_boolean (const int *data)
{
  return 1 + length_len(1) + 1;
}

size_t
length_integer (const int *data)
{
    size_t len = _heim_len_int (*data);

    return 1 + length_len(len) + len;
}

size_t
length_unsigned (const unsigned *data)
{
    unsigned val = *data;
    size_t len = 0;
 
    while (val > 255) {
	++len;
	val /= 256;
    }
    len++;
    if (val >= 128)
	len++;
    return 1 + length_len(len) + len;
}

size_t
length_enumerated (const unsigned *data)
{
    size_t len = _heim_len_int (*data);

    return 1 + length_len(len) + len;
}

size_t
length_general_string (const heim_general_string *data)
{
    char *str = *data;
    size_t len = strlen(str);
    return 1 + length_len(len) + len;
}

size_t
length_octet_string (const heim_octet_string *k)
{
    return 1 + length_len(k->length) + k->length;
}

size_t
length_oid (const heim_oid *k)
{
    size_t len = len_oid (k);

    return 1 + length_len(len) + len;
}

size_t
length_generalized_time (const time_t *t)
{
    heim_octet_string k;
    size_t ret;

    time2generalizedtime (*t, &k);
    ret = 1 + length_len(k.length) + k.length;
    free (k.data);
    return ret;
}
