#ifndef udp_h
#define udp_h



/******************************************************
//1.RET_CODE*/
typedef int             RET_CODE;

/*/,2.E_PRIMITIVE_CODE*/

typedef enum
{
    ePRIM_CODE_UNKNOWN,
    
    /* primitve from call control to driver*/
    /* to control the endpoint*/
    ePRIM_EPT_CONTROL_OFFHOOK        =  0x1001,
    ePRIM_EPT_CONTROL_ONHOOK         =  0x1002,
    ePRIM_EPT_CONTROL_RING           =  0x1003,
    ePRIM_EPT_CONTROL_WINK           =  0x1004,
    ePRIM_EPT_CONTROL_FLASH          =  0x1005,
     
    ePRIM_EPT_CONTROL_STOP_SIGNAL    =  0x1010,
    ePRIM_EPT_CONTROL_STOP_DETECT    =  0x1011,
    ePRIM_EPT_CONTROL_PLAY_TONE      =  0x1012,
    ePRIM_EPT_CONTROL_TONE_COLLECT   =  0x1013,
    ePRIM_EPT_CONTROL_CALLER_ID      =  0x1014,
    ePRIM_EPT_CONTROL_COLLECT        =  0x1015,
    ePRIM_EPT_CONTROL_PLAY_DIGIT     =  0x1016,
    ePRIM_EPT_CONTROL_COLLECT_FSK    =  0x1017,
    ePRIM_EPT_CONTROL_DETECT_TONE    =  0x1018,

    ePRIM_EPT_CONTROL_DETECT_MEDIA   =  0x1019,

    ePRIM_EPT_CONTROL_STOP_FSK       =  0x101A,  /* stop fsk, on_hook, start ring timer  */    
    ePRIM_EPT_CONTROL_HOOKFLASH      =  0x101B,
    ePRIM_EPT_CONTROL_STOP_HOOKFLASH =  0x101C,

    ePRIM_EPT_CONTROL_CW_CID         =  0x101D,  /* call waiting caller ID*/
    ePRIM_EPT_CONTROL_FSK            =  0x101E,  /* FSK    */

    ePRIM_EPT_CONTROL_STOP_SIG_NO_REL    =  0x101F,  /* stop signal, but not release release*/
    
    ePRIM_EPT_CONTROL_AUDIO          =  0x1020, 
    ePRIM_EPT_CONTROL_AUDIO_COLLECT  =  0x1021,
    ePRIM_EPT_CONTROL_AUDIO_RECORD   =  0x1022,

    ePRIM_EPT_CONTROL_SEND_MF        =  0x1030,
    ePRIM_EPT_CONTROL_SEND_DTMF      =  0x1031,
    
    /* primitive from driver to call control*/
    /* notify of endpoint events*/
    ePRIM_EPT_NOTIFY_OFFHOOK         =  0x1101,
    ePRIM_EPT_NOTIFY_ONHOOK          =  0x1102,
    ePRIM_EPT_NOTIFY_RING            =  0x1103,
    ePRIM_EPT_NOTIFY_WINK            =  0x1104,    
    ePRIM_EPT_NOTIFY_FLASH           =  0x1105,
    ePRIM_EPT_NOTIFY_ERROR           =  0x1106,
    ePRIM_EPT_NOTIFY_FAX             =  0x1107,
    ePRIM_EPT_NOTIFY_MODEM           =  0x1108,    
    
    ePRIM_EPT_NOTIFY_DIGIT           =  0x1110,
    ePRIM_EPT_NOTIFY_FSK             =  0x1111,
    ePRIM_EPT_NOTIFY_AUTH            =  0x1112,        


    ePRIM_EPT_NOTIFY_DIGIT_OFF       =  0x1114,
    
    
    ePRIM_EPT_NOTIFY_MEDIA_START     =  0x1120,
    ePRIM_EPT_NOTIFY_PATTERN         =  0x1121,
    ePRIM_EPT_NOTIFY_CODEC_CHANG     =  0x1122,
    ePRIM_EPT_NOTIFY_TONE            =  0x1123,

    /* no media*/
    ePRIM_EPT_NOTIFY_MEDIA_END       =  0x1124,
    ePRIM_EPT_NOTIFY_POLARITY_REV    =  0x1125,        

    ePRIM_EPT_NOTIFY_FXO_OFFLINE     =  0x1126,
    ePRIM_EPT_NOTIFY_FXO_ONLINE      =  0x1127,
    ePRIM_EPT_NOTIFY_POLARITY_TIP    =  0x1128,
    ePRIM_EPT_NOTIFY_POLARITY_RING   =  0x1129,

    ePRIM_EPT_NOTIFY_FXO_INUSE       =  0x112A,
    ePRIM_EPT_NOTIFY_FXO_IDLE        =  0x112B,    

    ePRIM_EPT_NOTIFY_RING_ON         =  0x112C,
    ePRIM_EPT_NOTIFY_RING_OFF        =  0x112D,

    ePRIM_EPT_NOTIFY_POLARITY_NORMAL =  0x112E,        
    
    
    /* interrupt service routine send to driver*/
    ePRIM_FXS_INTERRUPT              =  0x1200,
    /* timeout thread send request to driver*/
    ePRIM_FXS_RING_CONTROL           =  0x1201,

    ePRIM_FXS_AUDIT                  =  0x1202,
    ePRIM_FXS_MANUAL_AUDIT           =  0x1203,
    ePRIM_FXS_MANUAL_DUMP            =  0x1204,
    ePRIM_FXS_LINE_TEST              =  0x1205,            
    
    ePRIM_FXO_INTERRUPT              =  0x1300,  /*add by hualei 20031021 to support FXO interrupt*/
    
    

    /* primitive from call control to driver for connection*/
    ePRIM_CALL_CREATE_CONN           =  0x2001,
    ePRIM_CALL_MODIFY_CONN           =  0x2002,
    ePRIM_CALL_DELETE_CONN           =  0x2003,
    ePRIM_CALL_QUERY_CONN            =  0x2004,

    ePRIM_CALL_CUT_THROUGH           =  0x2005,
    ePRIM_CALL_DISCONNECT            =  0x2006,
    
    ePRIM_CALL_CREATE_SUDO           =  0x2007,
    ePRIM_CALL_DELETE_SUDO           =  0x2008,    

    ePRIM_CALL_CONF_ADD              =  0x200A,  /* conference*/
    ePRIM_CALL_CONF_DROP             =  0x200B,
    ePRIM_CALL_CONF_CLEAR            =  0x200C,
    
    ePRIM_CONN_CONTROL_PLAY_TONE     =  0x2011,
    ePRIM_CONN_CONTROL_COLLECT       =  0x2012,

    /* primitive from driver to call control for connection*/
    ePRIM_CALL_CREATE_CONN_RPY       =  0x2101,
    ePRIM_CALL_MODIFY_CONN_RPY       =  0x2102,
    ePRIM_CALL_DELETE_CONN_RPY       =  0x2103,
    ePRIM_CALL_QUERY_CONN_RPY        =  0x2104,

    ePRIM_TDM_CREATE_CONN            =  0x2201,
    ePRIM_TDM_MODIFY_CONN            =  0x2202,
    ePRIM_TDM_DELETE_CONN            =  0x2203,
    ePRIM_TDM_CONTROL_PLAY_TONE      =  0x2204,
    ePRIM_TDM_CONTROL_STOP_TONE      =  0x2205,    
    
    /* web config */
    ePRIM_CONFIG_GET                 =  0x3001,  
    ePRIM_CONFIG_POST                =  0x3002,
    ePRIM_CONFIG_GET_RPY             =  0x3101,
    ePRIM_CONFIG_POST_RPY            =  0x3102,


    /* hdlc*/
    ePRIM_HDLC_ENABLE                =  0x4001,
    ePRIM_HDLC_DISABLE               =  0x4002,
    ePRIM_HDLC_ENABLE_RPY            =  0x4011,
    ePRIM_HDLC_DISABLE_RPY           =  0x4012,
    
    ePRIM_HDLC_DATA                  =  0x4020,
    ePRIM_HDLC_STATUS                =  0x4021,

    /* trunk*/
    ePRIM_TRK_ENABLE                 =  0x4030,
    ePRIM_TRK_DISABLE                =  0x4031,
    ePRIM_TRK_ENABLE_RPY             =  0x4032,
    ePRIM_TRK_DISABLE_RPY            =  0x4033,

    ePRIM_TRK_STATUS                 =  0x4034,
    
    /* call fsm primitives
    // the primitive codes are defined in <call_event.h>*/
    ePRIM_CALL_FSM_MIN               =  0x5000,
    ePRIM_CALL_FSM_MAX               =  0x5100,
    
    /* primitive from transaction to sip ua*/
    ePRIM_UA_1XX_CFM                 =  0x5101,
    ePRIM_UA_2XX_CFM                 =  0x5102,
    ePRIM_UA_3456XX_CFM              =  0x5103,
    ePRIM_UA_INVITE_IND              =  0x5104,
    ePRIM_UA_REQUEST_IND             =  0x5105,
    ePRIM_UA_ACK_IND                 =  0x5106,
    ePRIM_UA_CANCEL_IND              =  0x5107,
    ePRIM_UA_STATUS_IND              =  0x5108,
    ePRIM_UA_LONG_TO                 =  0x5109,
    ePRIM_UA_SHORT_TO                =  0x510A,
    ePRIM_UA_UPDATE_INDEX            =  0x510B,
    ePRIM_UA_OPTION                  =  0x510C,    

    /* ePRIM_SIP_UA_REGISTRATION        =  0x5130,
    // ePRIM_SIP_UA_REG_CANCEL          =  0x5131,*/
    
    ePRIM_SIP_TRANS_MIN              =  0x5200,
    ePRIM_SIP_TRANS_MAX              =  0x5299
    
    
}E_PRIMITIVE_CODE;

