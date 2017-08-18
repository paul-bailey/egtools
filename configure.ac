#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([egtools], [1.0], [baileyp2012@gmail.com])

AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build/m4])
AC_CONFIG_SRCDIR([egscripts/egnum.sh])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

LT_PREREQ([2.2])
LT_INIT([dlopen])

AM_INIT_AUTOMAKE

# Checks for programs
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CC_C_O
AC_PROG_MKDIR_P
AC_PROG_GREP
AC_PROG_SED
AC_PROG_AWK

# TODO: all progs called by sh scripts
AC_CHECK_PROG(have_dc,dc,yes,no)
AM_CONDITIONAL([BUILD_MATH], [test "x${have_dc}" = "xyes"])
AC_CHECK_PROG(have_lp,lp,yes,no)
AM_CONDITIONAL([BUILD_PLP], [test "x${have_lp}" = "xyes"])
AC_CHECK_PROG(have_pr,pr,yes,no)
AM_CONDITIONAL([BUILD_PRINTIT], [test "x${have_pr}" = "xyes"])

# TODO: AC_CHECK_HEADER for progs in small_c/
fail_headers='time.h stdio.h stdint.h stdlib.h sys/types.h
unistd.h string.h math.h'
for i in $fail_headers; do
 AC_CHECK_HEADER([$i], ,[AC_MSG_ERROR([$i missing])])
done

# TODO: AC_CHECK_TYPE for types in small_c/, esp. sizeof.c
have_loff_t=no
AC_CHECK_TYPE(loff_t,have_loff_t=yes)
AM_CONDITIONAL([HAVE_LOFF_T], [test "x${have_loff_t}" = "xyes"])

# TODO: Better to do a AM_CONDITIONAL on compiling egfortune
AC_CHECK_LIB(m, fabs,,[AC_MSG_ERROR([math missing])])

# TODO: AC_CHECK_FUNC
AC_CONFIG_FILES([Makefile
                 egscripts/Makefile
                 man/Makefile
                 include/Makefile
                 lib/Makefile
                 misc_install/Makefile
                 tests/Makefile
                 small_c/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'

Now type 'make @<:@<target>@:>@'
    where the optional <target> is:
        all         - build all binaries
        install     - install everything

--------------------------------------------------"
