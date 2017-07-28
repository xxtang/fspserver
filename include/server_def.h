    /*********************************************************************\
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#ifndef _FSP_SERVER_DEF_H_
#define _FSP_SERVER_DEF_H_ 1

#include "common_def.h"
#include "server_conf.h"


/****************************************************************************
*  HTAB is structure for storing client information for one client machine.
*  They makes it easy to reuse regular unix tool's source for new purposes.
****************************************************************************/

typedef struct HTAB HTAB;

struct HTAB {	unsigned long	inet_num;	/* inet number of client     */
		char	       *hostname;	/* hostname of client        */
		time_t          last_acc;	/* last sucessful access time*/
		unsigned short	next_key;	/* next key client should use*/
		unsigned short	last_key;	/* previous key client used  */
		unsigned short   acc_cnt;	/* number of successful acc  */
		unsigned short   active; };	/* session continuing.	     */

/****************************************************************************
*  IPrange is the structure for storing information about disabled, ignored
*  or normal hosts.
****************************************************************************/
typedef struct {
    unsigned char lo[4];
    unsigned char hi[4];
    char *text;
} IPrange;

/*****************************************************************************
* The PPATH structure is filled in by the function check_path when given a
* path string.  See server_file.c for more info.
*****************************************************************************/

typedef struct {
    const char    *fullp; /* ptr to string containing full pathname  */
    const char    *f_ptr; /* ptr to begining of last path component  */
    unsigned int   f_len; /* length of last component in path        */
    const char    *d_ptr; /* ptr to beginning of directory component */
    unsigned int   d_len; /* length of directory part of path.       */
    const char   *passwd; /* ptr to password                  */
    char     inetstr[16]; /* inet-address of remote-FSP        */
    char      portstr[8];  /* port of remote-FSP                */
} PPATH;

/* open file handles cache */
typedef struct { FILE *fp;
                 unsigned long inet_num;
                 unsigned long port_num;
               } FPCACHE;

/* DIRLISTING holds open directory listings */
typedef struct {
    int8_t *listing;     /* pointer to directory listing */
    size_t listing_size; /* how many bytes has listing? */
    time_t mtime;        /* when cache was build */
} DIRLISTING;

/* hold directory information */
typedef struct {
    char *realname;           /* real directory name, full path resolved */
    IPrange *owner;           /* owners of this directory */
    unsigned char protection; /* directory protection flags */
    char *public_password;    /* password for file access */
    char *owner_password;     /* password for owners */
    char *readme;             /* readme content */
    time_t mtime;             /* directory last modified time */
    time_t lastcheck;         /* when the directory was last stat()-ed */
} DIRINFO;

#define REVERSE_ERR_MSG "Permission denied -- can't identify host.\n\
  Sorry, we can't reverse name you.  If you know that your site normally\n\
  can be, try again in a few minutes when the local maps may have been\n\
  updated.  Otherwise, this service is unavailable to you; check\n\
  with your local admins for as to why this is the case.\n"

/* definition of logging information */
#define L_NONE		0x0000
#define L_ERR		0x0001
#define L_VER		0x0002
#define L_GETDIR	0x0004
#define L_GETFILE	0x0008
#define L_UPLOAD	0x0010
#define L_INSTALL	0x0020
#define L_DELFILE	0x0040
#define L_DELDIR	0x0080
#define L_SETPRO	0x0100
#define L_MAKEDIR	0x0200
#define L_GRABFILE	0x0400
#define L_GETPRO	0x0800
#define L_RDONLY	0x1000
#define L_STAT		0x2000
#define L_RENAME	0x4000
#define L_CH_PASSWD	0x5000
#define L_ALL		0xffff

/* cache directory listing */
#define FSP_DIRLISTING ".FSP_CONTENT"
/* file names used for access control */
#define FSP_NOGET ".FSP_NO_GET"
#define FSP_DEL ".FSP_OK_DEL"
#define FSP_ADD ".FSP_OK_ADD"
#define FSP_MKDIR ".FSP_OK_MKDIR"
#define FSP_RENAME ".FSP_OK_RENAME"
#define FSP_NOLIST ".FSP_NO_LIST"
#define FSP_OWNER ".FSP_OWNER"
#define FSP_PASSWORD ".FSP_OK_PASSWORD"
#define FSP_OWNERPASSWORD ".FSP_OWNER_PASSWORD"

#define FOURGIGS 0xffffffffUL
#define TWOGIGS  0x7fffffffUL

#endif /* _FSP_SERVER_DEF_H_ */
