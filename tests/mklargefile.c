/* this file is public domain */
/* return values:
 * 3 - invocation error
 * 2 - no LFS support
 * 1 - file creation failed
 * 0 - okay!
 */

#include "tweak.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
    off_t GB=1024*1024*1024;
    FILE *fd;
    off_t pos;
    float size;

    if(argc<3)
    {
	printf("Makes a large file with hole using fseeko\n");
	printf("%s <filename> <floating point size in GB>\n",argv[0]);
	return 3;
    }

    fd = fopen(argv[1],"w");
    if(fd == NULL)
    {
	perror("fopen");
	return 1;
    }
    size=atof(argv[2]);
#if SIZEOF_OFF_T <= 4
    if(size>2)
    {
	printf("You do not have LFS your system.\nMaximum supported filesize is 2 GB.\n");
	return 2;
    }
#endif	
    pos=size*GB;
    if(fseeko(fd,pos,SEEK_SET))
    {
	perror("fseeko");
	return 1;
    }
    if(fwrite("!",1,1,fd)<1)
    {
	perror("write");
	return 1;
    }
    if(fclose(fd))
    {
	perror("fclose");
	return 1;
    }
    return 0;
}
