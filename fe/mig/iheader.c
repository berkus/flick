/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 *
 * This file is part of Flick, the Flexible IDL Compiler Kit.
 *
 * Flick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Flick is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <stdio.h>
#include <rpc/types.h>

#include <mom/compiler.h>
#include <mom/idl_id.h>
#include <mom/c/libcast.h>
#include <mom/pres_c.h>

/* #include "cpu.h" */
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "mom_routines.h"
#include "global.h"
#include "boolean.h"

char *ihead_name;

void make_iheader(void)
{
    FILE *ihead_file;

    statement_t *CurState;

    if ( (ihead_file = fopen(ihead_name,"wb")) == NULL)
	panic("Couldn't open %s",ihead_name);
    
    for (CurState = StatementList; CurState != NULL; CurState = 
							 CurState->stNext)
	{
	    if (CurState->stKind == skRoutine)
		{
		    fprintf(ihead_file,"#define %s %s_external\n",
			    CurState->stRoutine->rtName, 
			    CurState->stRoutine->rtName);
		}
	}

    fclose(ihead_file);
}
