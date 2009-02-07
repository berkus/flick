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

/*
 * This file contains various handlers for a be_state, currently, we just
 * put initialization type stuff here, and causing the output to the files.
 */

/* XXX --- Is this the correct test for a non-Windows platform? */
#if !defined(__CYGWIN__) && !defined(__CYGWIN32__)
#define SYS_INFO_AVAILABLE
/* used for w_version_info */
#include <sys/types.h>
#include <pwd.h>

#ifndef MAXHOSTNAMELEN
#include <sys/param.h>
#ifndef MAXHOSTNAMELEN
#include <netdb.h>
#endif
#endif

#include <unistd.h>
#endif /* !defined(__CYGWIN__) && !defined(__CYGWIN32__) */

#ifdef NEED_GETHOSTNAME_DECL
/*
 * `gethostname' should (?) be in <unistd.h>, but Solaris 2.5 at least doesn't
 * declare `gethostname' in any system header.
 */
extern "C" { int gethostname(char *name, int namelen); }
#endif /* NEED_GETHOSTNAME_DECL */

#include <assert.h>
#include <time.h>

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/c/be/be_state.hh>
#include <mom/c/be/be_file.hh>

struct be_event *be_state_make_system_info_handler(struct be_handler */*bh*/,
						   struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	tag_list *flick_tl, *tl;
	char *time_str;
	tag_item *ti;
	int len;
	
	if( be->id != BESE_INIT )
		return( be );
	if( (ti = find_tag(state->get_scml_root()->get_values(),
			   "system_info")) )
		tl = ti->data.tag_data_u.tl;
	else {
		tl = create_tag_list(0);
		add_tag(state->get_scml_root()->get_values(),
			"system_info", TAG_TAG_LIST, tl);
	}
	flick_tl = create_tag_list(0);
	
	time_t t;
#ifdef SYS_INFO_AVAILABLE
	char hostname[MAXHOSTNAMELEN];
	
	gethostname(hostname, MAXHOSTNAMELEN);
	add_tag(tl, "hostname", TAG_STRING, muststrdup(hostname));
#else
	add_tag(tl, "hostname", TAG_STRING, "graceland");
#endif
	t = time(NULL);
	time_str = muststrdup(ctime(&t));
	len = strlen(time_str);
	time_str[len - 2] = 0;
	add_tag(tl, "time", TAG_STRING, time_str);
#ifdef SYS_INFO_AVAILABLE
	add_tag(tl, "user", TAG_STRING, getpwuid(getuid())->pw_name);
#else
	add_tag(tl, "user", TAG_STRING, "elvis");
#endif
	add_tag(flick_tl, "version", TAG_STRING, FLICK_VERSION);
	add_tag(flick_tl, "date", TAG_STRING, __DATE__);
	add_tag(flick_tl, "time", TAG_STRING, __TIME__);
	add_tag(flick_tl, "contact", TAG_STRING, "flick-bugs@cs.utah.edu");
	add_tag(tl, "flick", TAG_TAG_LIST, flick_tl);
	return( be );
}

struct be_handler be_state_make_system_info("system info",
					    128,
					    be_state_make_system_info_handler);

struct be_event *be_state_gen_files_handler(struct be_handler */*bh*/,
					    struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	struct be_looper *files = state->get_file_looper();
	
	switch( be->id ) {
	case BESE_CLI_ARGS:
		delete (files->handle(files->make_event(BEFE_INIT)));
		delete (files->handle(files->make_event(BEFE_BODY)));
		break;
	case BESE_SHUTDOWN:
		delete (files->handle(files->make_event(BEFE_SHUTDOWN)));
		break;
	default:
		break;
	}
	return( be );
}

struct be_handler be_state_gen_files("gen files",
				     0,
				     be_state_gen_files_handler);

void register_state_handlers(struct be_state *state)
{
	state->add_handler(&be_state_make_system_info);
	state->add_handler(&be_state_gen_files);
}