/*,3.S_PRIMITIVE*/

typedef enum
{
	eCOMPONENT_SYSTEM = 1,

	eCOMPONENT_CALL,
	eCOMPONENT_DRIVER,

    eCOMPONENT_SIP_UA,
    eCOMPONENT_SIP_TRANSACTION,
    eCOMPONENT_SIP_UDP,    

    eCOMPONENT_BOA,
    eCOMPONENT_CONFIG,
    
    eCOMPONENT_MAX,

	eCOMPONENT_CLI,
	eCOMPONENT_SNMP,
	eCOMPONENT_RTP
    
    
} E_COMPONENT_ID;

typedef struct
{
	E_COMPONENT_ID   component_id;

} S_COMPONENT;

typedef unsigned int    UINT32;
typedef int             INT32;
typedef unsigned short  UINT16;
typedef short           INT16;
typedef unsigned char   UINT8;
typedef char            INT8;


typedef long            LONG;
typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef unsigned char   BOOL;
typedef void            VOID;
typedef unsigned char   BOOL8;

typedef struct _list_node   /* Node of a linked list. */
{
    struct _list_node *next;      /* Points at the next node in the list */
    struct _list_node *previous;  /* Points at the previous node in the list */
    
} NODE;

#define  nMAX_INBAND_PRIM_PAYLOAD_SIZE    150
typedef struct
{
    NODE               node;
    
    UINT8              bInUse;
    UINT8              reserve[3];
    
	E_PRIMITIVE_CODE   eCode;  /* primitive code*/

	S_COMPONENT        source;
	S_COMPONENT        dest;

    INT32              status;  /* error code*/
    UINT32             cookie;
    
	UINT32             payload_length;
	void*              payload;

    char               data[nMAX_INBAND_PRIM_PAYLOAD_SIZE];
    
} S_PRIMITIVE;


