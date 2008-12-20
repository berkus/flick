/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

/*
 * This file contains a set of tests using the functions defined in
 * mig-tests.defs.  
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <mach_error.h>

#include "mig-tests-client.h"

#define SENT_ONE "This is a very profound sentence."
#define SENT_TWO "Sentence two expands on sentence one's profoundness."
#define SENT_THREE "Our third sentence is quite boring."

/* macro for calling a client stub */
#define _try(_func, _params) {\
	int _result;\
	printf("Calling function %s...\n", #_func);\
	if ((_result = _func _params) != KERN_SUCCESS) {\
		printf("  FAILED CALL to function %s: "\
		       "result = %d (0x%08x) [%s]\n",\
			#_func, _result, _result,\
		       mach_error_string(_result));\
	        errs++;\
	}\
}

/* function to print an array of integers */
void parr(int *a, int size)
{
	int i;
	if (!a) {
		printf("NULL\n");
		return;
	}
	printf("{ ");
	for (i=0; i<size; i++) {
		if (i) printf(", ");
		printf("%d",a[i]);
	}
	printf(" }\n");
}

/* print out a port type */
char * typestr(mach_port_t p)
{
	static char str[500];
	mach_port_type_t tp;
	int ret;
	
	ret = mach_port_type(mach_task_self(), p, &tp);
	if (ret == KERN_INVALID_NAME) return "INVALID_NAME";
	if (ret != KERN_SUCCESS) return "UNKNOWN";
	str[0] = 0;
	if (tp & MACH_PORT_TYPE_SEND) strcat(str, "SEND ");
	if (tp & MACH_PORT_TYPE_SEND_ONCE) strcat(str, "SEND_ONCE ");
	if (tp & MACH_PORT_TYPE_RECEIVE) strcat(str, "RECV ");
	if (tp & MACH_PORT_TYPE_PORT_SET) strcat(str, "PORT_SET ");
	if (tp & MACH_PORT_TYPE_DEAD_NAME) strcat(str, "DEAD ");
	if (tp & MACH_PORT_TYPE_DNREQUEST) strcat(str, "DNREQ ");
	return str;
}

#define failed(_func) {\
	errs++;\
	printf("  FAILED CHECK\n");\
}

/*
 * This is the main body of the client-side tests.
 */
