/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * C-shell glob for random programs.
 */

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include  "bsd_extern.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_SYS_SYSLIMITS_H
#include <sys/syslimits.h>
#endif

#include <stdlib.h>
#include "my-string.h"

#ifndef NCARGS
#define NCARGS ARG_MAX
#endif

#define	QUOTE 0200
#define	TRIM 0177
#define	eq(a,b)		(strcmp(a, b)==0)
#define	GAVSIZ		(NCARGS/6)
#define	isdir(d)	((d.st_mode & S_IFMT) == S_IFDIR)

static	char **gargv;		/* Pointer to the (stack) arglist */
static	int gargc;		/* Number args in gargv */
static	int gnleft;
static	int gflag;
const   char *globerr;

static	int globcnt;

const char *globchars = "`{[*?";

static	char *gpath, *gpathp, *lastgpathp;
static	int globbed;
static	char *entp;
static	char **sortbas;

static int any (register int c, register const char * s)
{
  while (*s)
    if (*s++ == c) return(1);
  return(0);
}

static int tglob (char c)
{
  if (any(c, globchars))
    gflag |= c == '{' ? 2 : 1;
  return (c);
}

static int addpath (char c)
{
  if (gpathp >= lastgpathp) globerr = "Pathname too long";
  else {
    *gpathp++ = c;
    *gpathp = 0;
  }
  return(0);
}

static int ginit (char ** agargv)
{
  agargv[0] = 0; gargv = agargv; sortbas = agargv; gargc = 0;
  gnleft = NCARGS - 4;
  return(0);
}

static void sort (void)
{
  register char **p1, **p2, *c;
  char **Gvp = &gargv[gargc];

  p1 = sortbas;
  while (p1 < Gvp-1) {
    p2 = p1;
    while (++p2 < Gvp)
      if (strcmp(*p1, *p2) > 0) {
        c = *p1;
	*p1 = *p2;
	*p2 = c;
      }
    p1++;
  }
  sortbas = Gvp;
}

static char *strspl (register char * cp, register const char * dp)
{
  register char *ep = (char *)malloc((unsigned)(strlen(cp) + strlen(dp) + 1));

  if (ep == (char *)0) {
    perror("Out of memory 1");
    exit(EX_OSERR);
  }
  (void) strcpy(ep, cp);
  (void) strcat(ep, dp);
  return (ep);
}

static int Gcat (char * s1, const char * s2)
{
  register int len = strlen(s1) + strlen(s2) + 1;

  if (len >= gnleft || gargc >= GAVSIZ - 1) globerr = "Arguments too long";
  else {
    gargc++;
    gnleft -= len;
    gargv[gargc] = 0;
    gargv[gargc - 1] = strspl(s1, s2);
  }
  return(0);
}

static int execbrc (char *, char *);
void matchdir (char *);

static void expand (char * as)
{
  register char *cs;
  register char *sgpathp, *oldcs;
  struct stat stb;

  sgpathp = gpathp;
  cs = as;
  while (!any(*cs, globchars)) {
    if (*cs == 0) {
      if (!globbed) Gcat(gpath, "");
      else if (util_stat(gpath, &stb) >= 0) {
        Gcat(gpath, "");
        globcnt++;
      }
      break;
    }
    addpath(*cs++);
  }
  if(*cs) {
    oldcs = cs;
    while (cs > as && *cs != '/') {
      cs--; gpathp--;
    }
    if (*cs == '/') {
      cs++; gpathp++;
    }
    *gpathp = 0;
    if (*oldcs == '{') {
      (void) execbrc(cs, ((char *)0));
      return;
    }
    matchdir(cs);
  }
  gpathp = sgpathp;
  *gpathp = 0;
}

static int amatch (char * s, char * p)
{
  register int scc;
  int ok, lc;
  char *sgpathp;
  struct stat stb;
  int c, cc;

  globbed = 1;
  for (;;) {
    scc = *s++ & TRIM;
    switch (c = *p++) {
      case '{':
        return (execbrc(p - 1, s - 1));
      case '[':
        ok = 0;
        lc = 077777;
        while ( (cc = *p++) ) {
          if (cc == ']') {
            if (ok) break;
            return (0);
          }
          if (cc == '-') {
            if (lc <= scc && scc <= *p++) ok++;
          } else if (scc == (lc = cc)) ok++;
        }
        if (cc == 0) {
          if (ok) p--;
          else return 0;
        }
        continue;
      case '*':
        if (!*p) return (1);
        if (*p == '/') {
          p++;
          goto slash;
        }
        s--;
        do {
          if (amatch(s, p)) return (1);
        } while (*s++);
        return (0);
      case 0:
        return (scc == 0);
      default:
        if (c != scc) return (0);
        continue;
      case '?':
        if (scc == 0) return (0);
        continue;
      case '/':
        if (scc) return (0);
slash:
        s = entp;
        sgpathp = gpathp;
        while (*s) addpath(*s++);
        addpath('/');
        if (util_stat(gpath, &stb) == 0 && isdir(stb))
	{
          if (*p == 0) {
            Gcat(gpath, "");
            globcnt++;
          } else expand(p);
	}
        gpathp = sgpathp;
        *gpathp = 0;
        return (0);
    }
  }
}

