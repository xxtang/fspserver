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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"
#include "find.h"

OPTION options[] = {
 {"!",		N_NOT,		c_not,		O_ZERO},
 {"(",		N_OPENPAREN,	c_openparen,	O_ZERO},
 {")",		N_CLOSEPAREN,	c_closeparen,	O_ZERO},
 {"-a",		N_AND,		NULL,		O_NONE},
 {"-and",	N_AND,		NULL,		O_NONE},
 {"-exec",	N_EXEC,		c_exec,		O_ARGVP},
 {"-ls",	N_LS,		c_ls,		O_ZERO},
 {"-name",	N_NAME,		c_name,		O_ARGV},
 {"-newer",	N_NEWER,	c_newer,	O_ARGV},
 {"-o",		N_OR,		c_or,		O_ZERO},
 {"-ok",	N_OK,		c_exec,		O_ARGVP},
 {"-or",	N_OR,		c_or,		O_ZERO},
 {"-print",	N_PRINT,	c_print,	O_ZERO},
 {"-prune",	N_PRUNE,	c_prune,	O_ZERO},
 {"-size",	N_SIZE,		c_size,		O_ARGV},
 {"-time",	N_TIME,		c_time,		O_ARGV},
 {"-type",	N_TYPE,		c_type,		O_ARGV}
};

/*
 * find_create --
 *	create a node corresponding to a command line argument.
 *
 * TODO:
 *	add create/process function pointers to node, so we can skip
 *	this switch stuff.
 */
PLAN *find_create (char *** argvp)
{
  register OPTION *p;
  PLAN *new;
  char **argv;

  argv = *argvp;

  if ((p = option(*argv)) == NULL) {
    (void)fprintf(stderr, "find: unknown option %s.\n", *argv);
    exit(EX_USAGE);
  }
  ++argv;
  if (p->flags & (O_ARGV|O_ARGVP) && !*argv) {
    (void)fprintf(stderr, "find: %s requires additional arguments.\n",
		  *--argv);
    exit(EX_USAGE);
  }

  switch(p->flags) {
    case O_NONE:
      new = NULL;
      break;
    case O_ZERO:
      new = (p->create)();
      break;
    case O_ARGV:
      new = (p->create)(*argv++);
      break;
    case O_ARGVP:
      new = (p->create)(&argv, p->token == N_OK);
      break;
  }
  *argvp = argv;
  return(new);
}

static int typecompare (const void * a, const void * b)
{
  return(strcmp(((const OPTION *)a)->name, ((const OPTION *)b)->name));
}

OPTION *option (char * name)
{
  OPTION tmp;

  tmp.name = name;
  return((OPTION *)bsearch(&tmp, options, sizeof(options)/sizeof(OPTION),
			   sizeof(OPTION), typecompare));
}
