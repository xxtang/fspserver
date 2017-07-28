#ifndef _FSP_TWEAK_H_
#define _FSP_TWEAK_H_ 1

#include <sys/types.h>
#include <sysexits.h>

#ifndef HAVE_FSEEKO
/* fallback to old fseek if no fseeko is available */
#define fseeko fseek
#endif

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

#define FSP_STAT stat

#define fexist(A) (!access(A,F_OK))
#define touch(A) close(open(A,O_CREAT,0600))

#ifdef min
#undef min
#endif
#define min(x,y)  ((x) < (y) ? (x) : (y))

#ifndef SECSPERDAY
#define SECSPERDAY (long)60*60*24
#endif
#ifndef DAYSPERNYEAR
#define DAYSPERNYEAR 365
#endif

#if defined(HAVE_D_INO) && !defined(HAVE_D_FILENO)
#define d_fileno d_ino
#else
#if !defined(HAVE_D_INO) && defined(HAVE_D_FILENO)
#define d_ino d_fileno
#endif
#endif

/****************************************************************************
*  Macros to read and write multi-byte fields from the message header.
****************************************************************************/

#define BB_READ4(V) ntohl(*(const u_int32_t *)(V))
#define BB_WRITE4(V,A) *(u_int32_t *)(V) = htonl(A)

/* there is an integer type of size 2 */
#define BB_READ2(V) ntohs(*(const u_int16_t *)(V))
#define BB_WRITE2(V,A) *(u_int16_t *)(V) = htons(A)

#endif /* _FSP_TWEAK_H_ */