/*,4.S_CONFIG*/

typedef struct
{
    int            length;  /* length of the whole structure, include alignment*/
    int            type;
    char           data;
    
} S_CONFIG_DATA;

typedef enum
{
    eCFG_CAT_NONE,
    eCFG_CAT_SYSTEM       = 1,
    eCFG_CAT_PROFILE      = 2,
    eCFG_CAT_FEATURE      = 3,
    eCFG_CAT_TDM          = 4,
    eCFG_CAT_NETWORK      = 5,    
    eCFG_CAT_OPTIONAL     = 6,
    eCFG_CAT_REBOOT       = 7,
    
    eCFG_CAT_MSGLOG       = 8,
    eCFG_CAT_PASSWORD     = 9,        

    /* signaling*/
    eCFG_CAT_MGCP         = 10,
    eCFG_CAT_SIP          = 11,
    eCFG_CAT_ISDN         = 12,

    /* list*/
    eCFG_CAT_DIGITMAP     = 13,
    eCFG_CAT_ROUTE        = 14,
    eCFG_CAT_IPTABLE      = 15,

    eCFG_CAT_CHANGEPW     = 16,

    /* information*/
    eCFG_CAT_LINE_INFO    = 17,
    eCFG_CAT_RES_INFO     = 18,
    eCFG_CAT_ISDN_INFO    = 19,

    /* line config*/
    eCFG_CAT_LINE         = 20,
    
    /* feature lated */
    eCFG_CAT_FEATURE1     = 21,  /* all/busy/noans  forward*/
    eCFG_CAT_FEATURE2     = 22,  /* busy forward*/
    eCFG_CAT_FEATURE3     = 23,  /* noans forward*/
    eCFG_CAT_FEATURE4     = 24,  /* fashion ring*/
    eCFG_CAT_FEATURE5     = 25,  /* feature flags*/
    eCFG_CAT_FEATURE6     = 26,  /* hot line*/
    eCFG_CAT_FEATURE7     = 27,  /* speed dialing / fork*/
    eCFG_CAT_FEATURE8     = 28,  /* cell number*/
    eCFG_CAT_FEATURE9     = 29,  /* not using*/

    /* isdn call info */
    eCFG_CAT_ISDN_CALL    = 30,
    
    eCFG_CAT_FXO          = 31,
    eCFG_CAT_ENDPOINT     = 32,
    eCFG_CAT_EPT_ID       = 33,            

    eCFG_CAT_LOGOUT       = 34,        
    eCFG_CAT_UPDATE	      = 35,  /* helon update*/
    eCFG_CAT_RESET	      = 36,  /* reset data to factory setting*/

    eCFG_CAT_TIETRUNK      = 37,
    eCFG_CAT_GROUP_CONFIG  = 38,
    /* eCFG_CAT_CHINESE    = 37,  // set language to chinese
    // eCFG_CAT_ENGLISH    = 38,  // set language to english*/

	eCFG_CAT_CFG_FILE_OUT = 39,
	eCFG_CAT_LOG_FILE_OUT = 40,

	eCFG_CAT_STAT_INFO    = 39,
 	eCFG_CAT_MSTAT_INFO   = 40,
    
    eCFG_CAT_AUTO         = 41,
    
    /* the following are optional, */
    eCFG_CAT_DSP_OPT      = 42,
    eCFG_CAT_TONE_OPT     = 43,
    eCFG_CAT_FXO_OPT      = 44,
    eCFG_CAT_FXS_OPT      = 45,            
    eCFG_CAT_EMS_OPT      = 46,
    eCFG_CAT_ACS_OPT      = 47,    
 
    eCFG_CAT_GW           = 48,     /* SBC*/
    eCFG_CAT_SS           = 49,     
    eCFG_CAT_MS           = 50,     
   	eCFG_CAT_CALL_INFO    = 51,     
   	eCFG_CAT_PHONE_NUMBER = 52,     
    
	eCFG_CAT_IP_PHONE     = 56,
	eCFG_CAT_IP_TRUNK     = 57,
	eCFG_CAT_ISDN_MAP     = 58,    
    
  
    
    eCFG_CAT_CTI          = 60,        
	eCFG_CAT_DDL          = 61,
	eCFG_CAT_PDL          = 62,
	eCFG_CAT_XML          = 63,
	eCFG_CAT_QUERY        = 64 

    
} E_CFG_CAT;

typedef struct s_config
{
    E_CFG_CAT      eCategory;
    BOOL           flag_error;
    BOOL           flag_reset;
    BOOL           flag_continue;
    UINT8          index;
    UINT32         cookie;
    UINT32         remote_ip;
    int            num_variable;
    S_CONFIG_DATA  var_start;
    
} S_CONFIG;




 

/* parse app response data */
int parse_app_response_getid638(char* buf,int length);

int parse_app_response_setid638(char* buf,int length);

/* rc to app send data */
RET_CODE  rcToAppPrimitive(E_PRIMITIVE_CODE eCode, void *payload ); 

/*app to rc recv data */
int appTorcPrimitive(int method);

int close_fsp_socket(void);

int create_fsp_socket(void);

int read_WEB_PASSWD(void);

int GetApp_Id638(void);

int SetApp_Id638(void);

int rand_pass(void);

int GetApp_Id1103(void);

int update_INVITE_CODE(void);

int update_REC_PASSWD(void);



#endif





