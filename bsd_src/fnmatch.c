/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
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

/*
 * Function fnmatch() as proposed in Posix 1003.2 B.6 (rev. 9).
 * Compares a filename or pathname to a pattern.
 */

#include "tweak.h"
#include "common_def.h"
#ifdef HAVE_UNISTD_H
#ifndef __hpux
#include <unistd.h>
#endif
#endif
#include "my-string.h"
#include "find.h"

#define	EOS	'\0'

static char *rangematch (register char * pattern, register char test)
{
  register char c, c2;
  int negate, ok;

  if ( (negate = (*pattern == '!')) ) ++pattern;

  /*
   * TO DO: quoting
   */

  for (ok = 0; (c = *pattern++) != ']';) {
    if (c == EOS) return(NULL);		/* illegal pattern */
    if (*pattern == '-' && (c2 = pattern[1]) != EOS && c2 != ']') {
      if (c <= test && test <= c2) ok = 1;
      pattern += 2;
    } else if (c == test) ok = 1;
  }
  return(ok == negate ? NULL : pattern);
}

int fnmatch (register char * pattern, register char * string)
{
  register char c;
  char test;

  for (;;)
    switch (c = *pattern++) {
      case EOS:
        return(*string == EOS);
      case '?':
        if ((test = *string++) == EOS) return(0);
        break;
      case '*':
        c = *pattern;
        /* collapse multiple stars */
        while (c == '*') c = *++pattern;
        /* optimize for pattern with * at end or before / */
        if (c == EOS) return(1);

        /* general case, use recursion */
        while ((test = *string) != EOS) {
          if (fnmatch(pattern, string)) return(1);
          ++string;
        }
        return(0);
      case '[':
        if ((test = *string++) == EOS) return(0);
        if ((pattern = rangematch(pattern, test)) == NULL) return(0);
        break;
      case '\\':
        if ((c = *pattern++) == EOS) {
          c = '\\';
          --pattern;
        }
        if (c != *string++) return(0);
        break;
      default:
        if (c != *string++) return(0);
        break;
    }
}
