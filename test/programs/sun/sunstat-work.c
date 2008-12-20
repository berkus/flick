/*
 * Copyright (c) 1997 The University of Utah and
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
#include <flick/link/suntcp.h>
#ifndef RPCGEN
#include "sunstat-server.h"
#else
#include "sunstat.h"
#endif

int *dirlst_2(directory *arg, struct svc_req *_obj)
{
	static int res = 0;
	return &res;
}

int *lng_2(longlist *arg, struct svc_req *_obj)
{
	static int res = 0;
	return &res;
}

int *strct_2(structlist *arg, struct svc_req *_obj)
{
	static int res = 0;
	return &res;
}

/* End of file. */

