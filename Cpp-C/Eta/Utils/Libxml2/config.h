#if defined(WIN32)

#ifndef __LIBXML_WIN32_CONFIG__
#define __LIBXML_WIN32_CONFIG__

#define HAVE_CTYPE_H
#define HAVE_STDARG_H
#define HAVE_MALLOC_H
#define HAVE_ERRNO_H

#if defined(_WIN32_WCE)
#undef HAVE_ERRNO_H
#include <windows.h>
#include "wincecompat.h"
#else
#define HAVE_SYS_STAT_H
#define HAVE__STAT
#define HAVE_STAT
#define HAVE_STDLIB_H
#define HAVE_TIME_H
#define HAVE_FCNTL_H
#include <io.h>
#include <direct.h>
#endif

#include <libxml/xmlversion.h>

#ifndef ICONV_CONST
#define ICONV_CONST const
#endif

#ifdef NEED_SOCKETS
#include <wsockcompat.h>
#endif

/*
 * Windows platforms may define except 
 */
#undef except

#define HAVE_ISINF
#define HAVE_ISNAN
#include <math.h>
#if defined(_MSC_VER) || defined(__BORLANDC__)
/* MS C-runtime has functions which can be used in order to determine if
   a given floating-point variable contains NaN, (+-)INF. These are 
   preferred, because floating-point technology is considered propriatary
   by MS and we can assume that their functions know more about their 
   oddities than we do. */
#include <float.h>
/* Bjorn Reese figured a quite nice construct for isinf() using the _fpclass
   function. */
#ifndef isinf
#define isinf(d) ((_fpclass(d) == _FPCLASS_PINF) ? 1 \
	: ((_fpclass(d) == _FPCLASS_NINF) ? -1 : 0))
