#
# Copyright (c) 1998, 1999 The University of Utah and
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

# Set this to your Flick source tree.
SRCDIR=/home/users/barneyrubble/flick-src
# Set this to your Flick object directory.
OBJDIR=/home/users/barneyrubble/flick-obj

# `TPZ_HOME' and `MYRI_HOME' are environment variables which must be set in
# order for one to use Trapeze.
TPZ_ARCH := $(shell $(TPZ_HOME)/arch.sh)

CC=gcc

CFLAGS=	-g -O2 -D$(TPZ_ARCH) \
	-I$(OBJDIR)/runtime/headers -I$(SRCDIR)/runtime/headers \
	-I$(MYRI_HOME)/include -I$(TPZ_HOME)/include 

LIBS=	-L$(OBJDIR)/lib -lflick-trapeze \
	$(TPZ_HOME)/OBJ/$(TPZ_ARCH)/tpz_init.o \
	$(TPZ_HOME)/OBJ/$(TPZ_ARCH)/tpz_api.o \
	-L$(MYRI_HOME)/lib/$(TPZ_ARCH) -lLanaiDevice -lbfd -liberty

CORBA_FE=$(OBJDIR)/bin/flick-fe-newcorba
CORBA_PG=$(OBJDIR)/bin/flick-c-pfe-corba
TRAPEZE_BE=$(OBJDIR)/bin/flick-c-pbe-trapeze

all: phonebook phoneserver

phonebook: phone-client.o phonebook.o
	$(CC) -o phonebook phone-client.o phonebook.o $(LIBS)

phoneserver: phone-server.o phone-workfuncs.o
	$(CC) -o phoneserver phone-server.o phone-workfuncs.o $(LIBS)

phone-client.o: phone-client.c phone-client.h
	$(CC) $(CFLAGS) -o phone-client.o -c phone-client.c

phone-server.o: phone-server.c phone-server.h
	$(CC) $(CFLAGS) -o phone-server.o -c phone-server.c

phone-server.c phone-server.h phone-client.c phone-client.h: phone.idl
	$(CORBA_FE) phone.idl && \
	$(CORBA_PG) -s -o phone-server.prc phone.aoi && \
	$(TRAPEZE_BE) phone-server.prc && \
	$(CORBA_PG) -c -o phone-client.prc phone.aoi
	$(TRAPEZE_BE) phone-client.prc && \
	rm -rf phone-client.prc phone-server.prc phone.aoi

phonebook.o: phone-client.h phonebook.c
	$(CC) $(CFLAGS) -o phonebook.o -c phonebook.c

phone-workfuncs.o: phone-server.h phone-workfuncs.c
	$(CC) $(CFLAGS) -o phone-workfuncs.o -c phone-workfuncs.c

clean:
	rm -rf phone-client.? phone-server.? phonebook phoneserver *.o

## End of file.

