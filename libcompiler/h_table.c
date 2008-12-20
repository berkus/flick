/*
 * Copyright (c) 1999 The University of Utah and
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

#include <ctype.h>
#include <stdlib.h>

#include <mom/compiler.h>

int hash_name(const char *name, int table_size)
{
	int h;
	
	for( h = 0; *name; name++ )
		h = (64 * h + tolower(*name)) % table_size;
	return( h );
}

struct h_table *create_hash_table(int size)
{
	struct h_table *retval;
	int lpc;
	
	retval = (struct h_table *)mustmalloc(sizeof(struct h_table) +
					      (size * sizeof(struct h_entry)));
	retval->table_size = size;
	retval->entries = (struct h_entry **)(retval + 1);
	for( lpc = 0; lpc < size; lpc++ ) {
		retval->entries[lpc] = 0;
	}
	return( retval );
}

void delete_hash_table(struct h_table *ht)
{
	struct h_entry *curr, *next;
	int lpc;
	
	for( lpc = 0; lpc < ht->table_size; lpc++ ) {
		curr = ht->entries[lpc];
		while( curr ) {
			next = curr->next;
			free( curr );
			curr = next;
		}
	}
	free( ht );
}

void add_entry(struct h_table *ht, struct h_entry *he)
{
	int h;
	
	h = hash_name(he->name, ht->table_size);
	he->next = ht->entries[h];
	ht->entries[h] = he;
}

void rem_entry(struct h_table *ht, const char *name)
{
	struct h_entry *curr, **pred;
	int h, done = 0;
	
	h = hash_name(name, ht->table_size);
	curr = ht->entries[h];
	pred = &ht->entries[h];
	while( curr && !done ) {
		if( !strcmp( name, curr->name ) ) {
			*pred = curr->next;
			done = 1;
		}
		pred = &curr->next;
		curr = curr->next;
	}
}

struct h_entry *find_entry(struct h_table *ht, const char *name)
{
	struct h_entry *retval = 0, *curr;
	int h;
	
	h = hash_name(name, ht->table_size);
	curr = ht->entries[h];
	while( curr && !retval ) {
		if( !strcmp( name, curr->name ) )
			retval = curr;
		curr = curr->next;
	}
	return( retval );
}
