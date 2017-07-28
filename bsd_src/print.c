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

#include "tweak.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/param.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#else
#include <utmp.h>
#endif
#ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
# else
#   include <time.h>
# endif
#endif
#include "my-string.h"
#include "ls.h"

#define BLK(A) (((A)+1023)/1024)

static int printtype (mode_t mode)
{
  switch(mode & S_IFMT) {
    case S_IFDIR:
      (void)putchar('/');
      return(1);
  }
  return(0);
}

/*
 * print [inode] [size] name
 * return # of characters printed, no trailing characters
 */
static int printaname (LS * lp)
{
  int chcnt;

  chcnt = 0;

  if (f_inode) {
    printf("%5lu ", (unsigned long)lp->lstat.st_ino);
    chcnt += 6;
  }

  if (f_size) {
    printf("%4ld ", (long)BLK(lp->lstat.st_size));
    chcnt += 5;
  }

  printf("%s", lp->name); chcnt += strlen(lp->name);

  if (f_type) chcnt += printtype(lp->lstat.st_mode);

  return(chcnt);
}

void printscol (LS * stats, int num)
{
  for (; num--; ++stats) {
    printaname(stats);
    putchar('\n');
  }
}

static void printtime (time_t ftime)
{
  int i;
  char *longstring;

  longstring = (char *)ctime(&ftime);
  for (i = 4; i < 11; ++i) (void)putchar(longstring[i]);

#define	SIXMONTHS	((365 / 2) * 24 * 60 * 60)

  if (ftime + SIXMONTHS > time((time_t *)NULL))
    for (i = 11; i < 16; ++i) (void)putchar(longstring[i]);
  else {
    (void)putchar(' ');
    for (i = 20; i < 24; ++i) (void)putchar(longstring[i]);
  }
  (void)putchar(' ');
}

void printlong (LS * stats, int num)
{
  const char *modep;

  if (f_total) (void)printf("total %lu\n",(unsigned long) stats[0].lstat.st_btotal);
  for (; num--; ++stats) {
    if (f_inode) printf("%6lu ", (unsigned long)stats->lstat.st_ino);
    if (f_size ) printf("%4lu ", (unsigned long)BLK(stats->lstat.st_size));
    modep = ((S_IFDIR & stats->lstat.st_mode)) ? "drwxrwxrwx" : "-rw-rw-rw-" ;

    (void)printf("%s %3u %-*s ", modep, (unsigned int)stats->lstat.st_nlink, 8, "nobody");
    if (f_group) printf("%-*s ", 8, "nobody");
    else printf("%8lu ", (unsigned long)stats->lstat.st_size);
    if (f_accesstime) printtime(stats->lstat.st_atime);
    else if (f_statustime) printtime(stats->lstat.st_ctime);
    else printtime(stats->lstat.st_mtime);
    printf("%s", stats->name);
    if (f_type) printtype(stats->lstat.st_mode);
    putchar('\n');
  }
}

#define	TAB	8

extern int termwidth;

void printcol (LS * stats, int num)
{
  register int base, chcnt, cnt, col, colwidth;
  int endcol, numcols, numrows, row;

  colwidth = stats[0].lstat.st_maxlen;
  if (f_inode) colwidth += 6;
  if (f_size) colwidth += 5;
  if (f_type) colwidth += 1;

  colwidth = (colwidth + TAB) & ~(TAB - 1);
  if (termwidth < 2 * colwidth) {
    printscol(stats, num);
    return;
  }

  numcols = termwidth / colwidth;
  numrows = num / numcols;
  if (num % numcols) ++numrows;

  if (f_size && f_total) printf("total %lu\n", (unsigned long)stats[0].lstat.st_btotal);
  for (row = 0; row < numrows; ++row) {
    endcol = colwidth;
    for (base = row, chcnt = col = 0; col < numcols; ++col) {
      chcnt += printaname(stats + base);
      if ((base += numrows) >= num) break;
      while ((cnt = ( (chcnt + TAB) & ~(TAB - 1))) <= endcol) {
	(void)putchar('\t');
	chcnt = cnt;
      }
      endcol += colwidth;
    }
    putchar('\n');
  }
}
