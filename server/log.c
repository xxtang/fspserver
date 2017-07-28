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
#include <stdarg.h>

#define MAX_LOGFILE_SIZE 1024*512
int logfd = -1;  /* logfile file descriptor */
int tlogfd = -1; /* transfer log file descriptor */
int logfile_size = 0;/* the current size of logfile */  //Add by xxfan
int tlogfile_size = 0;/*the current sizeo of tlogfile */  //Add by xxfan

/****************************************************************************
 * A slightly better logging function.. It now takes a format string and    *
 * any number of args.                                                      *
 ****************************************************************************/

#define LOGBUFFER 1024+80
static char logbuf[LOGBUFFER];	/* buffer for log message */
static size_t logpos = 0;	/* current log message length */

/* append some text to the log message */
void fsploga(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	/* fmt = va_arg(args, char *); */
	vsprintf(logbuf + logpos, fmt, args);
	logpos += strlen(logbuf + logpos);
	va_end(args);
}

/* add a datestamp to begining of the log message, clear text in buffer */
void fsplogs (void)
{

	int timelen;
	char *timestr;
	time(&cur_time); 
	timestr = (char *)ctime(&cur_time);
	timelen = strlen(timestr) - 1; /* strip the CR */
	timestr[timelen] = '\0';
	strcpy(logbuf, timestr);
	logbuf[timelen] = ' ';
	logpos = timelen + 1;
}

/* flush logfile */
void fsplogf (void)
{
	int nres;
	if(dbug) 
	{
		fwrite("[LOG] ",6,1,stdout);
		fwrite(logbuf,logpos,1,stdout);
		fflush(stdout);
	}

	nres=write(logfd, logbuf, logpos);
	//Add by xxfan
	if(nres>0)
	{
		logfile_size+=nres;
		logfile_check(logname,&logfile_size,MAX_LOGFILE_SIZE,&logfd);
	}
	//?end
	logpos = 0;
}

//add by xxfan 2016-03-23
//?start
void logfile_check(char* file_name,int* cur_file_size,int max_file_size,int* file_fd)
{
	if(*cur_file_size>=max_file_size)
	{
		close(*file_fd);
		logfile_backup(file_name,".1");
		(*file_fd) = open(file_name,O_WRONLY | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
		*cur_file_size = 0;
	}
}
//?end
//add by xxfan 2016-03-23
//?start
void logfile_backup(char* old_filename,char* suffix)
{
	char new_filename[256];

	if(access(old_filename,F_OK) != 0)
		return;
	sprintf(new_filename,"%s%s",old_filename,suffix);
	if(access(new_filename,F_OK) == 0)
		remove(new_filename);
	rename(old_filename,new_filename);
}
//?end

/* wu ftpd xferlog subsystem */
static char tlogbuf[LOGBUFFER];	/* buffer for log message */

void xferlog(char direction, const char *filename,unsigned long filesize,const char *hostname)
{
	size_t pos=0,timelen;
	int nres;
	char *timestr;

	if(!tlogname) return; /* xfer logging is not enabled */

	/* current-time */
	timestr = (char *)ctime(&cur_time);
	timelen = strlen(timestr) - 1; /* strip the CR */
	timestr[timelen] = '\0';
	strcpy(tlogbuf, timestr);
	pos = timelen;
	/* transfer-time, remote-host, file-size, file-name, ... */
	pos+=sprintf(tlogbuf+pos," 1 %s %lu %s b * %c a fsp fsp 0 * c\n",hostname,filesize,filename,direction);
	nres = write(tlogfd, tlogbuf, pos);

	//Add by xxfan 2016-03-23
	//?start
	if(nres > 0)
	{
		nres += tlogfile_size; 
		logfile_check(filename,&tlogfile_size,MAX_LOGFILE_SIZE,&tlogfd);
	}
	//?end
}
