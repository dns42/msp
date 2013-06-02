# -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([msp], [0.1], [daniel.stodden@gmail.com])
AC_CONFIG_SRCDIR([msp/msp.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_LANG_WERROR
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([dnl
fcntl.h dnl
stdint.h dnl
string.h dnl
termios.h dnl
unistd.h dnl
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([dnl
memset dnl
select dnl
])

AC_OUTPUT([
Makefile
msp/Makefile
])