static int match (char * s, char * p)
{
  register int c;
  register char *sentp;
  char sglobbed = globbed;

  if (*s == '.' && *p != '.') return (0);
  sentp = entp;
  entp = s;
  c = amatch(s, p);
  entp = sentp;
  globbed = sglobbed;
  return (c);
}

void matchdir (char * pattern)
{
  struct stat stb;
  register struct rdirent *dp;
  RDIR *dirp;

  dirp = util_opendir(gpath);
  if (dirp == NULL) {
    if (globbed) return;
    globerr = "Bad directory components";
    return;
  }
  if (util_stat(gpath, &stb) < 0) {
    util_closedir(dirp);
    globerr = "Bad directory components";
  }
  if (!isdir(stb)) {
    errno = ENOTDIR;
    util_closedir(dirp);
    globerr = "Bad directory components";
  }
  while ((dp = util_readdir(dirp)) != NULL) {
    if (dp->d_fileno == 0) continue;
    if (match(dp->d_name, pattern)) {
      Gcat(gpath, dp->d_name);
      globcnt++;
    }
  }
  util_closedir(dirp);
}

static int execbrc (char * p, char * s)
{
  char restbuf[BUFSIZ + 2];
  register char *pe, *pm, *pl;
  int brclev = 0;
  char *lm, savec, *sgpathp;

  for (lm = restbuf; *p != '{'; *lm++ = *p++);
  for (pe = ++p; *pe; pe++)
    switch (*pe) {
      case '{':
        brclev++;
        continue;
      case '}':
        if (brclev == 0) goto pend;
        brclev--;
        continue;
      case '[':
        for (pe++; *pe && *pe != ']'; pe++);
    }
pend:
  brclev = 0;
  for (pl = pm = p; pm <= pe; pm++)
    switch (*pm & (QUOTE|TRIM)) {
      case '{':
        brclev++;
        continue;
      case '}':
        if (brclev) {
          brclev--;
          continue;
  	}
  	goto doit;
      case ','|QUOTE:
      case ',':
        if (brclev) continue;
doit:
        savec = *pm;
        *pm = 0;
	(void) strcpy(lm, pl);
        (void) strcat(restbuf, pe + 1);
        *pm = savec;
        if (s == 0) {
          sgpathp = gpathp;
          expand(restbuf);
          gpathp = sgpathp;
          *gpathp = 0;
        } else if (amatch(s, restbuf)) return (1);
        sort();
        pl = pm + 1;
        if (brclev) return (0);
        continue;
      case '[':
        for (pm++; *pm && *pm != ']'; pm++);
        if (!*pm) pm--;
	continue;
    }
    if (brclev) goto doit;
    return (0);
}

static void acollect (register char * as)
{
  register int ogargc = gargc;

  gpathp = gpath; *gpathp = 0; globbed = 0;
  expand(as);
  if (gargc != ogargc) sort();
}

static void collect (register char * as)
{
  if (eq(as, "{") || eq(as, "{}")) {
    Gcat(as, "");
    sort();
  } else acollect(as);
}

static void blkfree (char ** av0)
{
  register char **av = av0;

  while (*av) free(*av++);
}

static int blklen (register char ** av)
{
  register int i = 0;

  while (*av++) i++;
  return (i);
}

static char **blkcpy (char ** oav, register char ** bv)
{
  register char **av = oav;

  while ( (*av++ = *bv++) ) continue;
  return (oav);
}

static char **copyblk (register char ** v)
{
  register char **nv;

  nv = (char **)malloc((unsigned)((blklen(v) + 1) * sizeof(char **)));
  if (nv == (char **)0) {
    perror("Out of memory 2");
    exit(EX_OSERR);
  }
  return (blkcpy(nv, v));
}

typedef int (*charfunc) (char) ;
static void rscan (register char **t, charfunc f)
{
  register char *p, c;

  while ((p = *t++)) {
    if (f == (charfunc) tglob)
    {
      if (*p == '~') gflag |= 2;
      else if (eq(p, "{") || eq(p, "{}")) continue;
    }
    while ( (c = *p++) ) (*f)(c);
  }
}

char **bsdglob (register char * v)
{
  char agpath[BUFSIZ];
  char *agargv[GAVSIZ];
  char *vv[2];
  vv[0] = v;
  vv[1] = 0;
  gflag = 0;
  rscan(vv, (charfunc) tglob);
  if (gflag == 0) return (copyblk(vv));

  globerr = 0;
  gpath = agpath; gpathp = gpath; *gpathp = 0;
  lastgpathp = &gpath[sizeof agpath - 2];
  ginit(agargv); globcnt = 0;
  collect(v);
  if (globcnt == 0 && (gflag&1)) {
    blkfree(gargv), gargv = 0;
    return (0);
  } else return (gargv = copyblk(gargv));
}
