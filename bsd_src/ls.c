/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include "tweak.h"
#include "bsd_extern.h"
#include "client_def.h"
#include "c_extern.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"
#include "ls.h"

static char path[2*1024 + 1];
static char *endofpath = path;

typedef int (*COMPAR)(const void *, const void *);
int (*sortfcn)(LS *, LS *);
void (*printfcn)(LS *, int);

int termwidth = 80;		/* default terminal width */

/* flags */
int f_accesstime;		/* use time of last access */
int f_column;			/* columnated format */
int f_group;			/* show group ownership of a file */
int f_ignorelink;		/* indirect through symbolic link operands */
int f_inode;			/* print inode */
int f_kblocks;			/* print size in kilobytes */
int f_listalldot;		/* list . and .. as well */
int f_listdir;			/* list actual directory, not contents */
int f_listdot;			/* list files beginning with . */
int f_longform;			/* long listing format */
int f_needstat;			/* if need to stat files */
int f_newline;			/* if precede with newline */
int f_nonprint;			/* show unprintables as ? */
int f_nosort;			/* don't sort output */
int f_recursive;		/* ls subdirectories also */
int f_reversesort;		/* reverse whatever sort is used */
int f_singlecol;		/* use single column output */
int f_size;			/* list size in short listing */
int f_statustime;		/* use time of last mode change */
int f_dirname;			/* if precede with directory name */
int f_timesort;			/* sort by time vice name */
int f_total;			/* if precede with "total" line */
int f_type;			/* add type character for non-regular files */

static int tabdir (LS * lp, LS ** s_stats)
{
  register RDIR *dirp;
  register int cnt, maxentry, maxlen;
  register char *p;
  struct rdirent *dp;
  unsigned long blocks;
  LS *stats;

  if (!(dirp = util_opendir("."))) {
    perror(lp->name);
    return(0);
  }
  blocks = maxentry = maxlen = 0;
  stats = NULL;
  for (cnt = 0; (dp = util_readdir(dirp));) {
    /* this does -A and -a */
    p = dp->d_name;
    if (p[0] == '.') {
      if (!f_listdot) continue;
      if (!f_listalldot && (!p[1] || (p[1] == '.' && !p[2]))) continue;
    }
    if (cnt == maxentry) {
#define	DEFNUM	256
      maxentry += DEFNUM;
      if(stats) {
	*s_stats = stats = (LS *)realloc((char *)stats,
					 (unsigned int)maxentry * sizeof(LS));
	if (!*s_stats) nomem();
      } else {
	*s_stats = stats = (LS *)malloc((unsigned int)maxentry * sizeof(LS));
	if (!*s_stats) nomem();
      }
    }
    if (f_needstat && util_stat(dp->d_name, &stats[cnt].lstat)) {
      /*
       * don't exit -- this could be an NFS mount that has
       * gone away.  Flush stdout so the messages line up.
       */
      (void)fflush(stdout);
      perror(dp->d_name);
      continue;
    }
    stats[cnt].name = dp->d_name;

    /*
     * get the inode from the directory, so the -f flag
     * works right.
     */
    stats[cnt].lstat.st_ino = dp->d_fileno;

    /* save name length for -C format */
    stats[cnt].len = dp->d_namlen;

    /* calculate number of blocks if -l/-s formats */
    if (f_longform || f_size) blocks += (stats[cnt].lstat.st_size+1023)/1024;

    /* save max length if -C format */
    if (f_column && maxlen < (int)dp->d_namlen) maxlen = dp->d_namlen;
    ++cnt;
  }
  (void)util_closedir(dirp);

  if (cnt) {
    stats[0].lstat.st_btotal = blocks;
    stats[0].lstat.st_maxlen = maxlen;
  } else if (stats) {
    (void)free((char *)stats);
  }
  return(cnt);
}

static void displaydir (LS *, register int);

