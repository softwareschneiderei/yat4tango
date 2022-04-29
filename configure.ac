#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(yat4tango, 1.10.3, nicolas.leclercq@synchrotron-soleil.fr)
AC_CONFIG_SRCDIR([src/DeviceTask.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Werror -Wall])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
#AM_PROG_AR
AC_PROG_LIBTOOL

# Check gcc release
# TODO: move in external m4 file
dnl Determine whether we have gcc of a particular version or later,
dnl based on major, minor, patchlevel versions and date.
dnl
dnl gcc_AC_HAVE_GCC_VERSION(MAJOR_VERSION, MINOR_VERSION, PATCH_LEVEL) 
dnl   

AC_DEFUN([gcc_AC_HAVE_GCC_VERSION],
         [AC_CACHE_CHECK([for gcc compiler release (at least version $1.$2.$3)],
                         ac_cv_gcc_version_$1_$2_$3,
                         [if test x$GCC = x ; then 
                             ac_cv_gcc_version_$1_$2_$3=no
			  else
			     ac_gcc_date=0 ; 
                             AC_EGREP_CPP(yes,
    				      	  [#define HAVE_GCC_VERSION(MAJOR, MINOR, MICRO, DATE) \
					  (__GNUC__ > (MAJOR) \
					  || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ > (MINOR)) \
					  || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ == (MINOR) \
    				          && __GNUC_PATCHLEVEL__ > (MICRO)) \
					  || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ == (MINOR) \
    				          && __GNUC_PATCHLEVEL__ == (MICRO) && ${ac_gcc_date}L >= (DATE)))
                                          #if HAVE_GCC_VERSION($1,$2,$3,0)
                                          yes
                                          #endif],
					  ac_cv_gcc_version_$1_$2_$3=yes,
					  ac_cv_gcc_version_$1_$2_$3=no)
		           fi
	                  ])
	  if test x$ac_cv_gcc_version_$1_$2_$3 = xyes; then
	       AC_DEFINE_UNQUOTED(HAVE_GCC_VERSION_$1_$2_$3,
			          1,
				  [Define to 1 if we have gcc $1.$2.$3 ])
	  fi        
])

if test "$CXX" = "c++" -o "$CXX" = "g++"; then
   gcc_AC_HAVE_GCC_VERSION(3,2,0)
   if test "x$ac_cv_gcc_version_3_2_0" = "xno"; then
      AC_MSG_ERROR([gcc release not supported. Use gcc 3.2.0 or above.], -1)
   fi
   gcc_AC_HAVE_GCC_VERSION(4,3,0)
   if test "x$ac_cv_gcc_version_4_3_0" = "xyes"; then
      CPP_ELEVEN="-std=c++0x"
   else
      CPP_ELEVEN=""
   fi
fi
AC_SUBST(CPP_ELEVEN)
# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h string.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday inet_ntoa memset select socket])

# Dependencies
PKG_CHECK_MODULES([YAT], [yat >= 1.7.18])
PKG_CHECK_MODULES([TANGO], [tango >= 7.2.6])

YAT_VERSION=`eval pkg-config --modversion yat`
YAT_ROOT=`eval pkg-config --variable=prefix yat`

TANGO_VERSION=`eval pkg-config --modversion tango`
TANGO_ROOT=`eval pkg-config --variable=prefix tango`

AC_CONFIG_FILES([
	Makefile
	yat4tango.pc
	include/Makefile
	src/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([

    package.........: ${PACKAGE}
    version.........: ${VERSION}

    Compiler........: ${CXX}
    Compiler flags..: ${CXXFLAGS} ${AM_CXXFLAGS}

    YAT.............: ${YAT_ROOT}
    YAT VERSION.....: ${YAT_VERSION}

    TANGO...........: ${TANGO_ROOT}
    TANGO VERSION...: ${TANGO_VERSION}
])
