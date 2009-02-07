/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

/*
 * ANY CHANGES TO THIS FILE MUST BE MADE TO `corba-excepts.c' IN ALL LINK LIBS!
 * IT IS IDENTICAL IN ALL LINK LIBS --- EXCEPT THAT THE INCLUDED LINK HEADER
 * DIFFERS FROM ONE RUNTIME TO THE NEXT.
 */

/*
 * This file contains functions that are required when using CORBA
 * presentation.
 */

#include <flick/link/suntcp.h>
#include <flick/pres/corba.h>

/*****************************************************************************/

/*
 * We allocate CORBA system exception structures out of a static array, called
 * the `system_exception_buffer'.  This allows us, for instance, to produce a
 * CORBA_NO_MEMORY exception without needing to allocate memory for the
 * exception structure at runtime.
 */

#define FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE (16)

static int
system_exception_slot_used[FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE]
	= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static flick_system_exception
system_exception_buffer[FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE];

/* Index of the last-allocated table element. */
static int
system_exception_index = 0;

/*
 * Allocate a system exception structure from the table of exception structs.
 */
flick_system_exception *
flick_system_exception_alloc()
{
	int i;
	
	for (;;) {
		for (i = 1; (i <= FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE); i++)
			if (!system_exception_slot_used[
				((system_exception_index + i)
				 % FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE)]) {
				goto found;
			}
		/* XXX --- sleep, wait for slot to open up. */
	}
	
  found:
	system_exception_index = (system_exception_index + i)
				 % FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE;
	
	system_exception_slot_used[system_exception_index] = 1;
	
	return &(system_exception_buffer[system_exception_index]);
}

/*
 * Free a system exception structure.
 */
void
flick_system_exception_free(void *system_except)
{
	int i;
	
	if ((((flick_system_exception *) system_except)
	     < &(system_exception_buffer[0]))
	    || (((flick_system_exception *) system_except)
		> &(system_exception_buffer[FLICK_SYSTEM_EXCEPTION_BUFFER_SIZE
					   - 1]))
		) {
		/*
		 * `system_except' doesn't point into `system_exception_buffer'
		 * so warn and return.
		 */
		fprintf(stderr,
			"Warning: "
			"attempt to free invalid system exception.\n");
		return;
	}
	
	i = ((flick_system_exception *) system_except)
	    - system_exception_buffer;
	
	system_exception_slot_used[i] = 0;
}


/*****************************************************************************/

/*
 * Just as we keep a table of system exception structures, we also keep a table
 * of system exception identifiers (names).
 */

/*
 * Return a CORBA-runtime-managed string that is equal to `name'; i.e., intern
 * the `name' and return the interned version.
 *
 * The returned string may be statically allocated; it may be returned many
 * times by different invocations of this function; it may not be modified or
 * freed by the caller.
 */
char *
find_system_exception_id(char *name, int size)
{
        static char **table = 0;
      	static int count = 0, max = 0;
	int i;
	
	/*
	 * If `name' matches a known CORBA system exception, return a
	 * statically allocated string.
	 */
#define if_return(exname) if(!strcmp(name, ex_##exname)) return ex_##exname
	
        if_return(CORBA_UNKNOWN);
        if_return(CORBA_BAD_PARAM);
        if_return(CORBA_NO_MEMORY);
        if_return(CORBA_IMP_LIMIT);
        if_return(CORBA_COMM_FAILURE);
        if_return(CORBA_INV_OBJREF);
        if_return(CORBA_NO_PERMISSION);
        if_return(CORBA_INTERNAL);
        if_return(CORBA_MARSHAL);
        if_return(CORBA_INITIALIZE);
        if_return(CORBA_NO_IMPLEMENT);
        if_return(CORBA_BAD_TYPECODE);
        if_return(CORBA_BAD_OPERATION);
        if_return(CORBA_NO_RESOURCES);
        if_return(CORBA_NO_RESPONSE);
        if_return(CORBA_PERSIST_STORE);
        if_return(CORBA_BAD_INV_ORDER);
        if_return(CORBA_TRANSIENT);
        if_return(CORBA_FREE_MEM);
        if_return(CORBA_INV_IDENT);
        if_return(CORBA_INV_FLAG);
        if_return(CORBA_INTF_REPOS);
        if_return(CORBA_BAD_CONTEXT);
        if_return(CORBA_OBJ_ADAPTER);
        if_return(CORBA_DATA_CONVERSION);
        if_return(CORBA_OBJECT_NOT_EXIST);
        if_return(CORBA_TRANSACTION_REQUIRED);
        if_return(CORBA_TRANSACTION_ROLLEDBACK);
        if_return(CORBA_INVALID_TRANSACTION);
	
	/*
	 * Otherwise, we must search our table and possibly add a copy of
	 * `name' to the table.
	 */
	for (i = 0; i < count; i++)
                if (!strcmp(table[i], name)) return table[i];
	
	/* Add a copy of `name' to the table. */
	if (count >= max) {
		int new_max = max + 10;
		char **new_table = t_realloc(table, char *, new_max);
		
		if (!new_table) {
		        fprintf(stderr,
				"Error: can't allocate space for "
				"system exception identifier.\n");
			/* XXX --- Is this a reasonable thing to do? */
			return ex_CORBA_NO_MEMORY;
		}
		
		table = new_table;
		max = new_max;
	}
	
	table[count] = t_malloc(char, size);
	if (!table[count]) {
		fprintf(stderr,
			"Error: can't allocate space for "
			"system exception identifier.\n");
		/* XXX --- Is this a reasonable thing to do? */
		return ex_CORBA_NO_MEMORY;
	}
	
	i = count++;
	strncpy(table[i], name, size);
	
	return table[i];
}