static void subdir (LS * lp)
{
  LS *stats;
  int num;

  if (f_newline) (void)putchar('\n');
  if (f_dirname) (void)printf("%s:\n", path);

  if (util_cd2(lp->name)) {
    perror(lp->name);
    return;
  }
  if ( (num = tabdir(lp, &stats))) {
    displaydir(stats, num);
    (void)free((char *)stats);
  }
  if (util_cd2("..")) {
    perror("..");
    ls_bad(1);
  }
}

static void displaydir (LS * stats, register int num)
{
  register char *p, *savedpath;
  LS *lp;

  if (num > 1 && !f_nosort) {
    unsigned long save1, save2;

    save1 = stats[0].lstat.st_btotal;
    save2 = stats[0].lstat.st_maxlen;
    qsort((char *)stats, num, sizeof(LS), (COMPAR) sortfcn);
    stats[0].lstat.st_btotal = save1;
    stats[0].lstat.st_maxlen = save2;
  }

  printfcn(stats, num);

  if (f_recursive)
  {
    savedpath = endofpath;
    for (lp = stats; num--; ++lp)
    {
      if (!(S_ISDIR(lp->lstat.st_mode)))
	  continue;
      p = lp->name;
      if (p[0] == '.' && (!p[1] || (p[1] == '.' && !p[2]))) continue;
      if (endofpath != path && endofpath[-1] != '/') *endofpath++ = '/';
      for (; (*endofpath = *p++); ++endofpath);
      f_newline = f_dirname = f_total = 1;
      subdir(lp);
      *(endofpath = savedpath) = '\0';
    }
  }
}

static void doargs (int argc, char ** argv)
{
  register LS *dstatp=NULL, *rstatp=NULL;
  register int cnt, dircnt, dirmax, maxlen=0, regcnt, regmax;
  LS *dstats, *rstats;
  struct stat sb;
  char top[2*1024 + 1], **av, *av2[2];
  unsigned long blocks;
  //RDIR * dl;

  /*
   * walk through the operands, building separate arrays of LS
   * structures for directory and non-directory files.
   */
  dstats = rstats = NULL;
  dirmax = regmax = 0;
  /* disable use of new CC_STAT command for performance reasons */
  statworks = 0;

  for (dircnt = regcnt = 0; *argv; ++argv)  {
    if(!(av = bsdglob(*argv))) {
      av = av2;
      av2[0] = *argv;
      av2[1] = 0;
    }

    for( ; *av; av++) {
      if (util_stat(*av, &sb)) {
	perror(*av);
	if (errno == ENOENT) continue;
	ls_bad(1);
      }

      if ((S_ISDIR(sb.st_mode)) && !f_listdir) {
	if(dirmax == dircnt) {
          dirmax += 10;
          if (!dstats) {
	    dstatp = dstats = (LS *)emalloc(dirmax * (sizeof(LS)));
          } else {
	    dstats = (LS *)realloc(dstats, dirmax * (sizeof(LS)));
	    dstatp = dstats + dircnt;
          }
	}
	dstatp->name = *av;
	dstatp->lstat = sb;
	++dstatp;
	++dircnt;
      } else {
	if(regmax == regcnt) {
          regmax += 10;
	  if (!rstats) {
	    blocks = 0;
	    maxlen = -1;
	    rstatp = rstats = (LS *)emalloc(regmax * (sizeof(LS)));
	  } else {
	    rstats = (LS *)realloc(rstats, regmax * (sizeof(LS)));
	    rstatp = rstats + regcnt;
	  }
	}
	rstatp->name = *av;
	rstatp->lstat = sb;

	/* save name length for -C format */
	rstatp->len = strlen(*av);

	if (f_nonprint) prcopy(*av, *av, rstatp->len);

	/* calculate number of blocks if -l/-s formats */
	if (f_longform || f_size) blocks += (sb.st_size + 1023)/1024;

	/* save max length if -C format */
	if (f_column && maxlen < rstatp->len) maxlen = rstatp->len;
	
	++rstatp;
	++regcnt;
      }
    }
  }
  /* display regular files */
  if (regcnt) {
    rstats[0].lstat.st_btotal = blocks;
    rstats[0].lstat.st_maxlen = maxlen;
    displaydir(rstats, regcnt);
    f_newline = f_dirname = 1;
  }
  /* display directories */
  if (dircnt) {
    register char *p;

    f_total = 1;
    if (dircnt > 1) {
      (void)util_getwd(top);
      qsort((char *)dstats, dircnt, sizeof(LS), (COMPAR) sortfcn);
      f_dirname = 1;
    }
    for (cnt = 0; cnt < dircnt; ++dstats) {
      for (endofpath = path, p = dstats->name;
	   (*endofpath = *p++); ++endofpath);
      subdir(dstats);
      f_newline = 1;
      if (++cnt < dircnt && util_cd2(top)) {
	perror(top);
	ls_bad(1);
      }
    }
  }
}

