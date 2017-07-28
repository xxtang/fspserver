/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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

    /*********************************************************************\
    *  Copyright (c) 1993 by Michael Meskes                               *
    *  (meskes@ulysses.informatik.rwth-aachen.de)                         *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include "bsd_extern.h"
#include "merge.h"
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "find.h"

extern PLAN *plan;

time_t now;			/* time find was run */
int isoutput;			/* user specified output operator */
int process;			/* process current directory */

static void usage_ffind (void);

static void eval_file (char * name, struct stat * sbufp,
			     int  mode, int  level)
{
  register PLAN *p;

  for (p = plan; p && (p->eval)(p, sbufp, name); p = p->next);
}

static int eval_dir (char * name, struct stat * sbufp, u_long * sum)
{
  register PLAN *p;

  process = 0;
  for (p = plan; p && (p->eval)(p, sbufp, name); p = p->next);
  return (process);
}

int main (int argc, char **  argv)
{
  register char **p;
  char *singlefile[2], **files;

  env_client();
  (void)time(&now);	/* initialize the time-of-day */

  p = ++argv;

  /* First option delimits the file list. */
  while (*p && !option(*p)) p++;
  if (p == argv) usage_ffind();

  find_formplan(p);

  /* Execute plan for all file lists */
  while (*argv) {
    if (argv >= p) break;
    if (!(files = bsdglob(*argv))) {
      files = singlefile;
      singlefile[0] = *argv;
      singlefile[1] = 0;
    }
    for ( ; *files; files++)
      util_process_file(*files, 0, eval_file, eval_dir, 0L, 0);

    argv++;
  }

  client_done();
  return(0);
}

static void usage_ffind (void)
{
  fprintf(stderr,"usage: ffind file [file ...] expression\n");
  exit(EX_USAGE);
}
