/*	$OpenBSD: src/lib/libcurses/Attic/lib_beep.c,v 1.3 1997/12/03 05:21:12 millert Exp $	*/


/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*
 *	beep.c
 *
 *	The routine beep().
 *
 */

#include <curses.priv.h>
#include <term.h>	/* beep, flash */

MODULE_ID("Id: lib_beep.c,v 1.4 1997/10/08 09:38:17 jtc Exp $")

/*
 *	beep()
 *
 *	Sound the current terminal's audible bell if it has one.   If not,
 *	flash the screen if possible.
 *
 */

int beep(void)
{
	T((T_CALLED("beep()")));

	/* FIXME: should make sure that we are not in altchar mode */
	if (bell) {
		TPUTS_TRACE("bell");
		return(putp(bell));
	} else if (flash_screen) {
		TPUTS_TRACE("flash_screen");
		return(putp(flash_screen));
	}
	else
		returnCode(ERR);
}
