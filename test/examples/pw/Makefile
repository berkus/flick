#
# Copyright (c) 1996 The University of Utah and
# the Computer Systems Laboratory at the University of Utah (CSL).
#
# This file is part of Flick, the Flexible IDL Compiler Kit.
#
# Flick is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Flick is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Flick; see the file COPYING.  If not, write to
# the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
#

# makefile -- example

VPATH=../../corba.0:../support
CC=gcc
SVROBJ=pwsrv.o pw_svc.o flick_mach3mig_glue.o mig_put_reply_port.o svc_main.o
CLNOBJ=pwcln.o pw_clnt.o flick_mach3mig_glue.o mig_put_reply_port.o \
	clnt_create.o 

LDLIBS=-lnetname -lmach
CFLAGS=-I../../corba.0 \
	-DPROGRAM_NAME=\"4711/1\" -Dcall_server=pwsrv_1

default:
	@echo 'Type make stubs or make progs'

stubs:	pw_svc.c pw_clnt.c 

progs:  pwsrv pwcln 

clean:
	rm -f pwsrv pwcln $(SVROBJ) $(CLNOBJ) pw.h *.o pwsrv-sun pwcln-sun
	rm -f pw_clnt.c pw_svc.c *.aoi *.prc pw_xdr.c

pw.aoi: pw.x
	flick-fe-sun pw.x > $@

pw_svc.prc: pw.aoi
	flick-c-pfe-sun	-s -o $@ pw.aoi

pw_clnt.prc: pw.aoi
	flick-c-pfe-sun	-c -o $@ pw.aoi

pw_svc.c: pw_svc.prc
	flick-c-pbe-mach3mig -h pw.h pw_svc.prc

pw_clnt.c: pw_clnt.prc
	flick-c-pbe-mach3mig pw_clnt.prc

pwsrv: $(SVROBJ)
	$(CC) -o $@ $(SVROBJ) $(LDLIBS)

pwcln: $(CLNOBJ)
	$(CC) -o $@ $(CLNOBJ) $(LDLIBS)

