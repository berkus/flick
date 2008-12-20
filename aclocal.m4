dnl
dnl Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
dnl the Computer Systems Laboratory at the University of Utah (CSL).
dnl
dnl This file is part of Flick, the Flexible IDL Compiler Kit.
dnl
dnl Flick is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl Flick is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with Flick; see the file COPYING.  If not, write to
dnl the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
dnl

dnl Many of the functions in this file are slight modifications of standard
dnl Autoconf macros; see Autoconf's `acspecific.m4' file.  Autoconf is free
dnl software, distributed under the GNU General Public Licence, and copyrighted
dnl by the Free Software Foundation.
dnl
dnl Copyright (C) 1992, 93, 94, 95, 96, 1998 Free Software Foundation, Inc.

dnl ###########################################################################

dnl
dnl `AC_C_BYTE_ORDER' is like Autoconf 2.12's standard `AC_C_BIGENDIAN' macro,
dnl except that `AC_C_BYTE_ORDER' results in a substitution (AC_SUBST), not a
dnl definition (AC_DEFINE).
dnl
AC_DEFUN(AC_C_BYTE_ORDER,
[AC_CACHE_CHECK(byte ordering, ac_cv_c_byte_order,
[ac_cv_c_byte_order=unknown
# See if sys/param.h defines the BYTE_ORDER macro.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if !BYTE_ORDER || !BIG_ENDIAN || !LITTLE_ENDIAN
 bogus endian macros
#endif], [# It does; now see whether it defined to BIG_ENDIAN or not.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if BYTE_ORDER != BIG_ENDIAN
 not big endian
#endif], ac_cv_c_byte_order=big, ac_cv_c_byte_order=little)])
if test $ac_cv_c_byte_order = unknown; then
AC_TRY_RUN([main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
}],
  ac_cv_c_byte_order=little,
  ac_cv_c_byte_order=big,
  ac_cv_c_byte_order=unknown)
fi])
case "$ac_cv_c_byte_order" in
  big)    byte_order=BIG_ENDIAN ;;
  little) byte_order=LITTLE_ENDIAN ;;
  *)      byte_order=UNKNOWN ;;
esac
AC_SUBST(byte_order)dnl
])

dnl ###########################################################################

dnl
dnl `FLICK_FUNC_ALLOCA' is like Autoconf 2.13's standard `AC_FUNC_ALLOCA'
dnl macro, except that `FLICK_FUNC_ALLOCA' avoids making definitions
dnl (`AC_DEFINE's) and performing tests that we don't need.  By avoiding
dnl unneeded defines, we keep our compilation command lines cleaner.
dnl
dnl Rather than define `HAVE_ALLOCA_H', we set the output variable
dnl `have_alloca_h' to 1 or 0.  We don't define `HAVE_ALLOCA' or `C_ALLOCA'.
dnl Finally, we omit all the extra tests that `AC_FUNC_ALLOCA' does if `alloca'
dnl is discovered not to work.  Flick-generated code needs a working `alloca'.
dnl
AC_DEFUN(FLICK_FUNC_ALLOCA,
[AC_REQUIRE_CPP()dnl Set CPP; we run AC_EGREP_CPP conditionally.
# The Ultrix 4.2 mips builtin alloca declared by alloca.h only works
# for constant arguments.  Useless!
AC_CACHE_CHECK([for working alloca.h], ac_cv_header_alloca_h,
[AC_TRY_LINK([#include <alloca.h>], [char *p = alloca(2 * sizeof(int));],
  ac_cv_header_alloca_h=yes, ac_cv_header_alloca_h=no)])
dnl if test $ac_cv_header_alloca_h = yes; then
dnl   AC_DEFINE(HAVE_ALLOCA_H)
dnl fi
if test $ac_cv_header_alloca_h = yes; then
  have_alloca_h=1
else
  have_alloca_h=0
fi

AC_CACHE_CHECK([for alloca], ac_cv_func_alloca_works,
[AC_TRY_LINK([
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if $have_alloca_h
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif
], [char *p = (char *) alloca(1);],
  ac_cv_func_alloca_works=yes, ac_cv_func_alloca_works=no)])
dnl if test $ac_cv_func_alloca_works = yes; then
dnl   AC_DEFINE(HAVE_ALLOCA)
dnl fi

if test $ac_cv_func_alloca_works = no; then
  # The SVR3 libPW and SVR4 libucb both contain incompatible functions
  # that cause trouble.  Some versions do not even contain alloca or
  # contain a buggy version.  If you still want to use their alloca,
  # use ar to extract alloca.o from them instead of compiling alloca.c.
  ALLOCA=alloca.${ac_objext}
dnl  AC_DEFINE(C_ALLOCA)
dnl
dnl Removed extra tests performed by `AC_FUNC_ALLOCA'.  We don't need them.
dnl
fi
AC_SUBST(have_alloca_h)dnl
AC_SUBST(ALLOCA)dnl
])

