/*	$OpenBSD: src/libexec/talkd/table.c,v 1.2 1996/04/28 23:56:20 mickey Exp $	*/

/*
 * Copyright (c) 1983 Regents of the University of California.
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
/*static char sccsid[] = "from: @(#)table.c	5.7 (Berkeley) 2/26/91";*/
static char rcsid[] = "$Id: table.c,v 1.1.1.1 1995/10/18 08:43:23 deraadt Exp $";
#endif /* not lint */

/*
 * Routines to handle insertion, deletion, etc on the table
 * of requests kept by the daemon. Nothing fancy here, linear
 * search on a double-linked list. A time is kept with each 
 * entry so that overly old invitations can be eliminated.
 *
 * Consider this a mis-guided attempt at modularity
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <protocols/talkd.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "talkd.h"

#define MAX_ID 16000	/* << 2^15 so I don't have sign troubles */

#define NIL ((TABLE_ENTRY *)0)

struct	timeval tp;
struct	timezone txp;

typedef struct table_entry TABLE_ENTRY;

struct table_entry {
	CTL_MSG request;
	long	time;
	TABLE_ENTRY *next;
	TABLE_ENTRY *last;
};
TABLE_ENTRY	*table = NIL;

static void	delete __P((register TABLE_ENTRY *));

/*
 * Look in the table for an invitation that matches the current
 * request looking for an invitation
 */
CTL_MSG *
find_match(request)
	register CTL_MSG *request;
{
	register TABLE_ENTRY *ptr;
	time_t current_time;

	gettimeofday(&tp, &txp);
	current_time = tp.tv_sec;
	if (debug)
		print_request("find_match", request);
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if ((ptr->time - current_time) > MAX_LIFE) {
			/* the entry is too old */
			if (debug)
				print_request("deleting expired entry",
				    &ptr->request);
			delete(ptr);
			continue;
		}
		if (debug)
			print_request("", &ptr->request);
		if (strcmp(request->l_name, ptr->request.r_name) == 0 &&
		    strcmp(request->r_name, ptr->request.l_name) == 0 &&
		     ptr->request.type == LEAVE_INVITE)
			return (&ptr->request);
	}
	if (debug)
		syslog(LOG_DEBUG, "find_match: not found\n");

	return ((CTL_MSG *)0);
}

/*
 * Look for an identical request, as opposed to a complimentary
 * one as find_match does 
 */
CTL_MSG *
find_request(request)
	register CTL_MSG *request;
{
	register TABLE_ENTRY *ptr;
	time_t current_time;

	gettimeofday(&tp, &txp);
	current_time = tp.tv_sec;
	/*
	 * See if this is a repeated message, and check for
	 * out of date entries in the table while we are it.
	 */
	if (debug)
		print_request("find_request", request);
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if ((ptr->time - current_time) > MAX_LIFE) {
			/* the entry is too old */
			if (debug)
				print_request("deleting expired entry",
				    &ptr->request);
			delete(ptr);
			continue;
		}
		if (debug)
			print_request("", &ptr->request);
		if (strcmp(request->r_name, ptr->request.r_name) == 0 &&
		    strcmp(request->l_name, ptr->request.l_name) == 0 &&
		    request->type == ptr->request.type &&
		    request->pid == ptr->request.pid) {
			/* update the time if we 'touch' it */
			ptr->time = current_time;
			return (&ptr->request);
		}
	}
	return ((CTL_MSG *)0);
}

void
insert_table(request, response)
	CTL_MSG *request;
	CTL_RESPONSE *response;
{
	register TABLE_ENTRY *ptr;
	time_t current_time;

	if (debug)
		print_request( "insert_table", request );
	gettimeofday(&tp, &txp);
	current_time = tp.tv_sec;
	request->id_num = new_id();
	response->id_num = htonl(request->id_num);
	/* insert a new entry into the top of the list */
	ptr = (TABLE_ENTRY *)malloc(sizeof(TABLE_ENTRY));
	if (ptr == NIL) {
		syslog(LOG_ERR, "insert_table: Out of memory");
		_exit(1);
	}
	ptr->time = current_time;
	ptr->request = *request;
	ptr->next = table;
	if (ptr->next != NIL)
		ptr->next->last = ptr;
	ptr->last = NIL;
	table = ptr;
}

/*
 * Generate a unique non-zero sequence number
 */
int
new_id(void)
{
	static int current_id = 0;

	current_id = (current_id + 1) % MAX_ID;
	/* 0 is reserved, helps to pick up bugs */
	if (current_id == 0)
		current_id = 1;
	return (current_id);
}

/*
 * Delete the invitation with id 'id_num'
 */
int
delete_invite(id_num)
	int id_num;
{
	register TABLE_ENTRY *ptr;

	ptr = table;
	if (debug)
		syslog(LOG_DEBUG, "delete_invite(%d)", id_num);
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if (ptr->request.id_num == id_num)
			break;
		if (debug)
			print_request("", &ptr->request);
	}
	if (ptr != NIL) {
		delete(ptr);
		return (SUCCESS);
	}
	return (NOT_HERE);
}

/*
 * Classic delete from a double-linked list
 */
static void
delete(ptr)
	register TABLE_ENTRY *ptr;
{

	if (debug)
		print_request("delete", &ptr->request);
	if (table == ptr)
		table = ptr->next;
	else if (ptr->last != NIL)
		ptr->last->next = ptr->next;
	if (ptr->next != NIL)
		ptr->next->last = ptr->last;
	free((char *)ptr);
}
