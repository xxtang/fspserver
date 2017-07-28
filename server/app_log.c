// sys header file
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "app_log.h"

#define nMAX_BACKUP_FILE      1
#define nMAX_FILE_SIZE        100          // 100K
#define LOCAL                 static
#define LOG_FILE_DIR          "/var/log/fspd_error.log"

int log_level = 1;   // default

LOCAL  void rotatefile( S_LOG_INFO * log_info, int force_flag );
void   log_init_file();


S_EVENT_LOG_GLOBAL gEventLogGlobal;

int  _logger( S_LOG_INFO *log_info, const char* full_name, int line_num, int fline, int tline,
              const char* format, const char* out_str, va_list *argList );

void log_init_each_type ( S_LOG_INFO *log_info, const char *filename  )
{
    memset( log_info, 0, sizeof(S_LOG_INFO) );

    if ( strlen( filename )+1 > sizeof( log_info->filename ) )
    {
        printf( "ERROR: log filename is too long\n" );
    }
    else
    {
        strcpy( log_info->filename, filename );
    }

    log_info->num_backup_file = nMAX_BACKUP_FILE;
    log_info->file_size       = nMAX_FILE_SIZE;  // in KB
    log_info->flag_rotate     = 1;

}

void log_init()
{
    
	FILE *fp;
	if((fp=fopen(LOG_FILE_DIR,"wb"))==NULL) /*建立c:\45.CH文件*/
	{ 
		printf("\nopen file error!\n"); 
		getchar(); 
		exit(1); 
	}
	else
	{
		fclose(fp);
	}

	log_init_each_type( &gEventLogGlobal.log_info[eLOG_INFO_DEBUG], LOG_FILE_DIR );
	log_init_file();
	
}

void log_init_each_file( S_LOG_INFO *log_info )
{
    if ( log_info->fd == NULL )
    {
        // force rotate the file
        rotatefile( log_info, log_info->flag_rotate );
    }
}

void log_init_file()
{
    log_init_each_file( &gEventLogGlobal.log_info[0]  );
}

int  debug_log( int component, int level,
                const char* full_name, int line_num, const char* format, ... )
{
    va_list argList;

    if ( level > log_level )
        return -1;

    va_start( argList, format );
    _logger( &gEventLogGlobal.log_info[0], full_name, line_num, 0, 0, format, "", &argList );
    va_end( argList );

    return 0;
}

int  debug_log_line( int component, int level,
                const char* full_name, int line_num, int fline, int tline, const char* format, ... )
{
    va_list argList;

    if ( level > log_level )
        return -1;

    va_start( argList, format );
    _logger( &gEventLogGlobal.log_info[0], full_name, line_num, fline, tline, format, "", &argList );
    va_end( argList );

    return 0;
}


// boot log entry
int  boot_log( const char* full_name, int line_num, const char* format, ... )
{
    va_list	argList;

    va_start( argList, format );
    _logger( &gEventLogGlobal.log_info[0], full_name, line_num, 0, 0, format, "", &argList );
    va_end( argList );

    return 0;
}

// error log entry
int  err_log( const char* full_name, int line_num, const char* format, ... )
{
    va_list	argList;
    
    va_start( argList, format );
    _logger( &gEventLogGlobal.log_info[0], full_name, line_num, 0, 0, format, "", &argList );
    va_end( argList );

    return 0;
}

int  err_log_line( const char* full_name, int line_num, int fline, int tline, const char* format, ... )
{
    va_list	argList;
    
    va_start( argList, format );
    _logger( &gEventLogGlobal.log_info[0], full_name, line_num, fline, tline, format, "", &argList );
    va_end( argList );

    return 0;
}


void  _logger_to_file( FILE       *log_file, const char* filename, int line_num, int fline, int tline,
                       const char *format,   const char* str_out,
                       va_list    *argList,
                       struct tm  *tm,
                       struct timeval *now  )
{
    int nline = 0;

    if ( filename != NULL )
    {                
         nline = 0;
         while (nline < fline)
         {
             fprintf( log_file, "\n");
             ++nline;
         }
        
        if ( tm != NULL )
            fprintf(  log_file, "[%02d:%02d:%02d.%06ld]", tm->tm_hour, tm->tm_min, tm->tm_sec, now->tv_usec );
                  
        if ( line_num != 0 )
            fprintf(  log_file, " %s(%d) - %s",  filename, line_num, str_out );
        else
            fprintf(  log_file, " %s() - %s",    filename, str_out );         

        vfprintf( log_file, format, *argList );

        nline = 0;
        do 
        {
            fprintf( log_file, "\n");        
            ++nline;
        }while (nline <= tline);
    }
    else
    {
         for (nline = 0; nline < line_num; ++nline)
            fprintf( log_file, "\n");
    }
}