int mig_tests_client(mach_port_t right)
{
	int errs = 0;
	int i;

	/*
	 * IN-LINE TESTS
	 */
	
	/* integers */
	{
		int a = 20;
		int b = 10;
		int c = 0;
#if 0
		printf("\nBefore call, a=%d, b=%d, c=%d\n", a, b, c);
#endif
		_try(inline_integers, (right, a, &b, &c));
#if 0
		printf("After call, a=%d, b=%d, c=%d\n", a, b, c);
#endif
		if (b!=200 || c!=200) {
			failed(inline_integers);
		}
	}
	/* fixed structs */
	{
		IFstruct a, b, c, s;
		memset(&a, 1, sizeof(a));
		memset(&b, 2, sizeof(b));
		memset(&c, 0, sizeof(c));
		s = b;
#if 0
		printf("\nBefore call, a=");parr((int *)&a, 100);
		printf("b=");parr((int *)&b, 100);
		printf("c=");parr((int *)&c, 100);
#endif
		_try(inline_fixed_structs, (right, a, &b, &c));
#if 0
		printf("After call, a=");parr((int *)&a, 100);
		printf("b=");parr((int *)&b, 100);
		printf("c=");parr((int *)&c, 100);
#endif
		if (strncmp((char *)&b, (char *)&a, sizeof(IFstruct)) ||
		    strncmp((char *)&c, (char *)&s, sizeof(IFstruct))) {
			failed(inline_fixed_structs);
		}
	}
	/* fixed arrays */
	{
		IFarray a, b, c;
		for (i=0; i<300; i++) {
			a[i] = i;
			b[i] = i;
			c[i] = 0;
		}
#if 0
		printf("\nBefore call, a=");parr(a, 300);
		printf("b=");parr(b, 300);
		printf("c=");parr(c, 300);
#endif
		_try(inline_fixed_arrays, (right, a, b, c));
#if 0
		printf("After call, a=");parr(a, 300);
		printf("b=");parr(b, 300);
		printf("c=");parr(c, 300);
#endif
		for (i=0; i<300; i++) {
			if (a[i] != i ||
			    b[i] != i*i ||
			    c[i] != i*i) {
				failed(inline_fixed_arrays);
				break;
			}
		}
	}
	/* bounded arrays */
	{
		IBarray a, b, c;
		int n = 500;
		for (i=0; i<100; i++) {
			a[i] = i;
			b[i] = i;
			c[i] = 0;
		}
#if 0
		printf("\nBefore call, a=");parr(a, 10);
		/*printf("b=");parr(b, 10);*/
		printf("c=");parr(c, 10);
#endif
		_try(inline_bounded_arrays, (right, a, 10, c, &n));
#if 0
		printf("After call, i=%d, a=",i);parr(a, 10);
		/*printf("b=");parr(b, 10);*/
		printf("c=");parr(c, n);
#endif
		if (n != 10) {
			failed(inline_bounded_arrays);
		} else for (i=0; i<n; i++) {
			if (a[i] != i ||
			    c[i] != i) {
				failed(inline_bounded_arrays);
				break;
			}
		}
		n = 500;
		for (i=0; i<500; i++) {
			a[i] = i;
			b[i] = i;
			c[i] = 0;
		}
#if 0
		printf("\nBefore call, a=");parr(a, 500);
		/*printf("b=");parr(b, 500);*/
		printf("c=");parr(c, 500);
#endif
		_try(inline_bounded_arrays, (right, a, 500, c, &n));
#if 0
		printf("After call, i=%d, a=", i);parr(a, 500);
		/*printf("b=");parr(b, 500);*/
		printf("c=");parr(c, n);
#endif
		if (n != 500) {
			failed(inline_bounded_arrays);
		} else for (i=0; i<n; i++) {
			if (a[i] != i ||
			    c[i] != i) {
				failed(inline_bounded_arrays);
				break;
			}
		}
	}
	/* unbounded arrays */
	{
		/* NOTE:
		   Currently Flick allocates all out parameters,
		   regardless of whether a buffer was passed in or
		   not.  If a buffer is allocated and passed in, it
		   would NOT be de/reallocated.  This differs slightly
		   from MIG; however, assuming one always passes in a
		   null pointer and size 0, both MIG and Flick produce
		   the same effect. */
		
		Iarray a, c;
		int ac = 100, cc = 0;
		a = (unsigned *)malloc(sizeof(unsigned)*ac);
		c = 0;
		for (i=0; i<100; i++) {
			a[i] = i;
		}
#if 0
		printf("\nBefore call, a[%d]=",ac);parr(a, ac);
		/*printf("b[%d]=",bc);parr(b, bc);*/
		printf("c[%d]=",cc);parr(c, cc);
#endif
		_try(inline_unbounded_arrays, (right, a, ac, &c, &cc));
#if 0
		printf("After call, a[%d]=",ac);parr(a, ac);
		/*printf("b[%d]=",bc);parr(b, bc);*/
		printf("c[%d]=",cc);parr(c, cc);
#endif
		for (i=0; i<100; i++) {
			if (a[i] != i ||
			    c[i] != i) {
				failed(inline_unbounded_arrays);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) c, cc);
	}
	/* unbounded arrays (automatically sent out-of-line) */
	{
		/* NOTE:
		   Currently Flick allocates all out parameters,
		   regardless of whether a buffer was passed in or
		   not.  If a buffer is allocated and passed in, it
		   would NOT be de/reallocated.  This differs slightly
		   from MIG; however, assuming one always passes in a
		   null pointer and size 0, both MIG and Flick produce
		   the same effect. */
		
		Iarray a, c;
		int ac = 1000, cc = 0;
		a = (unsigned *)malloc(sizeof(unsigned)*ac);
		c = 0;
		for (i=0; i<1000; i++) {
			a[i] = i;
		}
#if 0
		printf("\nBefore call, a[%d]=",ac);parr(a, ac);
		/*printf("b[%d]=",bc);parr(b, bc);*/
		printf("c[%d]=",cc);parr(c, cc);
#endif
		_try(inline_unbounded_arrays, (right, a, ac, &c, &cc));
#if 0
		printf("After call, a[%d]=",ac);parr(a, ac);
		/*printf("b[%d]=",bc);parr(b, bc);*/
		printf("c[%d]=",cc);parr(c, cc);
#endif
		for (i=0; i<1000; i++) {
			if (a[i] != i ||
			    c[i] != i) {
				failed(inline_unbounded_arrays);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) c, cc);
	}
	/* fixed strings */
	{
		IFstring a = SENT_ONE,
			 b = SENT_TWO,
			 c = SENT_THREE;
#if 0
		printf("\nBefore call, a[%ld]= %s\n",strlen(a),a);
		printf("b[%ld]= %s\n",strlen(b),b);
		printf("c[%ld]= %s\n",strlen(c),c);
#endif
		_try(inline_fixed_strings, (right, a, b, c));
#if 0
		printf("After call, a[%ld]= %s\n",strlen(a),a);
		printf("b[%ld]= %s\n",strlen(b),b);
		printf("c[%ld]= %s\n",strlen(c),c);
#endif
		if (strcmp(a, SENT_ONE) ||
		    strcmp(b, SENT_ONE) ||
		    strcmp(c, SENT_TWO)) {
			failed(inline_fixed_strings);
		}
	}
	/* bounded strings */
	{
		IVstring a = SENT_ONE,
			 b = SENT_TWO;
#if 0
		printf("\nBefore call, a[%ld]= %s\n",strlen(a),a);
		printf("b[%ld]= %s\n",strlen(b),b);
#endif
		_try(inline_bounded_strings, (right, a, b));
#if 0
		printf("After call, a[%ld]= %s\n",strlen(a),a);
		printf("b[%ld]= %s\n",strlen(b),b);
#endif
		if (strcmp(a, SENT_ONE) ||
		    strcmp(b, SENT_ONE)) {
			failed(inline_fixed_strings);
		}
	}
	/* port rights */
	{
		mach_port_t a, b, c;
		mach_port_type_t tp;
		
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &a);
		b = right;
		c = MACH_PORT_NULL;
#if 0
		printf("\nBefore call, a = 0x%08x (%s)\n", a, typestr(a));
		printf("b = 0x%08x (%s)\n", b, typestr(b));
		printf("c = 0x%08x (%s)\n", c, typestr(c));
#endif
		_try(inline_port_rights, (right, a, &b, &c));
#if 0
		printf("After call, a = 0x%08x (%s)\n", a, typestr(a));
		printf("b = 0x%08x (%s)\n", b, typestr(b));
		printf("c = 0x%08x (%s)\n", c, typestr(c));
#endif
		mach_port_deallocate(mach_task_self(), c);
		if (mach_port_type(mach_task_self(), a, &tp) != KERN_INVALID_NAME ||
		    b != right ||
		    mach_port_type(mach_task_self(), c, &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE)) {
			failed(inline_port_rights);
		}
	}
	/* port right arrays */
	{
		Imach_port_array_t a, b;
		int ac = 2, bc = 0;
		mach_port_type_t tp;

		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(mach_port_t)*2, 1);
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(a[0]));
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(a[1]));
		b = MACH_PORT_NULL;