void fls_main (int argc, char ** argv)
{
  int ch;
  char *p;

  /* terminal defaults to -Cq, non-terminal defaults to -1 */
  if (isatty(1)) {
    f_nonprint = 1;
    termwidth  = 80;
#ifdef TIOCGWINSZ
    {
      struct winsize win;
      if (ioctl(1, TIOCGWINSZ, &win) == -1 || !win.ws_col) {
        if ( (p = (char *)getenv("COLUMNS"))) termwidth = atoi(p);
      } else termwidth = win.ws_col;
    }
#endif
    f_column = 1;
  } else f_singlecol = 1;

  /* root is -A automatically */
  if (!getuid()) f_listdot = 1;

  while ((ch = getopt(argc, argv, "1ACFLRacdfgiklqrstu")) != EOF) {
    switch (ch) {
      /*
       * -1, -C and -l all override each other
       * so shell aliasing works right
       */
      case '1':
        f_singlecol = 1;
        f_column = f_longform = 0;
        break;
      case 'C':
        f_column = 1;
        f_longform = f_singlecol = 0;
        break;
      case 'l':
        f_longform = 1;
        f_column = f_singlecol = 0;
        break;
    	/* -c and -u override each other */
      case 'c':
        f_statustime = 1;
        f_accesstime = 0;
        break;
      case 'u':
        f_accesstime = 1;
        f_statustime = 0;
        break;
      case 'F':
        f_type = 1;
        break;
      case 'L':
        f_ignorelink = 1;
        break;
      case 'R':
        f_recursive = 1;
        break;
      case 'a':
        f_listalldot = 1;
        /* FALLTHROUGH */
      case 'A':
        f_listdot = 1;
        break;
      case 'd':
        f_listdir = 1;
        break;
      case 'f':
        f_nosort = 1;
        break;
      case 'g':
        f_group = 1;
        break;
      case 'i':
        f_inode = 1;
        break;
      case 'k':
        f_kblocks = 1;
        break;
      case 'q':
        f_nonprint = 1;
        break;
      case 'r':
        f_reversesort = 1;
        break;
      case 's':
        f_size = 1;
        break;
      case 't':
        f_timesort = 1;
        break;
      case '?':
      default:
        usage();
    }
  }
  argc -= optind;
  argv += optind;

  /* -d turns off -R */
  if (f_listdir)
    f_recursive = 0;

  /* if need to stat files */
  f_needstat = f_longform || f_recursive || f_timesort || f_size || f_type;

  /* select a sort function */
  if (f_reversesort) {
    if (!f_timesort) sortfcn = revnamecmp;
    else if (f_accesstime) sortfcn = revacccmp;
    else if (f_statustime) sortfcn = revstatcmp;
    else sortfcn = revmodcmp; /* use modification time */
  } else {
    if (!f_timesort) sortfcn = namecmp;
    else if (f_accesstime) sortfcn = acccmp;
    else if (f_statustime) sortfcn = statcmp;
    else sortfcn = modcmp; /* use modification time */
  }

  /* select a print function */
  if (f_singlecol) printfcn = printscol;
  else if (f_longform) printfcn = printlong;
  else printfcn = printcol;

  if (!argc) {
    argc = 1;
    argv[0] = ".";
    argv[1] = NULL;
  }
  doargs(argc, argv);
}

