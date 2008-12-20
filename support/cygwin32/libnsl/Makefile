#
# Copyright (c) 1998 The University of Utah and
# the Computer Systems Laboratory at the University of Utah (CSL).
# All rights reserved.
#
# Permission to use, copy, modify and distribute this software is hereby
# granted provided that (1) source code retains these copyright, permission,
# and disclaimer notices, and (2) redistributions including binaries
# reproduce the notices in supporting documentation.
#
# THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
# IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
# ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#
# CSL requests users of this software to return to csl-dist@cs.utah.edu any
# improvements that they make and grant CSL redistribution rights.
#

###############################################################################

# What are we building?
#
TARGET = libnsl.a

# The objects that make up our `libnsl'.
#
OBJECTS = auth_none.o auth_unix.o authuxprot.o bindrsvprt.o clnt_gen.o \
	  clnt_perr.o clnt_raw.o clnt_simp.o clnt_tcp.o clnt_udp.o \
	  get_myaddr.o getrpcport.o pm_getmaps.o pm_getport.o pmap_clnt.o \
	  pmap_prot.o pmap_prot2.o pmap_rmt.o rpc_cmsg.o rpc_common.o \
	  rpc_dtable.o rpc_prot.o svc.o svc_auth.o svc_authux.o svc_raw.o \
	  svc_run.o svc_simple.o svc_tcp.o svc_udp.o xdr.o xdr_array.o \
	  xdr_float.o xdr_mem.o xdr_rec.o xdr_ref.o xdr_stdio.o

# `-imacros include/_.h' tells GCC to process `include/_.h' before processing
# the regular input file.  Since we need `-imacros', we require GCC.
#
CC     = gcc
CFLAGS = -O -I../include -Iinclude -imacros include/_.h

###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rv $@ $?
	ranlib $@

clean:
	$(RM) $(TARGET) $(OBJECTS)

###############################################################################

## End of file.