#endif
/* _isnan(x) returns nonzero if (x == NaN) and zero otherwise. */
#ifndef isnan
#define isnan(d) (_isnan(d))
#endif
#else /* _MSC_VER */
#ifndef isinf
static int isinf (double d) {
    int expon = 0;
    double val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 1;
        } else if (val == -0.5) {
            return -1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
#endif
#ifndef isnan
static int isnan (double d) {
    int expon = 0;
    double val = frexp (d, &expon);
    if (expon == 1025) {
        if (val == 0.5) {
            return 0;
        } else if (val == -0.5) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}
#endif
#endif /* _MSC_VER */

#if defined(_MSC_VER)
#define mkdir(p,m) _mkdir(p)
#include <stdio.h>
#ifndef snprintf
	#define snprintf _snprintf
#endif
#if _MSC_VER < 1500
#define vsnprintf(b,c,f,a) _vsnprintf(b,c,f,a)
#endif
#elif defined(__MINGW32__)
#define mkdir(p,m) _mkdir(p)
#endif

/* Threading API to use should be specified here for compatibility reasons.
   This is however best specified on the compiler's command-line. */
#if defined(LIBXML_THREAD_ENABLED)
#if !defined(HAVE_PTHREAD_H) && !defined(HAVE_WIN32_THREADS) && !defined(_WIN32_WCE)
#define HAVE_WIN32_THREADS
#endif
#endif

/* Some third-party libraries far from our control assume the following
   is defined, which it is not if we don't include windows.h. */
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(TRUE)
#define TRUE (!(FALSE))
#endif

#endif /* __LIBXML_WIN32_CONFIG__ */

#else  /* defined(WIN32) */

/* Linux/Solaris configuration */

/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */
#define PACKAGE "libxml2"
#define VERSION "2.7.8"
/* #undef HAVE_LIBZ */
/* #undef HAVE_LIBM */

#if defined(LINUX)

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define if isinf is there */
#define HAVE_ISINF /**/

#else

/* Define to 1 if you have the `fpclass' function. */
#define HAVE_FPCLASS 1

/* Define to 1 if you have the <ieeefp.h> header file. */
#define HAVE_IEEEFP_H 1

/* Define to 1 if you have the `isnand' function. */
#define HAVE_ISNAND 1

/* Define to 1 if you have the <nan.h> header file. */
#define HAVE_NAN_H 1

/* Define to 1 if you have the `_stat' function. */
#define HAVE__STAT 1

#endif

#define HAVE_ISNAN /**/
/* #undef HAVE_LIBHISTORY */
/* #undef HAVE_LIBREADLINE */
#define HAVE_LIBPTHREAD /**/
#define HAVE_PTHREAD_H /**/

/* Define if IPV6 support is there */
/* #undef SUPPORT_IP6 */

/* Define if getaddrinfo is there */
/* #undef HAVE_GETADDRINFO */

/* Define to 1 if you have the <ansidecl.h> header file. */
/* #undef HAVE_ANSIDECL_H */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <arpa/nameser.h> header file. */
#define HAVE_ARPA_NAMESER_H 1

/* Whether struct sockaddr::__ss_family exists */
/* #undef HAVE_BROKEN_SS_FAMILY */

/* Define to 1 if you have the `class' function. */
/* #undef HAVE_CLASS */

/* Define to 1 if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Have dlopen based dso */
/* #undef HAVE_DLOPEN */

/* Define to 1 if you have the <dl.h> header file. */
/* #undef HAVE_DL_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `finite' function. */
#define HAVE_FINITE 1

/* Define to 1 if you have the <float.h> header file. */
#define HAVE_FLOAT_H 1


/* Define to 1 if you have the `fprintf' function. */
#define HAVE_FPRINTF 1

/* Define to 1 if you have the `fp_class' function. */
/* #undef HAVE_FP_CLASS */

/* Define to 1 if you have the <fp_class.h> header file. */
/* #undef HAVE_FP_CLASS_H */

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define if getaddrinfo is there */
/* #undef HAVE_GETADDRINFO */

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1


/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <inttypes.h.h> header file. */
/* #undef HAVE_INTTYPES_H_H */


/* Define if isnan is there */
#define HAVE_ISNAN /**/


/* Define if history library is there (-lhistory) */
/* #undef HAVE_LIBHISTORY */

/* Define if pthread library is there (-lpthread) */
#define HAVE_LIBPTHREAD /**/

/* Define if readline library is there (-lreadline) */
/* #undef HAVE_LIBREADLINE */

/* Have compression library */
/* #undef HAVE_LIBZ */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `localtime' function. */
#define HAVE_LOCALTIME 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1


/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <poll.h> header file. */
#define HAVE_POLL_H 1

/* Define to 1 if you have the `printf' function. */
#define HAVE_PRINTF 1

/* Define if <pthread.h> is there */
#define HAVE_PTHREAD_H /**/

/* Define to 1 if you have the <resolv.h> header file. */
#define HAVE_RESOLV_H 1

/* Have shl_load based dso */
/* #undef HAVE_SHLLOAD */

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `sprintf' function. */
#define HAVE_SPRINTF 1

/* Define to 1 if you have the `sscanf' function. */
#define HAVE_SSCANF 1

/* Define to 1 if you have the `stat' function. */
#define HAVE_STAT 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1


/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Whether va_copy() is available */
#define HAVE_VA_COPY 1

/* Define to 1 if you have the `vfprintf' function. */
#define HAVE_VFPRINTF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsprintf' function. */
#define HAVE_VSPRINTF 1

/* Define to 1 if you have the <zlib.h> header file. */
/* #undef HAVE_ZLIB_H */


/* Whether __va_copy() is available */
/* #undef HAVE___VA_COPY */

/* Define as const if the declaration of iconv() needs const. */
/* #undef ICONV_CONST */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libxml2"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Support for IPv6 */
/* #undef SUPPORT_IP6 */

/* Version number of package */
#define VERSION "2.7.8"

/* Determine what socket length (socklen_t) data type is */
#define XML_SOCKLEN_T socklen_t

/* Using the Win32 Socket implementation */
/* #undef _WINSOCKAPI_ */

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Win32 Std C name mangling work-around */
/* #undef snprintf */

/* ss_family is not defined here, use __ss_family instead */
/* #undef ss_family */

/* Win32 Std C name mangling work-around */
/* #undef vsnprintf */

#endif  /* defined(WIN32) */
