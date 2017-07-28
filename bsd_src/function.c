/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Cimarron D. Taylor of the University of California, Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "tweak.h"
#ifdef HAVE_UNISTD_H
#ifndef __hpux
#include <unistd.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include "my-string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#ifdef _POSIX_SOURCE
#include "limits.h"
#if !defined(MAXPATHLEN) && defined(PATH_MAX)
#define MAXPATHLEN PATH_MAX
#endif
#endif
#ifdef HAVE_TZFILE_H
#include <tzfile.h>
#endif
#include "common_def.h"
#include "client_def.h"
#include "c_extern.h"
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#include "find.h"


#define	FIND_EQUAL	0
#define	FIND_LESSTHAN	1
#define	FIND_GREATER	2

#define	COMPARE(a, b) { \
  switch(plan->flags) { \
    case FIND_EQUAL: \
      return(a == b); \
    case FIND_LESSTHAN: \
      return(a < b); \
    case FIND_GREATER: \
      return(a > b); \
  } \
  return(0); \
}

static PLAN *palloc (enum ntype t, int (*f)(void))
{
  PLAN *new;

  new = (PLAN *) malloc(sizeof(PLAN));

  if ( new ) {
    new->type = t;
    new->eval = f;
    new->flags = 0;
    new->next = NULL;
    return(new);
  }
  perror("palloc");
  exit(EX_OSERR);
}

extern int isoutput;
extern int process;

/*
 * find_parsenum --
 *	Parse a string of the form [+-]# and return the value.
 */
static long find_parsenum (PLAN * plan, const char * option, char * str,
				 char * endch)
{
  long value;
  char *endchar;		/* pointer to character ending conversion */

  /* determine comparison from leading + or - */
  switch(*str) {
    case '+':
      ++str;
      plan->flags = FIND_GREATER;
      break;
    case '-':
      ++str;
      plan->flags = FIND_LESSTHAN;
      break;
    default:
      plan->flags = FIND_EQUAL;
      break;
  }

  /*
   * convert the string with strtol().  Note, if strtol() returns zero
   * and endchar points to the beginning of the string we know we have
   * a syntax error.
   */
  value = strtol(str, &endchar, 10);
  if ( (!value && endchar == str) || (endchar[0] &&
      (!endch || endchar[0] != *endch))) {
    fprintf(stderr,"%s: %s", option, "illegal numeric value");
    exit(EX_USAGE);
  }
  if (endch)
    *endch = endchar[0];
  return(value);
}

/*
 * -time n functions --
 *
 *	True if the difference between the file time and the
 *	current time is n 24 hour periods.
 *
 */

extern time_t now;
static int find_time (PLAN * plan, struct stat * sbuf,  char * path)
{

  /* with FSP sbuf.st_atime=sbuf.st_ctime=sbuf.st_mtime */
  COMPARE((now - sbuf->st_atime + SECSPERDAY - 1) / SECSPERDAY, plan->t_data);
}


PLAN * c_time (char * arg)
{
  PLAN *new;

  new = palloc(N_TIME, find_time);
  new->t_data = find_parsenum(new, "-time", arg, NULL);
  return(new);
}

/*
 * brace_subst --
 *      Replace occurrences of {} in s1 with s2 and return the result string.
 */
static void brace_subst (char * orig, char ** store, char * path, int len)
{
  register int plen;
  register char ch, *p;

  plen = strlen(path);
  for (p = *store; (ch = *orig) ; ++orig)
    if (ch == '{' && orig[1] == '}') {
      while ((p - *store) + plen > len)
        if (!(*store = (char *)realloc(*store, len *= 2))) {
          perror("realloc");
          client_done();
          exit(EX_OSERR);
        }
      memmove(p,path,plen);
      p += plen;
      ++orig;
    } else
      *p++ = ch;
  *p = '\0';
}

/*
 * queryuser --
 *      print a message to standard error and then read input from standard
 *      input. If the input is 'y' then 1 is returned.
 */
static int queryuser (char ** argv)
{
  int ch, first, nl;

  fprintf(stderr, "\"%s", *argv);
  while (*++argv) fprintf(stderr, " %s", *argv);
  fprintf(stderr, "\"? ");
  fflush(stderr);

  first = ch = getchar();
  for (nl = 0;;)
  {
    if (ch == '\n')
    {
      nl = 1;
      break;
    }
    if (ch == EOF) break;
    ch = getchar();
  }

  if (!nl) {
    fprintf(stderr, "\n");
    fflush(stderr);
  }
  return(first == 'y');
}

/*
 * [-exec | -ok] utility [arg ... ] ; functions --
 *
 *	True if the executed utility returns a zero value as exit status.
 *	The end of the primary expression is delimited by a semicolon.  If
 *	"{}" occurs anywhere, it gets replaced by the current pathname.
 *	The current directory for the execution of utility is the same as
 *	the current directory when the find utility was started.
 *
 *	The primary -ok is different in that it requests affirmation of the
 *	user before executing the utility.
 */
