#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

/****************************************************************************
*  UBUF is the structure of message exchanged between server and clients.
*
*    The 'buf' part of the buffer is variable lenght up to max of 1024.
*    The 'key' field is used by the server for sequence identification.
*    The 'seq' field is used by the client for sequence identification.
*
*  Client's message to server contain a key value that is the same as the
*  key value of the previous message received from the server.  Similarly,
*  the server's message to client contains a seq value that is the same
*  as the seq value of the previous message from the client.
*
*  The buf field is logically partitioned into two parts by the len field.
*  The len field indicate the size of the first part of the buffer starting
*  at buf[0].  The rest of the buffer is the second field.  In some cases
*  both fields can contain information.
*
****************************************************************************/

#define UBUF_HSIZE 12                           /* 12 bytes for the header */
/*#define UBUF_SPACE 1024			        /* maximum payload.        */
#define UBUF_SPACE 147080			        /* Changed by xxfan */

typedef struct UBUF {            char   cmd;  /* message code.             */
                        unsigned char   sum;  /* message checksum.         */
                        unsigned short  key;  /* message key.              */
                        unsigned short  seq;  /* message sequence number.  */
                        unsigned short  len;  /* number of bytes in buf 1. */
                        unsigned long   pos;  /* location in the file.     */

                        char   buf[UBUF_SPACE];
                    } UBUF;

/* definition of cmd */

#define CC_VERSION	0x10	/* return server's version string.	*/
#define CC_ERR          0x40    /* error response from server.          */
#define CC_GET_DIR      0x41    /* get a directory listing.             */
#define CC_GET_FILE     0x42    /* get a file.                          */
#define CC_UP_LOAD      0x43    /* open a file for writing.             */
#define CC_INSTALL      0x44    /* close a file opened for writing.     */
#define CC_DEL_FILE     0x45    /* delete a file.                       */
#define CC_DEL_DIR      0x46    /* delete a directory.                  */
#define CC_GET_PRO      0x47    /* get directory protection.            */
#define CC_SET_PRO      0x48    /* set directory protection.            */
#define CC_MAKE_DIR     0x49    /* create a directory.                  */
#define CC_BYE          0x4A    /* finish a session.                    */

/****************************************************************************
*  RDIRENT is the structure of a directory entry contained in a .FSP_CONTENT
*  file.  Each entry contains a 4 bytes quantity 'time', a 4 bytes quentity
*  'size', and 1 byte of 'type'.  Then followed by x number of bytes of
*  'name'.  'name' is null terminated.  Then followed by enough number of
*  padding to fill to an 4-byte boundary.  At this point, if the next entry
*  to follow will spread across 1k boundary, then two possible things will
*  happen.  1) if the header fits between this entry and the 1k boundary,
*  a complete header will be filled in with a 'type' set to RDTYPE_SKIP.
*  And then enough bytes to padd to 1k boundary.  2) if the header does
*  not fit, then simply pad to the 1k boundary.  This will make sure that
*  messages carrying directory information carry only complete directory
*  entries and no fragmented entries.  The last entry is type RDTYPE_END.
****************************************************************************/

#define RDHSIZE (2*sizeof(unsigned long)+sizeof(unsigned char))

typedef struct RDIRENT { unsigned long  time;
                         unsigned long  size;
                         unsigned char  type;
                         char        name[1]; } RDIRENT;

#define RDTYPE_END      0x00
#define RDTYPE_FILE     0x01
#define RDTYPE_DIR      0x02
#define RDTYPE_SKIP     0x2A

#define NULLP ((void *) 0)


/****************************************************************************
* These structures are used to implement a opendir/readdir mechanism similar
* to that of the normal opendir/reader mechanism in unix.
****************************************************************************/

typedef struct DDLIST {	struct DDLIST *next;
			char          *path;
			RDIRENT  **dep_root;
			int         ref_cnt; } DDLIST;

typedef struct RDIR { DDLIST   *ddp;
		      RDIRENT **dep; } RDIR;

typedef struct rdirent { unsigned long  d_fileno;
			 unsigned short d_reclen;
			 unsigned short d_namlen;
			 char          *d_name; } rdirent;

UBUF *client_interact(unsigned cmd, unsigned long pos, unsigned l1, unsigned char *p1,unsigned l2, unsigned char *p2);
void client_intr(int);
void init_client(char *host,int port,int myport);
int _x_select(fd_set *rf, long tt);
int _x_udp(int *port);
int _x_adr(char *host,int port,struct sockaddr_in *his);
