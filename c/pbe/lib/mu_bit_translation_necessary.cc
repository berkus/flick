/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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

#include <assert.h>

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * Return statements that specify whether or not we need to do bit translation.
 * This general case returns #if 1, #else, and #endif, indicating that byte
 * swapping may (always) be necessary, and optimizations such as bcopy could
 * be invalid.
 *
 * THIS SHOULD BE SPECIALIZED ACCORDING TO EACH BACK END'S ABILITY TO OPTIMIZE.
 *
 * The parameter specifies whether to output the if, else or endif
 * (0, 1, 2 respectively)
 */

static const char *if_text
	= "#if 1 /* is bit translation necessary? - should be specialized */";
static const char *else_text
	= "#else /* bit translation */";
static const char *endif_text
	= "#endif /* bit translation */";

cast_stmt mu_state::mu_bit_translation_necessary(int which,
						 mint_ref /*itype*/)
{
	cast_stmt macro = 0;
	
	switch (which) {
	case 0:
		// #if
		macro = cast_new_text(if_text);
		break;
	case 1:
		// #else
		macro = cast_new_text(else_text);
		break;
	case 2:
		// #endif
		macro = cast_new_text(endif_text);
		break;
	default:
		panic(("Invalid value passed to "
		       "`mu_bit_translation_necessary'"));
		break;
	}
	
	return macro;
}

/* End of file. */