static int find_exec (PLAN * plan, struct stat * sbuf, char * path)
{
  register int cnt;
  pid_t pid;
#ifndef HAVE_UNION_WAIT
  int status;
#else
  union wait status;
#endif

  for (cnt = 0; plan->e_argv[cnt]; ++cnt)
    if (plan->e_len[cnt])
      brace_subst(plan->e_orig[cnt], &plan->e_argv[cnt], path,
		  plan->e_len[cnt]);

  if (plan->flags && !queryuser(plan->e_argv)) return(0);

  switch(pid = fork())
   {
    case -1:
      perror ("fork");
      exit(EX_OSERR);
    case 0:
      execvp(plan->e_argv[0], plan->e_argv);
      perror ("execvp");
      exit(EX_OSERR);
  }
  pid = wait(&status);

 return(pid != -1 && WIFEXITED(status) && !WEXITSTATUS(status));
}

static char *emalloc_ffind (unsigned int len)
{
  char *p;

  if ( (p = (char *)malloc(len))) return((char *)p);
  perror("malloc");
  exit(EX_OSERR);
}

/*
 * c_exec --
 *	build three parallel arrays, one with pointers to the strings passed
 *	on the command line, one with (possibly duplicated) pointers to the
 *	argv array, and one with integer values that are lengths of the
 *	strings, but also flags meaning that the string has to be massaged.
 */
PLAN *c_exec (char *** argvp, int isok)
{
  PLAN *new;			/* node returned */
  register int cnt;
  register char **argv, **ap, *p;

  isoutput = 1;

  new = palloc(N_EXEC, find_exec);
  new->flags = isok;

  for (ap = argv = *argvp;; ++ap) {
    if (!*ap) {
      fprintf(stderr,"%s: no terminating", isok ? "-ok" : "-exec");
      exit(EX_USAGE);
    }
    if (**ap == ';') break;
  }

  cnt = ap - *argvp + 1;
  new->e_argv = (char **)emalloc_ffind((unsigned int)cnt * sizeof(char *));
  new->e_orig = (char **)emalloc_ffind((unsigned int)cnt * sizeof(char *));
  new->e_len = (int *)emalloc_ffind((unsigned int)cnt * sizeof(int));

  for (argv = *argvp, cnt = 0; argv < ap; ++argv, ++cnt) {
    new->e_orig[cnt] = *argv;
    for (p = *argv; *p; ++p)
      if (p[0] == '{' && p[1] == '}') {
       	new->e_argv[cnt] = (char *)emalloc_ffind((unsigned int)MAXPATHLEN);
      	new->e_len[cnt] = MAXPATHLEN;
      	break;
      }
    if (!*p) {
      new->e_argv[cnt] = *argv;
      new->e_len[cnt] = 0;
    }
  }
  new->e_argv[cnt] = new->e_orig[cnt] = NULL;

  *argvp = argv + 1;
  return(new);
}

static void printtime_ffind (time_t ftime)
{
  int i;
  char *longstring;

  longstring = (char *)ctime(&ftime);
  for (i = 4; i < 11; ++i) putchar(longstring[i]);

#define SIXMONTHS       ((DAYSPERNYEAR / 2) * SECSPERDAY)

  if (ftime + SIXMONTHS > time((time_t *)NULL))
    for (i = 11; i < 16; ++i) putchar(longstring[i]);
  else {
    (void)putchar(' ');
    for (i = 20; i < 24; ++i) putchar(longstring[i]);
  }
  putchar(' ');
}

#define BLK(A) (((A)+1023)/1024)

static void printlong_ffind (char * name, struct stat * sb)
{
  const char *modep;

  printf("%4ld ", (long)BLK(sb->st_size));
  modep = ((S_IFDIR & sb->st_mode)) ? "drwxrwxrwx" : "-rw-rw-rw-" ;
  printf("%s %3u %-*s %-*s ", modep, (unsigned int)sb->st_nlink, 8, "nobody", 8, "nobody");

  printf("%8ld ", (long)sb->st_size);
  printtime_ffind(sb->st_mtime);
  printf("%s", name);
  putchar('\n');
}

/*
 * -ls functions --
 *
 *	Always true - prints the current sbuf to stdout in "ls" format.
 */
static int find_ls (PLAN * plan, struct stat * sbuf, char * path)
{
  printlong_ffind(path, sbuf);
  return(1);
}

PLAN *c_ls (void)
{
  isoutput = 1;
  return(palloc(N_LS, find_ls));
}

/*
 * -name functions --
 *
 *	True if the basename of the filename being examined
 *	matches pattern using Pattern Matching Notation S3.14
 */
static int find_name (PLAN * plan, struct stat * sbuf, char * path)
{
  register char *name;

  /* extract filename */
  for (name = path + strlen(path); name > path && *name != '/'; name--);
  if (*name == '/') name++;
  return(fnmatch(plan->c_data, name));
}

