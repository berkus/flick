/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include <mom/compiler.h>

#define DEFAULT_STACK_SIZE 10

struct ptr_stack *create_ptr_stack()
{
	struct ptr_stack *stack;

	stack = (struct ptr_stack *)mustmalloc(sizeof(struct ptr_stack));
	stack->ptrs = (void **)mustmalloc(sizeof(void *) * DEFAULT_STACK_SIZE);
	stack->size = DEFAULT_STACK_SIZE;
	stack->top = -1;
	return stack;
}

void delete_ptr_stack(struct ptr_stack *stack)
{
}

int empty_ptr_stack(struct ptr_stack *stack)
{
	return( stack->top == -1 );
}

int ptr_stack_length(struct ptr_stack *stack)
{
	return( stack->top + 1 );
}

void push_ptr(struct ptr_stack *stack, void *ptr)
{
	stack->top++;
	if (stack->top >= stack->size) {
		stack->size++;
		stack->ptrs = (void **)mustrealloc(
			stack->ptrs,
			sizeof(void *) * stack->size);
	}
	stack->ptrs[stack->top] = ptr;
}

void pop_ptr(struct ptr_stack *stack)
{
	stack->top--;
}

void *top_ptr(struct ptr_stack *stack)
{
	if (stack->top >= 0)
		return stack->ptrs[stack->top];
	return 0;
}
