/* $OpenBSD: src/usr.sbin/pkg_install/delete/Attic/delete.h,v 1.1 1996/06/04 07:56:07 niklas Exp $ */

/*
 * FreeBSD install - a package for the installation and maintainance
 * of non-core utilities.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * Jordan K. Hubbard
 * 18 July 1993
 *
 * Include and define various things wanted by the delete command.
 *
 */

#ifndef _INST_DELETE_H_INCLUDE
#define _INST_DELETE_H_INCLUDE

extern char	*Prefix;
extern Boolean	NoDeInstall;
extern Boolean	CleanDirs;
extern Boolean	Force;
extern char	*Directory;
extern char	*PkgName;

#endif	/* _INST_DELETE_H_INCLUDE */
