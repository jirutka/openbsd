/* x509_trs.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/x509v3.h>


static int tr_cmp(X509_TRUST **a, X509_TRUST **b);
static void trtable_free(X509_TRUST *p);

static int trust_1oidany(X509_TRUST *trust, X509 *x, int flags);
static int trust_any(X509_TRUST *trust, X509 *x, int flags);

static int obj_trust(int id, X509 *x, int flags);
static int (*default_trust)(int id, X509 *x, int flags) = obj_trust;

/* WARNING: the following table should be kept in order of trust
 * and without any gaps so we can just subtract the minimum trust
 * value to get an index into the table
 */

static X509_TRUST trstandard[] = {
{X509_TRUST_ANY, 0, trust_any, "Any", 0, NULL},
{X509_TRUST_SSL_CLIENT, 0, trust_1oidany, "SSL Client", NID_client_auth, NULL},
{X509_TRUST_SSL_SERVER, 0, trust_1oidany, "SSL Client", NID_server_auth, NULL},
{X509_TRUST_EMAIL, 0, trust_1oidany, "S/MIME email", NID_email_protect, NULL},
};

#define X509_TRUST_COUNT	(sizeof(trstandard)/sizeof(X509_TRUST))

IMPLEMENT_STACK_OF(X509_TRUST)

static STACK_OF(X509_TRUST) *trtable = NULL;

static int tr_cmp(X509_TRUST **a, X509_TRUST **b)
{
	return (*a)->trust - (*b)->trust;
}

int (*X509_TRUST_set_default(int (*trust)(int , X509 *, int)))(int, X509 *, int)
{
int (*oldtrust)(int , X509 *, int);
oldtrust = default_trust;
default_trust = trust;
return oldtrust;
}


int X509_check_trust(X509 *x, int id, int flags)
{
	X509_TRUST *pt;
	int idx;
	if(id == -1) return 1;
	if(!(idx = X509_TRUST_get_by_id(id)))
			return default_trust(id, x, flags);
	pt = X509_TRUST_get0(idx);
	return pt->check_trust(pt, x, flags);
}

int X509_TRUST_get_count(void)
{
	if(!trtable) return X509_TRUST_COUNT;
	return sk_X509_TRUST_num(trtable) + X509_TRUST_COUNT;
}

X509_TRUST * X509_TRUST_get0(int idx)
{
	if(idx < 0) return NULL;
	if(idx < X509_TRUST_COUNT) return trstandard + idx;
	return sk_X509_TRUST_value(trtable, idx - X509_TRUST_COUNT);
}

int X509_TRUST_get_by_id(int id)
{
	X509_TRUST tmp;
	int idx;
	if((id >= X509_TRUST_MIN) && (id <= X509_TRUST_MAX))
				 return id - X509_TRUST_MIN;
	tmp.trust = id;
	if(!trtable) return -1;
	idx = sk_X509_TRUST_find(trtable, &tmp);
	if(idx == -1) return -1;
	return idx + X509_TRUST_COUNT;
}

int X509_TRUST_add(int id, int flags, int (*ck)(X509_TRUST *, X509 *, int),
					char *name, int arg1, void *arg2)
{
	int idx;
	X509_TRUST *trtmp;
	/* This is set according to what we change: application can't set it */
	flags &= ~X509_TRUST_DYNAMIC;
	/* This will always be set for application modified trust entries */
	flags |= X509_TRUST_DYNAMIC_NAME;
	/* Get existing entry if any */
	idx = X509_TRUST_get_by_id(id);
	/* Need a new entry */
	if(idx == -1) {
		if(!(trtmp = Malloc(sizeof(X509_TRUST)))) {
			X509err(X509_F_X509_TRUST_ADD,ERR_R_MALLOC_FAILURE);
			return 0;
		}
		trtmp->flags = X509_TRUST_DYNAMIC;
	} else trtmp = X509_TRUST_get0(idx);

	/* Free existing name if dynamic */
	if(trtmp->flags & X509_TRUST_DYNAMIC_NAME) Free(trtmp->name);
	/* dup supplied name */
	if(!(trtmp->name = BUF_strdup(name))) {
		X509err(X509_F_X509_TRUST_ADD,ERR_R_MALLOC_FAILURE);
		return 0;
	}
	/* Keep the dynamic flag of existing entry */
	trtmp->flags &= X509_TRUST_DYNAMIC;
	/* Set all other flags */
	trtmp->flags |= flags;

	trtmp->trust = id;
	trtmp->check_trust = ck;
	trtmp->arg1 = arg1;
	trtmp->arg2 = arg2;

	/* If its a new entry manage the dynamic table */
	if(idx == -1) {
		if(!trtable && !(trtable = sk_X509_TRUST_new(tr_cmp))) {
			X509err(X509_F_X509_TRUST_ADD,ERR_R_MALLOC_FAILURE);
			return 0;
		}
		if (!sk_X509_TRUST_push(trtable, trtmp)) {
			X509err(X509_F_X509_TRUST_ADD,ERR_R_MALLOC_FAILURE);
			return 0;
		}
	}
	return 1;
}

static void trtable_free(X509_TRUST *p)
	{
	if(!p) return;
	if (p->flags & X509_TRUST_DYNAMIC) 
		{
		if (p->flags & X509_TRUST_DYNAMIC_NAME)
			Free(p->name);
		Free(p);
		}
	}

void X509_TRUST_cleanup(void)
{
	int i;
	for(i = 0; i < X509_TRUST_COUNT; i++) trtable_free(trstandard + i);
	sk_X509_TRUST_pop_free(trtable, trtable_free);
	trtable = NULL;
}

int X509_TRUST_get_flags(X509_TRUST *xp)
{
	return xp->flags;
}

char *X509_TRUST_get0_name(X509_TRUST *xp)
{
	return xp->name;
}

int X509_TRUST_get_trust(X509_TRUST *xp)
{
	return xp->trust;
}

static int trust_1oidany(X509_TRUST *trust, X509 *x, int flags)
{
	if(x->aux) return obj_trust(trust->arg1, x, flags);
	/* we don't have any trust settings: for compatibility
	 * we return trusted if it is self signed
	 */
	X509_check_purpose(x, -1, 0);
	if(x->ex_flags & EXFLAG_SS) return X509_TRUST_TRUSTED;
	else return X509_TRUST_UNTRUSTED;
}

static int obj_trust(int id, X509 *x, int flags)
{
	ASN1_OBJECT *obj;
	int i;
	X509_CERT_AUX *ax;
	ax = x->aux;
	if(!ax) return X509_TRUST_UNTRUSTED;
	if(ax->reject) {
		for(i = 0; i < sk_ASN1_OBJECT_num(ax->reject); i++) {
			obj = sk_ASN1_OBJECT_value(ax->reject, i);
			if(OBJ_obj2nid(obj) == id) return X509_TRUST_REJECTED;
		}
	}	
	if(ax->trust) {
		for(i = 0; i < sk_ASN1_OBJECT_num(ax->trust); i++) {
			obj = sk_ASN1_OBJECT_value(ax->trust, i);
			if(OBJ_obj2nid(obj) == id) return X509_TRUST_TRUSTED;
		}
	}
	return X509_TRUST_UNTRUSTED;
}

static int trust_any(X509_TRUST *trust, X509 *x, int flags)
{
	return X509_TRUST_TRUSTED;
}