#if 0
		printf("\nBefore call, a[0] = 0x%08x (%s),   ",
		       a[0], typestr(a[0]));
		printf("a[1] = 0x%08x (%s)\n", a[1], typestr(a[1]));
#endif
		_try(inline_port_arrays, (right, a, ac, &b, &bc));
#if 0
		printf("After call, a[0] = 0x%08x (%s),   ",
		       a[0], typestr(a[0]));
		printf("a[1] = 0x%08x (%s)\n", a[1], typestr(a[1]));
		printf("b[%d]: b[0] = 0x%08x (%s),   ", bc,
		       b[0], typestr(b[0]));
		printf("b[1] = 0x%08x (%s)\n", b[1], typestr(b[1]));
#endif
		if (mach_port_type(mach_task_self(), a[0], &tp) != KERN_INVALID_NAME ||
		    mach_port_type(mach_task_self(), a[1], &tp) != KERN_INVALID_NAME ||
		    mach_port_type(mach_task_self(), b[0], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE) ||
		    mach_port_type(mach_task_self(), b[1], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE)) {
			failed(inline_port_arrays);
		}
		mach_port_deallocate(mach_task_self(), b[0]);
		mach_port_deallocate(mach_task_self(), b[1]);
	}
	
	/*
	 * OUT-OF-LINE TESTS
	 */
