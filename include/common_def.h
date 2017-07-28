    /*********************************************************************\
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *  Copyright (c) 2003-2009 by Radim Kolar                             *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#ifndef _FSP_COMMON_DEF_H_
#define _FSP_COMMON_DEF_H_ 1

#include <stdio.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

/****************************************************************************
*  UBUF is the structure of message exchanged between server and clients.
*
*    The 'buf' part of the buffer is variable length up to max of 1024.
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

#define UBUF_HSIZE 12                      /* 12 bytes for the FSP header.    */
#define UBUF_SPACE 147080			   /* maximum standard payload.       */
#define DEFAULT_SPACE 147080-20-8-12	   /* Max packet size for standard    */
                                           /* 1492 pppOE mtu ethernet.        */
					   /*  20 bytes is IP header          */
                                           /*   8 bytes is UDP header         */
                                           /*  12 bytes is FSP header         */
#define UBUF_MAXSPACE 147080                /* maximum payload supported by    */
                                           /*  server. 3 max packets for 1500 */
                                           /*  mtu ethernet.                  */

#define NBSIZE (UBUF_MAXSPACE+UBUF_SPACE)

typedef struct UBUF {   unsigned char       cmd; /* message code.             */
                        unsigned char       sum; /* message checksum.         */
                        unsigned char bb_key[2]; /* message key.              */
                        unsigned char bb_seq[2]; /* message sequence number.  */
                        unsigned char bb_len[2]; /* number of bytes in buf 1. */
                        unsigned char bb_pos[4]; /* location in the file.     */

                        char   buf[UBUF_MAXSPACE];
                    } UBUF;

/* definition of cmds */

#define CC_VERSION	0x10	/* return server's version string.	*/
#define CC_INFO		0x11	/* return server's extended info block  */
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
#define CC_GRAB_FILE    0x4B	/* atomic get+delete a file.		*/
#define CC_GRAB_DONE    0x4C	/* atomic get+delete a file done.	*/
#define CC_STAT         0x4D    /* get information about file.          */
#define CC_RENAME       0x4E    /* rename file or directory.            */
#define CC_CH_PASSWD    0x4F    /* change password                      */
#define CC_LIMIT	0x80	/* # > 0x7f for future cntrl blk ext.   */
#define CC_TEST		0x81	/* reserved for testing			*/

/* definition of global bitfield for version information */
/* Global information is also going to be a bit vector   */
#define VER_BYTES	1	/* currently only 8 bits or less of info  */
#define VER_LOG		0x01	/* does the server do logging             */
#define VER_READONLY	0x02	/* is the server in readonly mode         */
#define VER_REVNAME	0x04	/* does the server refuse non reversables */
#define VER_PRIVMODE	0x08	/* Is the server being run 'private' mode */
#define VER_THRUPUT	0x10	/* does the server enforce thruput control*/
#define VER_XTRADATA    0x20    /* server accept packets with xtra data  */

/* definition of directory bitfield for directory information */
/* directory information is just going to be a bitfield encoding
 * of which protection bits are set/unset
 */
#define PRO_BYTES	1	/* currently only 8 bits or less of info  */
#define DIR_OWNER	0x01	/* does caller own directory              */
#define DIR_DEL		0x02	/* can files be deleted from this dir     */
#define DIR_ADD		0x04	/* can files be added to this dir         */
#define DIR_MKDIR	0x08	/* can new subdirectories be created      */
#define DIR_GET		0x10	/* are files readable by non-owners?      */
#define DIR_README	0x20	/* does this dir contain an readme file?  */
#define DIR_LIST        0x40    /* public can list directory              */
#define DIR_RENAME      0x80    /* can files be renamed in this dir       */

/****************************************************************************
*  RDIRENT is the structure of a directory entry contained in a .FSP_CONTENT
*  file.  Each entry contains a 4 bytes quantity 'time', a 4 bytes quantity
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

#define RDHSIZE (2*4+1)

typedef struct RDIRENT { unsigned char bb_time[4];
                         unsigned char bb_size[4];
                         unsigned char type      ;
                         char          name[1]   ; } RDIRENT;

#define RDTYPE_END      0x00
#define RDTYPE_FILE     0x01
#define RDTYPE_DIR      0x02
#define RDTYPE_SKIP     0x2A

#define NULLP ((char *) 0)

#define MIN_DELAY	1000L
#define DEFAULT_DELAY	1340L
#define DEFAULT_MAXDELAY 60000L
#define MAX_DELAY       300000L
#define DEFAULT_TIMEOUT 360

#endif /* _FSP_COMMON_DEF_H_ */
