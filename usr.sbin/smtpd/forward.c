/*	$OpenBSD: src/usr.sbin/smtpd/forward.c,v 1.17 2009/11/08 23:08:56 gilles Exp $	*/

/*
 * Copyright (c) 2008 Gilles Chehade <gilles@openbsd.org>
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
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <event.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smtpd.h"

int
forwards_get(int fd, struct expandtree *expandtree)
{
	FILE *fp;
	struct alias alias;
	char *buf, *lbuf, *p, *cp;
	size_t len;
	size_t nbaliases = 0;
	int quoted;
	struct expand_node *expnode;

	fp = fdopen(fd, "r");
	if (fp == NULL)
		return 0;

	lbuf = NULL;
	while ((buf = fgetln(fp, &len))) {
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		else {
			/* EOF without EOL, copy and add the NUL */
			if ((lbuf = malloc(len + 1)) == NULL)
				fatal("malloc");
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}

		/* ignore empty lines and comments */
		if (buf[0] == '#' || buf[0] == '\0')
			continue;

		quoted = 0;
		cp = buf;
		do {
			/* skip whitespace */
			while (isspace((int)*cp))
				cp++;

			/* parse line */
			for (p = cp; *p != '\0'; p++) {
				if (*p == ',' && !quoted) {
					*p++ = '\0';
					break;
				} else if (*p == '"')
					quoted = !quoted;
			}
			buf = cp;
			cp = p;

			if (! alias_parse(&alias, buf)) {
				log_debug("bad entry in ~/.forward");
				continue;
			}

			if (alias.type == EXPAND_INCLUDE) {
				log_debug(
				    "includes are forbidden in ~/.forward");
				continue;
			}

			expnode = calloc(sizeof(struct expand_node), 1);
			if (expnode == NULL)
				fatal("calloc");
			expnode->type = alias.type;
			expnode->u = alias.u;
			expandtree_insert(expandtree, expnode);
			nbaliases++;
		} while (*cp != '\0');
	}
	free(lbuf);
	fclose(fp);
	return (nbaliases);
}