/*****************************************************************************/

/*
 * Here are functions for translating between Flick's internal error codes and
 * CORBA exception identifiers.
 */

/*
 * Translate a Flick internal error code into a CORBA system exception id.
 *
 * At one time, this error --> id translation was implemented as a macro in
 * <flick/pres/corba.h>: since the `errval' argument is always known at compile
 * time, the CORBA system exception id can be determined at compile time, too.
 * But it's not a big win to have this be a macro; exceptions are supposed to
 * be exceptional.
 */

char *
flick_error_to_CORBA_exception_id(int errval)
{
	switch (errval) {
	default:				return ex_CORBA_UNKNOWN;
	case FLICK_ERROR_CONSTANT:		return ex_CORBA_MARSHAL;
	case FLICK_ERROR_VIRTUAL_UNION:		return ex_CORBA_UNKNOWN;
	case FLICK_ERROR_STRUCT_UNION:		return ex_CORBA_BAD_PARAM;
	case FLICK_ERROR_DECODE_SWITCH:		return ex_CORBA_BAD_OPERATION;
	case FLICK_ERROR_COLLAPSED_UNION:	return ex_CORBA_BAD_OPERATION;
	case FLICK_ERROR_VOID_UNION:		return ex_CORBA_UNKNOWN;
	case FLICK_ERROR_COMMUNICATION:		return ex_CORBA_COMM_FAILURE;
	case FLICK_ERROR_OUT_OF_BOUNDS:		return ex_CORBA_BAD_PARAM;
	case FLICK_ERROR_INVALID_TARGET:	return ex_CORBA_INV_OBJREF;
	case FLICK_ERROR_NO_MEMORY:             return ex_CORBA_NO_MEMORY;
	}
}


/*****************************************************************************/

/*
 * This section contains the CORBA-specified functions for managing exceptions.
 */

/*
 * Define the allocators for the standard CORBA exceptions.
 */
#define sys_except__alloc(name) \
name *name##__alloc() { return (name *) flick_system_exception_alloc(); }

sys_except__alloc(CORBA_UNKNOWN)
sys_except__alloc(CORBA_BAD_PARAM)
sys_except__alloc(CORBA_NO_MEMORY)
sys_except__alloc(CORBA_IMP_LIMIT)
sys_except__alloc(CORBA_COMM_FAILURE)
sys_except__alloc(CORBA_INV_OBJREF)
sys_except__alloc(CORBA_NO_PERMISSION)
sys_except__alloc(CORBA_INTERNAL)
sys_except__alloc(CORBA_MARSHAL)
sys_except__alloc(CORBA_INITIALIZE)
sys_except__alloc(CORBA_NO_IMPLEMENT)
sys_except__alloc(CORBA_BAD_TYPECODE)
sys_except__alloc(CORBA_BAD_OPERATION)
sys_except__alloc(CORBA_NO_RESOURCES)
sys_except__alloc(CORBA_NO_RESPONSE)
sys_except__alloc(CORBA_PERSIST_STORE)
sys_except__alloc(CORBA_BAD_INV_ORDER)
sys_except__alloc(CORBA_TRANSIENT)
sys_except__alloc(CORBA_FREE_MEM)
sys_except__alloc(CORBA_INV_IDENT)
sys_except__alloc(CORBA_INV_FLAG)
sys_except__alloc(CORBA_INTF_REPOS)
sys_except__alloc(CORBA_BAD_CONTEXT)
sys_except__alloc(CORBA_OBJ_ADAPTER)
sys_except__alloc(CORBA_DATA_CONVERSION)
sys_except__alloc(CORBA_OBJECT_NOT_EXIST)
sys_except__alloc(CORBA_TRANSACTION_REQUIRED)
sys_except__alloc(CORBA_TRANSACTION_ROLLEDBACK)
sys_except__alloc(CORBA_INVALID_TRANSACTION)