int  _logger( S_LOG_INFO *log_info, const char *full_name, int line_num, int fline, int tline,
              const char *format,   const char *str_out,
              va_list    *argList )
{

    const char    *filename = NULL;
    struct tm     *tm = NULL;
    struct timeval now;
    int            err;

    if ( full_name !=NULL)
    {
        filename = strrchr( full_name, '/' );
        if ( filename )
            filename ++;
        else
            filename = full_name;        
    }
   
    err = gettimeofday( &now, NULL );
    if ( err == 0 )
        tm  = localtime( &now.tv_sec );

    if ( !log_info->flag_rotating )  // file not in rotating
    {
        if ( log_info->fd != NULL )
        {
            if ( !log_info->flag_logging )
            {
                log_info->flag_logging = 1;
                _logger_to_file( log_info->fd, filename, line_num, fline, tline,
                                 format, str_out, argList,
                                 tm, &now );
                fflush( log_info->fd );
                rotatefile( log_info, 0 );
                log_info->flag_rotating = 0;
                log_info->flag_logging = 0;
                return 0;
            }
        }
        // TBD: should never come here
    }
    else
    {
        // TBD: queue the log
        //_logger_to_ram( log_info, filename, line_num, format, str_out, argList, tm, &now );
        log_info->total_miss_event_counter++;
        return 1;
    }
    return -1;
}

void log_term_file_each_type( S_LOG_INFO *log_info  )
{
    if ( log_info->fd )
    {
        fclose( log_info->fd );
        log_info->fd = NULL;
    }
}

void log_term_file()
{
    log_term_file_each_type( &gEventLogGlobal.log_info[0] );
}

void log_term()
{
     log_term_file();
}

LOCAL void rotatefile( S_LOG_INFO * log_info, int force_flag )
{
    /* First double-check the log file size, to avoid a race condition.
    It is possible that, between the time rotateFiles was called and
    the present moment, some other thread has attempted to log a message
    (using vCpLog), noticed that fileInfo.st_size +. SIZE_PER_LOGFILE
    (in rotateFilesIfNecessary), and rotated the logs out from under us.
    */

    struct stat fileInfo;
    int i;

    if ( log_info->flag_rotating )
        return;

    log_info->flag_rotating = 1;

    if ( log_info->fd )
    {
        if ( fstat( fileno(log_info->fd), &fileInfo ) != 0 )
        {
            perror( "fstat:" );
            goto Exit;
        }
    }
    else
    {
        if (stat (log_info->filename, &fileInfo) != 0)
        {
            // error
            perror( "stat:" );
            goto Exit;
        }
    }

    if ( fileInfo.st_size == 0 ||
         ( force_flag == 0 && fileInfo.st_size < log_info->file_size * 1000 ) )
    {
        //if ( force_flag == 1 )
        //    printf( "file size =%d (%d)\n", (int)fileInfo.st_size, force_flag );
        goto Exit;
    }

    //printf( "file size=%d\n", fileInfo.st_size );


    /* Close the current log file */
    if( log_info->fd && fclose ( log_info->fd ) )
    {
        // error
    }
    log_info->fd = NULL;

    /* Make room for the new log file */
    for( i = log_info->num_backup_file  - 1; i >= 0; i-- )
    {
        char  oldFilename[132];
        char  newFilename[132];
        int   length;

        strcpy( oldFilename, log_info->filename );
        length = strlen( oldFilename );
        if( i > 0 )
        {
            oldFilename[length] = '.';
            sprintf( oldFilename+length+1, "%d", i );
        }
        // printf( "need to rename the file - %s\n", oldFilename );
        if (stat (oldFilename, &fileInfo) == 0) /* if the file _does_ exist... */
        {
            strcpy( newFilename, log_info->filename );
            newFilename[length] = '.';
            sprintf( newFilename+length+1, "%d", i+1 );

            if (rename (oldFilename, newFilename) != 0) /* If rename() fails... */
            {

                printf( "fail to rename the file - %s to %s\n", oldFilename, newFilename );
                // error
            }
        }
    }

    /* Open the log file for writing once more.  (The current log file will
    always have the name without a numeric extension.)*/
 Exit:
    if ( log_info->fd == NULL )
    {
        log_info->fd = fopen( log_info->filename,  "a" );
        if ( log_info->fd == NULL )
        {
            printf( "Fail to open log file - %s\n", log_info->filename );
        }
        else
        {
            if ( log_info->miss_event_counter )
            {
                fprintf( log_info->fd, "log miss events: %d\n", log_info->miss_event_counter );
                log_info->miss_event_counter = 0;
            }
        }

    }

    log_info->flag_rotating = 0;
}

int mib_test(char* cmd)
{
	return 1;
}