#if 1
	/* Only variable-length out-of-line data works under
           MIG. Fixed length data is set to be of length 0, and thus
           nothing is transmitted.  In Flick, this problem is
           resolved, thus any type of out-of-line data can be
           sent/received correctly. */
	
	/* integers  -- CURRENTLY DON'T WORK UNDER MIG */
	{
		Oint a, b, c = 0;
		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(int), 1);
		*a = 10;
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    sizeof(int), 1);
		*b = 20;
#if 0
		printf("\nBefore call, a=0x%08x,%d, b=0x%08x,%d, c=0x%08x\n",
		       (int)a, *a, (int)b, *b, (int)c);
#endif
		_try(outline_integers, (right, a, &b, &c));
#if 0
		printf("After call, a=0x%08x,%d, b=0x%08x,%d, c=0x%08x,%d\n",
		       (int)a, (a)?(*a):0, (int)b, (b)?(*b):0, (int)c, (c)?(*c):0);
		fflush(stdout);
#endif
		if (*b != 200 ||
		    c == NULL ||
		    *c != 200) {
			failed(outline_integers);
		}
		vm_deallocate(mach_task_self(), (vm_address_t) a, sizeof(int));
		vm_deallocate(mach_task_self(), (vm_address_t) b, sizeof(int));
		if (c)
			vm_deallocate(mach_task_self(), (vm_address_t) c, sizeof(int));
	}
	/* fixed structs  -- CURRENTLY DON'T WORK UNDER MIG */
	{
		OFstruct a, b, c = 0;
		OFstruct s;
		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(*a), 1);
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    sizeof(*b), 1);
		vm_allocate(mach_task_self(), (vm_address_t *) &s,
			    sizeof(*s), 1);
		memset(a, 1, sizeof(*a));
		memset(b, 2, sizeof(*b));
		*s = *b;
#if 0
		printf("\nBefore call, a=");parr((int *)a, 200);
		printf("b=");parr((int *)b, 200);
#endif
		_try(outline_fixed_structs, (right, a, &b, &c));
#if 0
		printf("After call, a=");parr((int *)a, 200);
		printf("b=");parr((int *)b, 200);
		printf("c=");parr((int *)c, 200);
#endif
		if (strncmp((char *)&b, (char *)&a, sizeof(*a)) ||
		    strncmp((char *)&c, (char *)&s, sizeof(*s))) {
			failed(inline_fixed_structs);
		}
		vm_deallocate(mach_task_self(), (vm_address_t) a, sizeof(*a));
		vm_deallocate(mach_task_self(), (vm_address_t) b, sizeof(*b));
		if (c)
			vm_deallocate(mach_task_self(), (vm_address_t) c, sizeof(*c));
	}
	/* fixed arrays  -- CURRENTLY DON'T WORK UNDER MIG */
	{
		OFarray a, b, c = 0;
		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(int)*400, 1);
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    sizeof(int)*400, 1);
		for (i=0; i<400; i++) {
			(*a)[i] = i;
			(*b)[i] = i;
		}
#if 0
		printf("\nBefore call, a=");parr(*a, 400);
		printf("b=");parr(*b, 400);
#endif
		_try(outline_fixed_arrays, (right, a, &b, &c));