#undef sys_except__alloc

/*
 * CORBA::ORB::InvalidName is a user exception raised by `resolve_initial_
 * references'; see Section 17.29 of the CORBA 2.1 spec.
 */
CORBA_ORB_InvalidName *
CORBA_ORB_InvalidName__alloc()
{
	/*
	 * A `CORBA::ORB::InvalidName' has no exception data, but the mapped C
	 * structure contains one (dummy) field, because C structures must have
	 * at least one member.
	 *
	 * But: presumably, if one of these is allocated, the reason is so that
	 * it may be passed to `CORBA_BOA_set_exception'.  But Section 17.27.2
	 * of the CORBA 2.1 specification says that if the exception has no
	 * members, then the passed pointer-to-exception-struct must be null.
	 *
	 * So: we return null here, so that one may use a consistent technique
	 * to allocate exceptions to pass to `CORBA_BOA_set_exception', without
	 * suffering possible memory leakage.
	 */
	return 0;
	/*
	 * (CORBA_ORB_InvalidName *) CORBA_alloc(sizeof CORBA_ORB_InvalidName);
	 */
}

/*
 * This function returns the value of an exception.  It could be a macro.
 */
void *
CORBA_exception_value(CORBA_Environment *ev)
{
	if (ev->_major == CORBA_SYSTEM_EXCEPTION)
		return &((ev)->_value._system_except);
	else
		return (ev)->_value._user_except;
}

/*
 * Here is the function that one calls in order to signal an exception.  See
 * Section 17.27.2 of the CORBA 2.1 specification.
 *
 * The `exceptname' is considered to be a static or ``interned'' string, e.g.,
 * ex_NAME, and is therefore not copied.  (Nor is it freed later on; see
 * `CORBA_exception_free'.)
 *
 * System exceptions are handled specially: the `minor' and `completed' values
 * are copied directly into the `CORBA_Environment' structure, thus avoiding
 * any need to allocate memory when throwing a system (e.g., CORBA_NO_MEMORY)
 * exception.  When signaling a system exception, `param' MUST have been
 * allocated using one of the standard CORBA system exception allocators (e.g.,
 * `CORBA_NO_MEMORY__alloc').
 *
 * In all cases, the `param' becomes ``owned'' by the environment structure,
 * and therefore doesn't need to be copied.  The runtime will take care of
 * freeing the storage (either now in the case of system exceptions, or later
 * in the case of user exceptions --- see `CORBA_exception_free').
 */
void
CORBA_BOA_set_exception(CORBA_BOA boa, CORBA_Environment *ev,
			CORBA_exception_type major,
			CORBA_char *exceptname, void *param)
{
	ev->_major = major;
	ev->_id = exceptname;
	
	switch (major) {
	case CORBA_SYSTEM_EXCEPTION:
		ev->_value._system_except.minor
			= ((flick_system_exception *) param)->minor;
		ev->_value._system_except.completed
			= ((flick_system_exception *) param)->completed;
		/*
		 * We've copied the fields into the environment, so we can
		 * now free the system exception structure.
		 */
		flick_system_exception_free(param);
		break;
		
	case CORBA_USER_EXCEPTION:
		ev->_value._user_except = param;
		/* `param' will be freed later by `CORBA_excpetion_free'. */
		break;
		
	case CORBA_NO_EXCEPTION:
		/* assert(param == 0); */
		ev->_value._user_except = 0;
		break;
		
	default:
		fprintf(stderr,
			("Warning: invalid exception type (value = %d) "
			 "passed to `CORBA_BOA_set_exception'.\n"),
			major);
		/* User code blew it!  Manufacture a system exception. */
		ev->_major = CORBA_SYSTEM_EXCEPTION;
		ev->_id = ex_CORBA_BAD_PARAM;
		ev->_value._system_except.minor = 0;
		ev->_value._system_except.completed = CORBA_COMPLETED_MAYBE;
		break;
	}
}

/* End of file. */

