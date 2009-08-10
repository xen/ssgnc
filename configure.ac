#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([ssgnc], [0.2.1], [syata@acm.org])
AC_CONFIG_SRCDIR([src/build-vocab-dic.cc])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_LANG_CPLUSPLUS

# Checks for libraries.
AC_CHECK_LIB([boost_program_options], [main], ,
	[AC_MSG_ERROR(boost_program_options: not found)])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sys/time.h unistd.h])

# Checks for C++ header files.
AC_CHECK_HEADERS([dawgdic/dawg.h], ,
	[AC_MSG_ERROR(dawgdic 0.30 or later: not found)])

AC_CHECK_HEADERS([boost/algorithm/string.hpp], ,
	[AC_MSG_ERROR(boost/algorithm/string.hpp: not found)])
AC_CHECK_HEADERS([boost/program_options.hpp], ,
	[AC_MSG_ERROR(boost/program_options.hpp: not found)])
AC_CHECK_HEADERS([boost/shared_ptr.hpp], ,
	[AC_MSG_ERROR(boost/shared_ptr.hpp: not found)])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SSIZE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([clock_gettime gettimeofday memmove munmap strchr strtol])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_OUTPUT
