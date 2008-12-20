/*
 * Copyright (c) 1999 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include "mach3mig-link.h"

/*
 * XXX - The CORBA C mapping doesn't offer inheritance, and the
 * release flag is not specified as a member of a sequence
 * structure, so the only way to ensure the runtime functions
 * for getting and setting the release flag work properly is if
 * the release flag is at a known offset from the beginning of
 * the structure.  For optimal compatibility, we can't assume
 * that the release flag is part of the normal (defined)
 * sequence type (such as at offset 0), since the user may wish
 * to view/set it's internals solely by offset (or overlaying a
 * standard sequence type over ours).  Thus, we put the release
 * flag as the fourth member of the structure.
 *
 * XXX - Note that runtime/headers/flick/pres/corba.h currently
 * predefines two sequence types (necessary for the runtime).
 * THESE MUST CONFORM TO THE SAME LAYOUT AS GIVEN HERE!!
 * See pfe/libcorba/p_variable_array_type.cc for more info.
 */
void CORBA_sequence_set_release(void *seq, CORBA_boolean rel)
{
	((CORBA_sequence_string *) seq)->_release = rel;
}

CORBA_boolean CORBA_sequence_get_release(void *seq)
{
	return ((CORBA_sequence_string *) seq)->_release;
}

/* End of file. */

