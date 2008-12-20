#!/bin/sh 
#
# Copyright (c) 1995, 1996 The University of Utah and
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

pdl=${FLICK_C_PDL-@FLICK_C_PDL@}
pdlflags=
files=

user=/dev/null
server=/dev/null
header=/dev/null
sheader=/dev/null
iheader=/dev/null

# If an argument to this shell script contains whitespace,
# then we will screw up.  PDL will see it as multiple arguments.

until [ $# -eq 0 ]
do
    case "$1" in
	-i	) echo -i option not supported yet; exit 1;;
	-user   ) user="$2"; shift; shift;;
	-server ) server="$2"; shift; shift;;
	-header ) header="$2"; shift; shift;;
	-sheader ) sheader="$2"; shift; shift;;
	-iheader ) iheader="$2"; shift; shift;;
	-prefix | -subrprefix ) echo -prefix not supported yet; exit 1;;

	-MD ) sawMD=1; pdlflags="$pdlflags -Xcpp $1"; shift;;
	-imacros ) pdlflags="$pdlflags -Xcpp '$1 $2'"; shift; shift;;
	-pdl) pdl="$2"; shift; shift;;
	-* ) pdlflags="$pdlflags -Xcpp $1"; shift;;
	* ) files="$files $1"; shift;;
    esac
done

for file in $files
do
if $pdl $pdlflags -client -o $user -h $header <<EOF
[read(mig,"$file")];
[write(mach3mig)];
EOF
then true; else exit 1; fi
if $pdl $pdlflags -server -o $server -h $sheader <<EOF
[read(mig,"$file")];
[write(mach3mig)];
EOF
then true; else exit 1; fi

    if [ $sawMD ]
    then
    	echo "Warning: don't know how to deal properly with -MD"
    fi
done

exit 0
