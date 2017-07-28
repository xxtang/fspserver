#ifndef _LOG_H
#define _LOG_H

#include "stdio.h"
#include "stdarg.h"
#include <sys/time.h>


/* #include <sysglobal.h>
// #include <lib_list.h>
// #include <lib_hash.h>
// #include "types.h"
// #include "primitive.h"*/

#define nMAX_EVENT_LOG        50  /* 50K events*/
#define nMAX_EVENT_LOG_LEVEL  10

typedef enum
{
    eLOG_TYPE_NONE,
    eLOG_TYPE_FILE,
    eLOG_TYPE_SCREEN,
    eLOG_TYPE_RAM,
    eLOG_TYPE_MAX
    
} E_LOG_TYPE;

typedef enum
{
    eLOG_DUMP_SCREEN,
    eLOG_DUMP_FILE

} E_LOG_DUMP;

typedef enum
{
    eLOG_COMP_SYSTEM,
    eLOG_COMP_SNMP,
    
    eLOG_COMP_MAX

} E_LOG_COMP;

typedef enum
{
    eLOG_SEVERITY_TRAP,
    eLOG_SEVERITY_ERROR,
    eLOG_SEVERITY_DEBUG,
    eLOG_SEVERITY_TRACE,

    eLOG_SEVERITY_INFO = 6

} E_LOG_SEVERITY;



typedef struct S_LOG_INFO S_LOG_INFO;
struct S_LOG_INFO
{
    char           filename[128];
    FILE          *fd;
/*    LIST           list;*/
    unsigned long  current_index;
    unsigned short file_size;    /* K*/
    unsigned char  num_backup_file;
    unsigned char  flag_to_file;
    unsigned char  flag_rotating;
    unsigned char  flag_logging;
    unsigned short miss_event_counter;
    unsigned long  total_miss_event_counter;
    unsigned char  flag_rotate;
    
} ;

typedef enum
{
    eLOG_INFO_DEBUG,
    eLOG_INFO_BOOT,
    eLOG_INFO_ERROR,

    eLOG_INFO_MAX
  
} E_LOG_INFO;

typedef struct
{
    S_LOG_INFO  log_info[1];
    
    unsigned char log_type;  /* file / ram / remote ...*/
    char          log_level[eLOG_COMP_MAX];
    
} S_EVENT_LOG_GLOBAL;


extern S_EVENT_LOG_GLOBAL gEventLogGlobal;


#ifdef __cplusplus
extern "C"{
#endif

int    logger(     const char* file_name, int line_num, const char* format, ... );      // debug log need to have log level
int    debug_log(  int comp, int level, const char* file_name, int line_num, const char* format, ... );
int    debug_log_line( int component, int level, const char* full_name, int line_num, int fline, int tline, const char* format, ... );


#define SNMP_DEBUG_LOG(format...) \
{\
    debug_log( eLOG_COMP_SNMP, 3, __FUNCTION__, __LINE__, ##format);\
}

#define SNMP_DEBUG_LOG_FLINE(fline, format...)  \
{\
    debug_log_line(eLOG_COMP_SNMP, 3, __FUNCTION__, __LINE__, fline, 0,  ##format);\
}

#define SNMP_DEBUG_LOG_TLINE(tline, format...)  \
{\
    debug_log_line(eLOG_COMP_SNMP, 3, __FUNCTION__, __LINE__, 0, tline,  ##format);\
}

#define SNMP_DEBUG_LOG_LINE(fline, tline, format...)  \
{\
    debug_log_line(eLOG_COMP_SNMP, 3, __FUNCTION__, __LINE__, fline, tline,  ##format);\
}

    
int    err_log( const char* file_name, int line_num, const char* format, ... );        // always log
int  err_log_line( const char* full_name, int line_num, int fline, int tline, const char* format, ... );


#define SNMP_ERR_LOG(format...)\
{\
        err_log(__FUNCTION__, __LINE__, ##format);\
}\

#define SNMP_ALAWAYS_LOG(format...)\
{\
        err_log(__FUNCTION__, __LINE__, ##format);\
}


#define SNMP_ALAWAYS_LOG_FLINE(fline, format...)\
{\
        err_log_line(__FUNCTION__, __LINE__, fline, 0,  ##format);\
}

#define SNMP_ALAWAYS_LOG_TLINE(tline, format...)\
{\
        err_log_line(__FUNCTION__, __LINE__, 0, tline,  ##format);\
}

#define SNMP_ALAWAYS_LOG_LINE(fline, tline, format...)\
{\
        err_log_line(__FUNCTION__, __LINE__, fline, tline, ##format);\
}



int    boot_log(   const char* file_name, int line_num, const char* format, ... );

#define SNMP_BOOT_LOG(format...)\
{\
    boot_log(__FUNCTION__, __LINE__, ##format);\
}\

    // vlog with vlist
int    vdebug_log(  int comp, int level, const char* full_name, int line_num,
                    const char* format, const char* out_str, va_list *argList );
int    vSNMP_ERR_LOG(    const char* file_name, int line_num, const char* format, va_list *argList );
    
    
    
    // component - CALL, MGCP_MSG, MGCP, SDP, DRIVER, SYSTEM, FUNC 
int    set_event_log_level(  int component,  int level );
int    get_event_log_level(  int component );    
int    set_event_log_endpoint(  int eptid,   int level );  // only effect ept_log
int    get_event_log_endpoint(  int eptid );  // only effect ept_log    
int    set_event_log_type(   int log_type );
int    show_event_log_config();
int    dump_event_log(  int type );

int    web_get_app_log( char *ret_buffer, int length, int type ); 


int	   mib_test(char* cmd);

#if 0
#define  FUNC_ENT()        func_ent( __FUNCTION__ )
#define  FUNC_EXIT(rc)     func_exit( __FUNCTION__, rc )
#define  FUNC_MARK(rc)     func_exit( __FUNCTION__, rc )    
#else
#define  FUNC_ENT()   
#define  FUNC_EXIT(rc)
#define  FUNC_MARK(rc)
#endif
    
void func_ent( const char *func );
void func_exit( const char *func, int rc );    

void   log_init();
void   log_term();



#ifdef __cplusplus
}
#endif

#endif // _LOG_H