PLAN *c_name (char * pattern)
{
  PLAN *new;

  new = palloc(N_NAME, find_name);
  new->c_data = pattern;
  return(new);
}

/*
 * -newer file functions --
 *
 *	True if the current file has been modified more recently
 *	then the modification time of the file named by the pathname
 *	file.
 */
static int find_newer (PLAN * plan, struct stat * sbuf, char * path)
{
  return(sbuf->st_mtime > plan->t_data);
}

PLAN *c_newer (char * filename)
{
  PLAN *new;
  struct stat sb;

  if (stat(filename, &sb)) {
    perror("stat");
    exit(EX_NOINPUT);
  }
  new = palloc(N_NEWER, find_newer);
  new->t_data = sb.st_mtime;
  return(new);
}

/*
 * -print functions --
 *
 *	Always true, causes the current pathame to be written to
 *	standard output.
 */
static int find_print (PLAN * plan, struct stat * sbuf, char * path)
{
  (void)printf("%s\n", path);
  return(1);
}

PLAN *c_print (void)
{
  isoutput = 1;

  return(palloc(N_PRINT, find_print));
}

/*
 * -prune functions --
 *
 *	Prune a portion of the hierarchy.
 */
static int find_prune (PLAN * plan, struct stat * sbuf, char * path)
{
  process = -1;
  return(1);
}

PLAN *c_prune (void)
{
  return(palloc(N_PRUNE, find_prune));
}

/*
 * -size n[c] functions --
 *
 *	True if the file size in bytes, divided by an implementation defined
 *	value and rounded up to the next integer, is n.  If n is followed by
 *	a c, the size is in bytes.
 */
#define	FIND_SIZE	512
static int divsize = 1;

static int find_size (PLAN * plan, struct stat * sbuf, char * path)
{
  off_t size;

  size = divsize ? ((sbuf->st_size + FIND_SIZE - 1)/FIND_SIZE) : sbuf->st_size;
  COMPARE(size, plan->o_data);
}

PLAN *c_size (char * arg)
{
  PLAN *new;
  char endch='c';

  new = palloc(N_SIZE, find_size);
  new->o_data = find_parsenum(new, "-size", arg, &endch);
  if (endch == 'c') divsize = 0;
  return(new);
}

/*
 * -type c functions --
 *
 *	True if the type of the file is c, where c is d or f for
 *	directory or regular file, respectively.
 */
static int find_type (PLAN * plan, struct stat * sbuf, char * path)
{
  return((sbuf->st_mode & S_IFMT) == plan->m_data);
}

PLAN *c_type (char * typestring)
{
  PLAN *new;
  mode_t  mask;

  switch (typestring[0]) {
    case 'd':
      mask = S_IFDIR;
      break;
    case 'f':
      mask = S_IFREG;
      break;
    default:
      fprintf(stderr,"-type: unknown type");
      exit(EX_USAGE);
  }

  new = palloc(N_TYPE, find_type);
  new->m_data = mask;
  return(new);
}

/*
 * ( expression ) functions --
 *
 *	True if expression is true.
 */
int find_expr (PLAN * plan, struct stat * sbuf, char * path)
{
  register PLAN *p;
  register int state=0;

  for(p=plan->p_data[0]; p && (state=(p->eval)(p, sbuf, path)); p=p->next);
  return(state);
}

/*
 * N_OPENPAREN and N_CLOSEPAREN nodes are temporary place markers.  They are
 * eliminated during phase 2 of find_formplan() --- the '(' node is converted
 * to a N_EXPR node containing the expression and the ')' node is discarded.
 */
PLAN *c_openparen (void)
{
  return(palloc(N_OPENPAREN, (int (*)())-1));
}

PLAN *c_closeparen (void)
{
  return(palloc(N_CLOSEPAREN, (int (*)())-1));
}


/*
 * ! expression functions --
 *
 *	Negation of a primary; the unary NOT operator.
 */
static int find_not (PLAN * plan, struct stat * sbuf, char * path)
{
  register PLAN *p;
  register int state=0;

  for(p=plan->p_data[0]; p && (state=(p->eval)(p, sbuf, path)); p=p->next);
  return(!state);
}

PLAN *c_not (void)
{
  return(palloc(N_NOT, find_not));
}

/*
 * expression -o expression functions --
 *
 *	Alternation of primaries; the OR operator.  The second expression is
 * not evaluated if the first expression is true.
 */
static int find_or (PLAN * plan, struct stat * sbuf, char * path)
{
  register PLAN *p;
  register int state=0;

  for(p=plan->p_data[0]; p && (state=(p->eval)(p, sbuf, path)); p=p->next);

  if (state) return(1);

  for(p=plan->p_data[1]; p && (state=(p->eval)(p, sbuf, path)); p=p->next);
  return(state);
}

PLAN *c_or (void)
{
  return(palloc(N_OR, find_or));
}
