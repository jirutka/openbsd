/*	$OpenBSD: src/lib/libform/frm_scale.c,v 1.1 1997/12/03 05:40:15 millert Exp $	*/

/*-----------------------------------------------------------------------------+
|           The ncurses form library is  Copyright (C) 1995-1997               |
|             by Juergen Pfeifer <Juergen.Pfeifer@T-Online.de>                 |
|                          All Rights Reserved.                                |
|                                                                              |
| Permission to use, copy, modify, and distribute this software and its        |
| documentation for any purpose and without fee is hereby granted, provided    |
| that the above copyright notice appear in all copies and that both that      |
| copyright notice and this permission notice appear in supporting             |
| documentation, and that the name of the above listed copyright holder(s) not |
| be used in advertising or publicity pertaining to distribution of the        |
| software without specific, written prior permission.                         | 
|                                                                              |
| THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO  |
| THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-  |
| NESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR   |
| ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RE- |
| SULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, |
| NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH    |
| THE USE OR PERFORMANCE OF THIS SOFTWARE.                                     |
+-----------------------------------------------------------------------------*/

#include "form.priv.h"

MODULE_ID("Id: frm_scale.c,v 1.1 1997/10/21 13:24:19 juergen Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int scale_form( const FORM *form, int *rows, int *cols )
|   
|   Description   :  Retrieve size of form
|
|   Return Values :  E_OK              - no error
|                    E_BAD_ARGUMENT    - invalid form pointer
|                    E_NOT_CONNECTED   - no fields connected to form
+--------------------------------------------------------------------------*/
int scale_form(const FORM * form, int * rows, int * cols)
{
  if ( !form )
    RETURN(E_BAD_ARGUMENT);

  if ( !(form->field) )
    RETURN(E_NOT_CONNECTED);
  
  if (rows) 
    *rows = form->rows;
  if (cols) 
    *cols = form->cols;
  
  RETURN(E_OK);
}

/* frm_scale.c ends here */
