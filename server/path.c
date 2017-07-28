/*********************************************************************\
 *  Copyright (c) 2003 by Radim Kolar (hsn@cybermail.net)              *
 *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
 *                                                                     *
 *  You may copy or modify this file in any manner you wish, provided  *
 *  that this notice is always included, and that you hold the author  *
 *  harmless for any loss or damage resulting from the installation or *
 *  use of this software.                                              *
 \*********************************************************************/

#include "tweak.h"
#include "server_def.h"
#include "s_extern.h"
#include "my-string.h"

/*****************************************************************************
 * Routine to parse a path string given by the client.
 * input: fullp - pointer to path for parsing
 *        len   - length of the path for parsing
 * Will replace null string by ".".
 * In case of error, returns the error string.
 *
 *  The PPATH structure is filled in by the function check_path when given a
 *  path string.  The elements are filled in as such:
 *
 *	fullp      pointer to a string containing the full path name
 *	f_ptr      pointer to begining of the last component of the path
 *      f_len      length of the last component of the path
 *	d_ptr      pointer to begining of the directory part of path
 *      d_len      length of the directory part of the path
 *      passwd     ptr to password
 *
 *  fullp is a null-terminated full path string.
 *  f_ptr is always a null-terminated sub-string of fullp.
 *  d_ptr is generally not null-terminated.
 *****************************************************************************/

const char *parse_path (char * fullp, unsigned int len, PPATH * pp)
{
	char *s;
	char *p;
	int state;

	if(len < 1) return("Path must have non-zero length");
	if(fullp[len-1]) return("Path must be null terminated");

	pp->passwd = NULL; 			/* default, no password */
	pp->d_len = 0;

	
	for(
			s = pp->fullp = pp->f_ptr = pp->d_ptr = fullp, state = 0;
			*s;
			s++
	   )
	{
		if(*s == '\n')
		{
			p=strchr(pp->fullp,'\n');
			pp->passwd = p+1;
			*p = '\0';
			*s = '\0';

			if(dbug) fprintf(stderr,"parse_path: parse  password-%s from date\n", p+1);
			break;
		}
		else
			if(*s < ' ') return("Path contains control chars");
			else
				if(*s >= '~') return("Path contains high chars");

		switch(*s)
		{
			case '\\':
			case '.':
				if(state==0) return("Path can't begin with '.' or '\\'");
				if(state==2) return("Files can't begin with '.'");
			default:
				state = 1;
				break;
			case '/':
				pp->f_ptr=s+1;
				switch(state)
				{
					case 0:
						/* state 0: front slashes */
						pp->fullp=s+1;
						pp->d_ptr=s+1;
						pp->d_len=0;
						break;
					case 1:
						/* state 1: slash after nonslash */
						pp->d_len= s-pp->fullp;
						state=2;
						break;
					case 2:
						break;
				}
		}
	}

	/*  pp->d_len = pp->f_ptr - pp->fullp; */

	/* turn empty directory into "." */
	if(pp->d_len == 0)
	{
		pp->d_ptr = ".";
		pp->d_len = 1;
	}

	pp->f_len = s - pp->f_ptr;

	/* turn empty file into "." */
	if(pp->f_len == 0)
	{
		pp->f_ptr = ".";
		pp->f_len = 1;
	}

	/* turn empty fullp into "." */
	if(pp->fullp[0] == 0)	
	{
		/* null path --> root */
		pp->fullp = ".";
	}

	return(NULLP);
}