#if 0
		printf("After call, a=");parr(*a, 400);
		printf("b=");parr(*b, 400);
		printf("c=");parr(*c, 400);
#endif
		for (i=0; i<400; i++) {
			if ((*b)[i] != i*i ||
			    (*c)[i] != i*i) {
				failed(outline_fixed_arrays);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) a, sizeof(int)*400);
		vm_deallocate(mach_task_self(), (vm_address_t) b, sizeof(int)*400);
		vm_deallocate(mach_task_self(), (vm_address_t) c, sizeof(int)*400);
	}
	/* unbounded arrays */
	{
		Oarray a, b, c = 0;
		int ac = 100, bc = 200, cc = 0;
		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(int)*ac, 1);
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    sizeof(int)*bc, 1);
		for (i=0; i<ac; i++) {
			a[i] = i;
		}
		for (i=0; i<bc; i++) {
			b[i] = i * i;
		}
#if 0
		printf("\nBefore call, a[%d]=",ac);parr(a, ac);
		printf("b[%d]=",bc);parr(b, bc);
		printf("c[%d]=",cc);parr(c, cc);
#endif
		_try(outline_unbounded_arrays, (right, a, ac, &b, &bc, &c, &cc));
#if 0
		printf("After call, a[%d]=",ac);parr(a, ac);
		printf("b[%d]=",bc);parr(b, bc);
		printf("c[%d]=",cc);parr(c, cc);
#endif
		if (bc != 100 || cc != 200) {
			failed(outline_unbounded_arrays);
		} else for (i=0; i<((bc>cc)?bc:cc); i++) {
			if ((i<bc && (b[i] != i)) ||
			    (i<cc && (c[i] != i*i))) {
				failed(outline_unbounded_arrays);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) a, sizeof(int)*ac);
		vm_deallocate(mach_task_self(), (vm_address_t) b, sizeof(int)*bc);
		vm_deallocate(mach_task_self(), (vm_address_t) c, sizeof(int)*cc);
	}
	/* fixed strings  -- CURRENTLY DON'T WORK UNDER MIG */
	{
		OFstring a, b, c = 0;

		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    800, 1);
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    800, 1);
		strcpy(*a, SENT_ONE);
		strcpy(*b, SENT_TWO);
#if 0
		printf("\nBefore call, a[%ld]= %s\n",strlen(*a),*a);
		printf("b[%ld]= %s\n",strlen(*b),*b);
#endif
		_try(outline_fixed_strings, (right, a, &b, &c));
#if 0
		printf("After call, a[%ld]= %s\n",strlen(*a),*a);
		printf("b[%ld]= %s\n",strlen(*b),*b);
		printf("c[%ld]= %s\n",strlen(*c),*c);
#endif
		if (strcmp(*a, SENT_ONE) ||
		    strcmp(*b, SENT_ONE) ||
		    strcmp(*c, SENT_TWO)) {
			failed(outline_fixed_strings);
		}
		vm_deallocate(mach_task_self(), (vm_address_t) a, 800);
		vm_deallocate(mach_task_self(), (vm_address_t) b, 800);
		vm_deallocate(mach_task_self(), (vm_address_t) c, 800);
	}
	/* port right arrays */
	{
		mach_port_array_t a, b, c = 0;
		int ac = 2, bc = 2, cc = 0;
		mach_port_t bs1, bs2;
		mach_port_type_t tp;
		
		vm_allocate(mach_task_self(), (vm_address_t *) &a,
			    sizeof(mach_port_t)*2, 1);
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(a[0]));
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(a[1]));
		vm_allocate(mach_task_self(), (vm_address_t *) &b,
			    sizeof(mach_port_t)*2, 1);
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(b[0]));
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &(b[1]));
		bs1 = b[0];
		bs2 = b[1];
#if 0
		printf("\nBefore call, a[0] = 0x%08x (%s),   ",
		       a[0], typestr(a[0]));
		printf("a[1] = 0x%08x (%s)\n", a[1], typestr(a[1]));
		printf("b[%d]: b[0] = 0x%08x (%s),   ", bc, 
		       b[0], typestr(b[0]));
		printf("b[1] = 0x%08x (%s)\n", b[1], typestr(b[1]));