dnl ###########################################################################

dnl
dnl `AC_CHECK_STDCPLUSPLUS_LIB' is a version of `AC_CHECK_LIB' that doesn't
dnl construct its cache variable name from the library name.  (The library we
dnl want to check is `stdc++', but `+' isn't allowed within a shell variable
dnl name.)
dnl
AC_DEFUN(AC_CHECK_STDCPLUSPLUS_LIB,
[AC_MSG_CHECKING([for -l$1])
AC_CACHE_VAL(ac_cv_lib_stdcplusplus,
[ac_save_LIBS="$LIBS"
LIBS="-l$1 $5 $LIBS"
AC_TRY_LINK(, [$2()], eval "ac_cv_lib_stdcplusplus=yes", eval "ac_cv_lib_stdcplusplus=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo '$ac_cv_lib_'stdcplusplus`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], ,
[changequote(, )dnl
  ac_tr_lib=HAVE_LIB`echo $1 | tr '[a-z]' '[A-Z]'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_lib)
  LIBS="-l$1 $LIBS"
], [$3])
else
  AC_MSG_RESULT(no)
ifelse([$4], , , [$4
])dnl
fi
])

dnl ###########################################################################

dnl
dnl `AC_CHECK_TARGET_HEADER' is like the standard `AC_CHECK_HEADER', but looks
dnl for the header file using the target-platform C preprocessor instead of the
dnl host-platform CPP.
dnl
dnl AC_CHECK_TARGET_HEADER(HEADER-FILE,
dnl                        [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl First, check for the target preprocessor.  XXX --- The name is hardwired!
dnl If we can't find/execute the target CPP, execute ACTION-IF-NOT-FOUND, else
dnl check if the system header file HEADER-FILE exists for the target platform.
dnl If it exists, execute the shell commands ACTION-IF-FOUND; otherwise execute
dnl ACTION-IF-NOT-FOUND.
dnl
AC_DEFUN(AC_CHECK_TARGET_HEADER,
[ac_save_ac_cpp="$ac_cpp"
if test "$host_alias" != "$target_alias"; then
  if test "$ac_target_cpp" = ""; then
    ac_cpp="${target_alias}-gcc -E"
    AC_CHECK_PROG(ac_target_cpp, $ac_cpp, $ac_cpp, no)
  else
    ac_cpp="$ac_target_cpp"
  fi
fi
if test "$ac_target_cpp" = "no"; then
  $3
else
  AC_CHECK_HEADER($1, [$2], [$3])
fi
ac_cpp="$ac_save_ac_cpp"])

dnl
dnl `AC_CHECK_TARGET_HEADER_USING_ENV' is like `AC_CHECK_TARGET_HEADER' above,
dnl but can look for the header file in a directory specified by an environment
dnl variable.
dnl
dnl AC_CHECK_TARGET_HEADER_USING_ENV(HEADER-FILE, ENVVAR, DIRECTORY,
dnl                                  [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl If ENVVAR is set, temporarily add DIRECTORY as a directory to be searched
dnl for system header files.  Then, whether or not ENVVAR is set, check if the
dnl system header file HEADER-FILE exists.  If it exists, execute the shell
dnl commands ACTION-IF-FOUND; otherwise execute ACTION-IF-NOT-FOUND.
dnl
AC_DEFUN(AC_CHECK_TARGET_HEADER_USING_ENV,
[ac_save_CPPFLAGS="$CPPFLAGS"
if test "${$2+set}" = "set"; then
  CPPFLAGS="-I$3 $CPPFLAGS"
fi
AC_CHECK_TARGET_HEADER($1, [$4], [$5])
CPPFLAGS="$ac_save_CPPFLAGS"])

dnl ###########################################################################

dnl End of file.

