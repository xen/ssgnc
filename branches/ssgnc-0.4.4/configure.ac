#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ssgnc], [0.4.4], [syata@acm.org])
AC_CONFIG_SRCDIR([include/ssgnc.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_LANG_CPLUSPLUS
AC_C_INLINE

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDBOOL

AC_CHECK_HEADERS([fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

AC_FUNC_MMAP

AC_CHECK_FUNCS([strtol] [munmap] [localtime_r])

# Checks for library functions.

AC_CONFIG_FILES([Makefile] [lib/Makefile] [tools/Makefile]
	[cgi/Makefile] [tests/Makefile])
AC_OUTPUT