#endif
		_try(outline_port_arrays, (right, a, ac, &b, &bc, &c, &cc));
#if 0
		printf("After call, a[0] = 0x%08x (%s),   a[1] = 0x%08x (%s)\n",
		       a[0], typestr(a[0]), a[1], typestr(a[1]));
		printf("b[%d]: b[0] = 0x%08x (%s),   ", bc,
		       b[0], typestr(b[0]));
		printf("b[1] = 0x%08x (%s)\n", b[1], typestr(b[1]));
		printf("c[%d]: c[0] = 0x%08x (%s),   ", cc,
		       c[0], typestr(c[0]));
		printf("c[1] = 0x%08x (%s)\n", c[1], typestr(c[1]));
#endif
		if (bc !=2 ||
		    cc != 2 ||
		    mach_port_type(mach_task_self(), a[0], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE) ||
		    mach_port_type(mach_task_self(), a[1], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE) ||
		    mach_port_type(mach_task_self(), b[0], &tp) != KERN_INVALID_NAME ||
		    mach_port_type(mach_task_self(), b[1], &tp) != KERN_INVALID_NAME ||
		    mach_port_type(mach_task_self(), c[0], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_SEND) ||
		    mach_port_type(mach_task_self(), c[1], &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_SEND)) {
			failed(outline_port_arrays);
		}
		mach_port_deallocate(mach_task_self(), a[0]);
		mach_port_deallocate(mach_task_self(), a[1]);
		mach_port_deallocate(mach_task_self(), c[0]);
		mach_port_deallocate(mach_task_self(), c[1]);
		/* free these just in case they didn't get moved */
		mach_port_deallocate(mach_task_self(), bs1);
		mach_port_deallocate(mach_task_self(), bs2);
	}
#endif	
	/*
	 * OTHER TESTS
	 */
	
	/* reply ports */
	{
		mach_port_t a;
		mach_port_type_t tp;

/*		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &a);*/
		a = mig_get_reply_port();
		i = 10;
#if 0
		printf("\nBefore call, a = 0x%08x (%s), ", a, typestr(a));
		printf("right = 0x%08x (%s), i = %d\n", right, typestr(right), i);
#endif
		_try(test_reply_ports, (a, right, &i));
#if 0
		printf("After call, a = 0x%08x (%s), ", a, typestr(a));
		printf("right = 0x%08x (%s), i = %d\n", right, typestr(right), i);
#endif
		if (i != 100 ||
		    mach_port_type(mach_task_self(), a, &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE)) {
			failed(test_reply_ports);
		}
/*		mach_port_deallocate(mach_task_self(), a);*/
		
		i = 10;
#if 0
		printf("\nBefore call, right = 0x%08x (%s), i = %d\n",
		       right, typestr(right), i);
#endif
		_try(test_sreply_ports, (right, &i));
#if 0
		printf("After call, right = 0x%08x (%s), i = %d\n",
		       right, typestr(right), i);
#endif
		if (i != 100) {
			failed(test_sreply_ports);
		}
		
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &a);
		i = 10;
#if 0
		printf("\nBefore call, a = 0x%08x (%s), ", a, typestr(a));
		printf("right = 0x%08x (%s), i = %d\n", right, typestr(right), i);
#endif
		_try(test_ureply_ports, (right, a, &i));
#if 0
		printf("After call, a = 0x%08x (%s), ", a, typestr(a));
		printf("right = 0x%08x (%s), i = %d\n", right, typestr(right), i);
