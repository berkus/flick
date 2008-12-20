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

#include <mom/c/pbe.hh>

tag_list *flags_to_tag_list(struct flags_in *in, struct flags_out *out,
			    int flag_count)
{
	struct flag_value_seq *fvs;
	unsigned int lpc2;
	tag_list *retval;
	tag_item *ti;
	int lpc;
	
	retval = create_tag_list(0);
	add_tag(retval, "progname", TAG_STRING, out->progname);
	for( lpc = 0; lpc < flag_count; lpc++ ) {
		fvs = &out->flag_seqs[lpc];
		ti = add_tag(retval, in[lpc].dbl, TAG_NONE);
		switch( in[lpc].kind ) {
		case fk_FLAG:
			if( (in[lpc].max_occur == FLAG_UNLIMITED_USE_LAST) ||
			    (in[lpc].max_occur == 1) ) {
				ti->data.kind = TAG_BOOL;
				ti->data.tag_data_u.b = fvs->values[0].flag;
			} else {
				ti->data = create_tag_data(TAG_BOOL_ARRAY,
							   fvs->len);
				for( lpc2 = 0; lpc2 < fvs->len; lpc2++ ) {
					ti->data.tag_data_u.b_a.b_a_val[lpc2] =
						fvs->values[lpc2].flag;
				}
			}
			break;
		case fk_STRING:
			if( (in[lpc].max_occur == FLAG_UNLIMITED_USE_LAST) ||
			    (in[lpc].max_occur == 1) ) {
				ti->data.kind = TAG_STRING;
				if( fvs->values[0].string )
					ti->data.tag_data_u.str =
						(char *)fvs->values[0].string;
				else
					ti->data.tag_data_u.str =
						ir_strlit("");
			} else {
				if( (fvs->len == 1) &&
				    !fvs->values[0].string )
					fvs->len = 0;
				ti->data = create_tag_data(TAG_STRING_ARRAY,
							   fvs->len);
				for( lpc2 = 0; lpc2 < fvs->len; lpc2++ ) {
					if( fvs->values[lpc2].string )
						ti->data.tag_data_u.str_a.
							str_a_val[lpc2] =
							(char *)fvs->
							values[lpc2].string;
					else
						ti->data.tag_data_u.str_a.
							str_a_val[lpc2] =
							ir_strlit("");
				}
			}
			break;
		case fk_NUMBER:
			if( (in[lpc].max_occur == FLAG_UNLIMITED_USE_LAST) ||
			    (in[lpc].max_occur == 1) ) {
				ti->data.kind = TAG_INTEGER;
				ti->data.tag_data_u.i = fvs->values[0].number;
			} else {
				ti->data = create_tag_data(TAG_INTEGER_ARRAY,
							   fvs->len);
				for( lpc2 = 0; lpc2 < fvs->len; lpc2++ ) {
					ti->data.tag_data_u.i_a.i_a_val[lpc2] =
						fvs->values[lpc2].number;
				}
			}
			break;
		}
	}
	return( retval );
}
