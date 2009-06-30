#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(ssgnc, 0.0.2, syata@acm.org)
AC_CONFIG_SRCDIR([src/build-dic.cc])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

AC_LANG_CPLUSPLUS

# Checks for libraries.

# Checks for header files.

# Checks for C++ header files.
AC_CHECK_HEADERS([dawgdic/dawg.h], ,
	[AC_MSG_ERROR(dawgdic 0.30 or later: not found)])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([munmap])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strtol])

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/time.h])

AC_FUNC_MMAP
AC_FUNC_STAT

AC_HEADER_STDC
AC_HEADER_TIME

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