#endif
		if (i != 100 ||
		    mach_port_type(mach_task_self(), a, &tp) != KERN_SUCCESS ||
		    !(tp & MACH_PORT_TYPE_RECEIVE)) {
			failed(test_ureply_ports);
		}
		mach_port_deallocate(mach_task_self(), a);
	}
	/* parameter flags */
	{
		/* Normal operation */
		i = 10;
		_try(test_param_flags, (500, MACH_MSG_OPTION_NONE, right, &i));
		if (i != 100) {
			failed(test_param_flags(notimeout1));
		}
		
		/* No reply ever sent */
		i = 0;
#if 1
		printf("Calling function test_param_flags...\n");
		if ((i = test_param_flags(500, MACH_MSG_OPTION_NONE, right, &i)) !=
		     MACH_RCV_TIMED_OUT) {
			failed(test_param_flags(timeout));
			printf("  result = %d (0x%08x)", i, i);
		}
#else
		_try(test_param_flags, (500, MACH_MSG_OPTION_NONE, right, &i));
#endif
		sleep(2); /* make sure server is done waiting */
		
		/* Normal operation */
		i = 20;
		_try(test_param_flags, (500, MACH_MSG_OPTION_NONE, right, &i));
		if (i != 400) {
			printf("i=%d\n",i);
			failed(test_param_flags(notimeout2));
		}
		
		/* Reply delayed */
		i = -2;
		_try(test_param_flags, (3000, MACH_MSG_OPTION_NONE, right, &i));
		if (i != 0) {
			printf("i=%d\n",i);
			failed(test_param_flags(notimeout3));
		}
	}
	/* countinout */
	{
		Iarray c = 0;
		int cc = 45;
#if 0
		printf("\nBefore call, c[%d]=",cc);parr(c, cc);
#endif
		_try(test_countinout, (right, &c, &cc));
#if 0
		printf("After call, c[%d]=",cc);parr(c, cc);
#endif
		if (cc != 45) failed(test_countinout);
		for (i=0; i<cc; i++) {
			if (c[i] != i) {
				failed(test_countinout);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) c, cc);
		
		cc = 55;
#if 0
		printf("\nBefore call, c[%d]=",cc);parr(c, cc);
#endif
		_try(test_countinout, (right, &c, &cc));
#if 0
		printf("After call, c[%d]=",cc);parr(c, cc);
#endif
		if (cc != 50) failed(test_countinout);
		for (i=0; i<cc; i++) {
			if (c[i] != i) {
				failed(test_countinout);
				break;
			}
		}
		vm_deallocate(mach_task_self(), (vm_address_t) c, cc);
	}
	/* servercopy */
	{
		Iarray a;
		boolean_t scopy;
		int ac = 1000;
		a = (unsigned *)malloc(sizeof(unsigned)*ac);
		for (i=0; i<1000; i++) {
			a[i] = i;
		}
#if 0
		printf("\nBefore call, a[%d]=",ac);parr(a, ac);
#endif
		_try(test_servercopy, (right, a, ac, &scopy));
#if 0
		printf("After call, a[%d]=",ac);parr(a, ac);
		printf("%s",scopy? "in-line":"out-of-line");
#endif
		if ((!scopy && ac <= 512)
		    || (scopy && ac > 512)) {
			failed(test_servercopy);
		}
		for (i=0; i<ac; i++) {
			if (a[i] != i) {
				failed(test_servercopy);
				break;
			}
		}
		
		ac /= 2;
		
#if 0
		printf("\nBefore call, a[%d]=",ac);parr(a, ac);
#endif
		_try(test_servercopy, (right, a, ac, &scopy));
#if 0
		printf("After call, a[%d]=",ac);parr(a, ac);
		printf("%s",scopy? "in-line":"out-of-line");
#endif
		if ((!scopy && ac <= 512)
		    || (scopy && ac > 512)) {
			failed(test_servercopy);
		}
		for (i=0; i<ac; i++) {
			if (a[i] != i) {
				failed(test_servercopy);
				break;
			}
		}
	}
	/* terminate server */
	{
		_try(terminate, (right));
	}
	return errs;
}

/* This is the client function called by the test client. */
int call_client(mach_port_t right) {
#if 0
	printf("About to run client...\n");
	printf("Returning from client(%d)...\n", mig_tests_client(right));
#else
	if (mig_tests_client(right) != 0)
		printf("\n\nClient test(s) FAILED!\n");
	else
		printf("\n\nClient tests passed!\n");
#endif
	return 0;
}

/* End of file. */
