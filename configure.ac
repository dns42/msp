# -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mwrt], [0.1], [daniel.stodden@gmail.com])
AC_CONFIG_SRCDIR([msp/msp.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign nostdinc])

# Checks for programs.
AC_LANG_WERROR
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB

AC_PATH_PROGS([LUAC], [luac5.1])
AS_IF([test "x$LUAC" = x],
      [AC_MSG_ERROR([Missing Lua 5.1 compiler])])
AC_SUBST([LUAC])

AC_CHECK_TOOL([OBJCOPY], [objcopy])
AS_IF([test "x$OBJCOPY" = x],
      [AC_MSG_ERROR([Missing host objcopy])])
AC_SUBST([OBJCOPY])

# Checks for libraries.
LT_INIT
PKG_CHECK_MODULES([LUA], [lua5.1])

# Checks for header files.
AC_CHECK_HEADERS([dnl
fcntl.h dnl
stdint.h dnl
stddef.h dnl
string.h dnl
termios.h dnl
unistd.h dnl
netdb.h dnl
syslog.h dnl
sys/time.h dnl
sys/socket.h dnl
sys/ioctl.h dnl
netinet/in.h dnl
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([dnl
gettimeofday dnl
memset dnl
memmove dnl
strdup dnl
strcasecmp dnl
strncasecmp dnl
strtol dnl
strtoul dnl
strerror dnl
select dnl
socket dns
])
AC_FUNC_MALLOC
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE

AC_OUTPUT([
Makefile
crt/Makefile
msp/Makefile
nrx/Makefile
ntx/Makefile
mcc/Makefile
])
