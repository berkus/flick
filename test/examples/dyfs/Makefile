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
CC=gcc -g
GENOBJ=flick_mach3mig_glue.o mig_put_reply_port.o 
SVROBJ0=$(GENOBJ) svc_main.o
CLNOBJ0=$(GENOBJ) clnt_create.o 
SVROBJ=yfs_server.o yfs_svc.o $(SVROBJ0)
CLNOBJ=readdir.o yfs_client.o yfs_clnt.o $(CLNOBJ0) 

LDLIBS=-lnetname -lmach
CFLAGS=-Ilibyfs -I../../corba.0 \
	-DPROGRAM_NAME=\"4711/1\" -Dcall_server=yfs_program_1

default:
	@echo 'Type make stubs or make progs'

stubs:	yfs_svc.c yfs_clnt.c 

progs:  libyfs.a yfsserver yfsformat yfstest tygrys 

clean:
	rm -f yfsserver yfstest tygrys yfsformat $(SVROBJ) $(CLNOBJ) 
	rm -f yfsserver-sun yfstest-sun tygrys-sun 
	rm -f yfs_clnt.c yfs_svc.c *.aoi *.prc *.o yfs.h

yfs.aoi: yfs.x
	flick-fe-sun yfs.x > $@

yfs_svc.prc: yfs.aoi
	flick-c-pfe-sun	-s -o $@ yfs.aoi

yfs_clnt.prc: yfs.aoi
	flick-c-pfe-sun	-c -o $@ yfs.aoi

yfs_svc.c: yfs_svc.prc
	flick-c-pbe-mach3mig -h yfs.h yfs_svc.prc

yfs_clnt.c: yfs_clnt.prc
	flick-c-pbe-mach3mig yfs_clnt.prc

yfsserver: $(SVROBJ)
	$(CC) -o $@ $(SVROBJ) $(LDLIBS) -L. -lyfs

yfstest: yfstest.o $(CLNOBJ)
	$(CC) -o $@ yfstest.o $(CLNOBJ) $(LDLIBS)

tygrys: tygrys.o $(CLNOBJ)
	$(CC) -o $@ tygrys.o $(CLNOBJ) $(LDLIBS)

libyfs.a:
	(cd libyfs; make)

yfsformat:	format.o
	$(CC) -o $@ format.o -L. -lyfs

