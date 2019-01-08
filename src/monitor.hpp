#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <asm/types.h>
//#include <mtd/mtd-user.h>
#include <sys/inotify.h>
//#include "inotify.h"
#include <pthread.h>
#include <dirent.h>
#include "config_file.hpp"
#include "param.hpp"
#include <sys/ipc.h>
#include <sys/msg.h>
//#include "ipc_resource.h"


#define  MAX_REPORT_NUM      10
#define  MAX_PROCESS_NUM     25
#define  MAX_PATH_NAME       64
#define  MAX_TITLE_SIZE      30
#define  MAX_RESULT_SIZE     10
#define  LEN_TIME            6
#define  MAX_WHITE_PROCESS_NUM  50
#define  LEN_PROCESS_NAME    30
#define  MAX_IP_LEN          4
#define  MAX_PORT_LEN        2
#define  MAX_WHITE_NET_NUM   20
#define  MAX_DIR_NUM         10
#define  LEN_PATH            33
#define  MAX_LOGIN_NUM       50
#define  LEN_MSG             7
#define  MAX_PORT_RECORD     100
#define  MAX_NET_RECORD      30
#define  MAX_SPOT_DATA_LEN   250
#define  UEVENT_BUFFER_SIZE  2048  
#define  M_MSG_LEN	         2048
#define  MODULE_NAME         "monitor"
#define  DEV_RTC             "/dev/wfet1000clock"
#define  REPORT_PATH         "/mnt/report"
#define  DATA1               "/data1/"
#define  DATA1CFG            "/data1/conf/"
#define  HOME                "/home/et1000/"
#define  HOMECFG             "/home/et1000/conf/"
#define  PROCESS_STAT        "/proc/"
#define  PROCESS_WHITE_LIST  "/mnt/whitelist"
#define  SECURITY_PARAM      "/mnt/securityparam"
#define  NET_PORT_EVENT_LOG  "/mnt/net_port"
#define  NET_IP_EVENT_LOG    "/mnt/net_ip"
#define  INOTIFY_MASK       (IN_MODIFY|IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF)		
#define  INOTIFY_MASK_EX    (IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF)




#ifdef DLIBLOG
#define TRACE(fmt, args...) do { \
	FILE *fp = fopen("/mnt/monitorlog","a+");\
	fprintf(fp, "[%s:%s]%s:%d - "fmt"\n", \
	__DATE__,__TIME__,__FUNCTION__, __LINE__, ##args);\
	fclose(fp); \
	} while (0)	
#define __ASSERT(exp)   ((void)((exp) || printf("%s:%d fatal error!!!",__FUNCTION__,__LINE__)))

#define __VERIFY(exp)   (__ASSERT(exp))

#else
#define TRACE(fmt, args...) 
#define __ASSERT(exp)
#define __VERIFY(exp)    ((void)(exp))

#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif



#define __attribute_section__(S)	__attribute__((section(#S)))
#define __attribute_packed          __attribute__((packed))
#define __attribute_used            __attribute_used__

#ifndef OK
#define OK    0
#endif 

#ifndef  ERROR
#define ERROR    -1
#endif

#ifndef  EXIST
#define  EXIST    1
#endif

#ifndef  NONE    
#define  NONE     0
#endif

#ifndef  REPORTED
#define  REPORTED  		100
#endif

#ifndef  NOTREPORT
#define  NOTREPORT 		99
#endif

/**
* errno just used for this module
*/
#define __get_errno       (errno)

#define __set_errno(v)    do { \
	errno = v; \
	} while (0)

#define set_bit(x,y)    (x |= (1 << y))





typedef enum{
	//processes
	E_MENU_STAT_ERROR                    = -1000,
	E_TASKMNG_STAT_ERROR,
	E_CONTROL_STAT_ERROR,
	E_HOMONIC_STAT_ERROR,
	E_EVENTRECORD_STAT_ERROR,
	E_MNG513_STAT_ERROR,
	E_ALARM_STAT_ERROR,
	E_EVENTGRADE_STAT_ERROR,
	E_READMETER_STAT_ERROR,
	E_ANALYSIS_STAT_ERROR,
	E_PROPARSE_STAT_ERROR,
	E_DEVMON_STAT_ERROR,
	E_VOICETASK_STAT_ERROR,
	E_PWMNG_STAT_ERROR,
	E_RELAYTASK_STAT_ERROR,
	E_MEASGATHER_STAT_ERROR,
	E_EXPTPROC_STAT_ERROR,
	E_WATCHGPRS_STAT_ERROR,
	//system resource
	E_MEMUSED_EXCEED_LIMIT              = -2000,
	E_FLASHUSED_EXCEED_LIMIT,
	E_CPUUSAGE_EXCEED_LIMIT,
	E_LOADAVG_EXCEED_LIMIT,
	E_BADFLASH_EXCEED_LIMIT,
	//clock change
	E_CLOCK_CHANGE_EXCEED_LIMIT         = -3000,
	//semaphore
	E_ARRAYUSED_EXCEED_LIMIT            = -4000, 
	E_SEMAALLOC_EXCEED_LIMIT,
	//message queue
	E_MSGNUM_EXCEED_LIMIT               = -5000,
	E_QUEUEALLOC_EXCEED_LIMIT,
	//share memory
	E_SEGSALLOC_EXCEED_LIMIT            = -6000,
	E_PAGESALLOC_EXCEED_LIMIT,
	E_FS_EVENT_TRIGGER                  = -7000,
	E_SYS_CLOCK_CHANGED                 = -8000,

	E_DIRCETORY_CHANGED                 = -9000,
	E_NETPORT_OPEND                     = -9001,
	E_EXTERNAL_NET_CONNECTED            = -9002,
	E_PPP_PORT_OPENED                   = -9003,
	E_EXTERNAL_PPP_CONNECTED            = -9004,
	E_NET_LOGIN_ON                      = -9005,
	E_SERIAL_LOGIN_ON                   = -9006,
	E_NET_LOGIN_EXIT                    = -9007,             
    E_SERIAL_LOGIN_EXIT                 = -9008,
}MonitorErrorCode;

enum{REPORT_ID_ADD,	   REPORT_ID_SET,    REPORT_ID_GET};


static inline int __System(const char *cmdstring, char *out, unsigned int size)
{
	if (NULL == out)
		return ERROR;

	FILE *fp = popen(cmdstring, "r");
	if (NULL == fp)
		return ERROR - 1;
	else{
		char buf[1024]={0};
		unsigned int sz = 0;
		while(fgets(buf, sizeof(buf), fp) != NULL){
			sz += snprintf(out + sz, size - sz, "%s", buf);
			if(size <= sz + 1){
				pclose(fp);
				return ERROR - 2;
			}
		}
	}
	pclose(fp);
	return OK;
}




/**
* error code form monitor processing
*/
typedef enum{
	E_MONITOR_PROCESS_OK       = 0,
	E_MONITOR_PROCESS_ERR      = -1001,
	E_MONITOR_PROCESS_DISABLE,
	E_MONITOR_VM_ERR,
	E_MONITOR_PM_ERR,
	E_MONITOR_DATASECTION_ERR,
	E_MONITOR_EXE_CODE_ERR,
	E_MONITOR_PTHREAD_ERR,
	E_MONITOR_MEM_USED_ERR,
	E_MONITOR_SHM_USED_ERR,
	E_MONITOR_LOAD1_ERR,
	E_MONITOR_LOAD5_ERR,
	E_MONITOR_LOAD10_ERR,
	E_MONITOR_CPU_ERR,
	E_MONITOR_MSGQUE_ERR,
	E_MONITOR_SEMPV_ERR,
	E_MONITOR_HANDLE_ERR,
	E_MONITOR_RESOURCE_ERR,
	E_MONITOR_THREAD_ERR,
	//
	E_MONITOR_SYSRES_ERR       = -2001,
	E_MONITOR_SYSRES_DISABLE ,
}ExitMonitorCode;

typedef struct{
	char                process_name[MAX_TITLE_SIZE];
	char                check_rst[MAX_RESULT_SIZE];
}stProcessReport;


typedef struct{
	char                sub_title[MAX_TITLE_SIZE];
	
    unsigned int        uiMemTotal;
	unsigned int        uiMemFree;
	unsigned int        uiMemShar;
	unsigned int        uiBuffer;
	unsigned int        uiCached;
	unsigned int        uiMemUsed;
	unsigned int        uiFlashUsed;
}stSysResReport; 

typedef struct {
	char                sub_title[MAX_TITLE_SIZE];
	
	unsigned int        uiCPUStat;
	float               load_avg1;
    float               load_avg2;
	float               load_avg3;	
}stCpuInfo;


typedef struct{
	char                sub_title[MAX_TITLE_SIZE];
    unsigned int        uiClockChangeIdx;
	unsigned char       prev_time[LEN_TIME + 1];
    unsigned char       next_time[LEN_TIME + 1];
}stClkChangeReport;

typedef struct {
    char                sub_title[MAX_TITLE_SIZE];
    unsigned char       ucEth0Stat;                //0: down 1:up
	unsigned char       ucEth1Stat;                //0: dwon 1:up
}stNetStatReport;

typedef struct {
	char         sub_title[MAX_TITLE_SIZE];
	unsigned int uiUsedArrays;
    unsigned int uiAllocatSemas;
    unsigned int uiMaxArrays;
	unsigned int uiMaxSemas;
}stSemaStatReport;


typedef struct {	
	char         sub_title[MAX_TITLE_SIZE];
	unsigned int uiUsedMsgNum;
	unsigned int uiUsedSpace;
	unsigned int uiAllocatQueues;
	unsigned int uiMaxMsgSize;
	unsigned int uiMaxQueSize;
    unsigned int uiMaxQueues;
}stMsgqueStatReport;


typedef struct {	
	char         sub_title[MAX_TITLE_SIZE];
	unsigned int    uiAllocatSegment;
	unsigned int    uiMaxSegment;
	unsigned long   ulAllocatPages;
	unsigned long   ulResidentPages;
	unsigned long   ulMaxPages;	
}stShmStatReport;

typedef struct{
	char            sub_title[MAX_TITLE_SIZE];
    int             uiEventNum;
	char            event[64];
	char            event_name[32];
}stFilsSystemReport;

typedef struct{
    char            sub_title[MAX_TITLE_SIZE];  
    char            BlackProcess;
}stBlackProcesReport;

typedef struct{
    char            sub_title[MAX_TITLE_SIZE];  
	unsigned char   ucPortOpenedNum;
	//unsigned char   szPort[MAX_PORT_NUM][2];
	unsigned char   ucNetConnectedNum;
	//unsigned char   szBlackNet[MAX_NET_NUM][6];
}stNetEventReport;

typedef struct{
    char            sub_title[MAX_TITLE_SIZE];  
    char            sLoginSucc;
	char            sLoginFail;
	char            sLoginExit;
    char            rLoginSucc;
	char            rLoginFail;
	char            rLoginExit;
}stLoginEventReport;


typedef struct {
    unsigned char ucflag;       //error exist 1 ,error not exist 0
    unsigned int  uiErrCode;
    unsigned char enMsg[48 + 1];
	unsigned char chMsg[48 + 1];
	unsigned char ucrpt;        //reported 1, not report 0
}stErrMessage;


typedef struct ST_MONITOR_REPORT stMonitorReport;

struct ST_MONITOR_REPORT{
	char  			    title[MAX_TITLE_SIZE * 2]; 
	char                procss_title[MAX_TITLE_SIZE];
	stProcessReport     stprocessrpt[MAX_PROCESS_NUM];
	stSysResReport      stsysresrpt;
	stCpuInfo           stcpuinfo;
	stClkChangeReport   stclkchangerpt;
    stNetStatReport     stnetstatrpt;
	int                 inandbadblocknum;
	stSemaStatReport    stsemastatrpt;
	stMsgqueStatReport  stmsgquerpt;
	stShmStatReport     stshmstatrpt; 
	stFilsSystemReport  stfsrpt;
};

typedef struct {
	int		flag;
	char	name[60];
} stConfItem;


typedef struct {
    int    process_num;
	char   white_process[MAX_WHITE_PROCESS_NUM][LEN_PROCESS_NAME];
}stProcessWhiteList;

typedef struct {
    unsigned char           szRemoteIp[MAX_IP_LEN ];
    unsigned char           szRemotePort[MAX_PORT_LEN ];
}__attribute_packed stNetWhiteList;


typedef struct {
    unsigned char           szPort[2 + 1];
    //unsigned char           szBlackNet[6 + 1];
    //unsigned char         szRef[10];
}stNetPortRecord;


typedef struct {
    //unsigned char           szPort[2 + 1];
    unsigned char           szBlackNet[6 + 1];	
    //unsigned char         szRef[10];
}stNetIpRecord;



typedef struct {
    unsigned char  ucUsbAuthFlag;                    // 0:usb临时授权关闭，1:临时授权开启
    unsigned char  ucSSHAuthFlag;                    // 0:ssh临时授权关闭，1:临时授权开启
    unsigned char  ucDirNum;                         //关键文件目录个数
    unsigned char  szFileDir[MAX_DIR_NUM][LEN_PATH]; //关键文件目录内容
    unsigned char  ucNetNum;                         //以太网外联白名单个数
    stNetWhiteList stNetList[MAX_WHITE_NET_NUM];     //以太网外联参数内容
} __attribute_packed stSecurityParam;

typedef struct _my_msgbuf 
{
  long int msgtype;
  unsigned char  pbuf[M_MSG_LEN];
} my_msgbuf;


typedef struct {
    unsigned char  szFlag[2];                          
	unsigned char  szNetPort[2];
	unsigned char  szNetInfo[6];
	unsigned char  szPPPPort[2];
	unsigned char  szPPPInfo[6];
    unsigned char  ucUsb;
	unsigned char  ucRs232;
	unsigned char  szKeyDirectory[33];
	unsigned char  ucNetLoginOn;
	unsigned char  ucSerialLoginOn;
	unsigned char  ucDangerCmd;
}stEventDetails;

typedef struct 
{
	unsigned int PREDEF/*.= 0XFEFEFEFE*/;     // "前导符"  此消息是由第三方发出的兼容格式的事件	
	unsigned int meterpoint; 	           //事件测量点号	---------------------------------增加4个字节，为了保持与CSG/NSG 的SSpotData_t结构体的长度一致
    unsigned int EventTime;               //事件发生时间
    unsigned short id;                    //异常数据标识，根据此标志选择内部ID和内部格式转换方法
    unsigned char EventStatus;            //事件状态，发生恢复
    unsigned char bufLen;                 //因MAX_SPOT_DATA_LEN = 250 ，一字节可以表示事件长度
    unsigned char buf[MAX_SPOT_DATA_LEN]; //事件体
} __attribute_packed SSpotData_Out;

extern stEventDetails gstEventDetails;


typedef struct ST_MONITOR_ITEM stMonitorItem;

struct ST_MONITOR_ITEM{
	/**
	* the content name which in monitor module
	*/
	char                *name;
	/**
	* the user configure ,pending,need to check it later
	*/
	void                *userconf;
	/**
	* start monitor thread
	*/
	ExitMonitorCode     (*start_monitor)(stMonitorItem * pstMonitorItem);
	/**
	* stop monitor thread 
	*/
	ExitMonitorCode     (*stop_monitor)(stMonitorItem * pstMonitorItem);
};



typedef struct{
	float    load_avg1;
    float    load_avg2;
	float    load_avg3;
}stSysLoadAvg;


typedef struct{
	unsigned int user;      
	unsigned int system;   
	unsigned int nice;
	unsigned int idle;
	unsigned int iowait;
	unsigned int hardirq;
	unsigned int softirq;
	unsigned int st;
}stCPUStat;

void sync_process_report(stMonitorReport *pstmonitorrpt);
void sync_sysres_data(stMonitorReport *pstmonitorrpt);
void sync_cpu_info(stMonitorReport *pstmonitorrpt);
void sync_clock_record(stMonitorReport *pstmonitorrpt);
void sync_nand_block_bad_info(stMonitorReport *pstmonitorrpt);
void sync_sema_data(stMonitorReport *pstmonitorrpt);
void sync_shrm_data(stMonitorReport *pstmonitorrpt);
void sync_msgqueue_data(stMonitorReport *pstmonitorrpt);
void sync_netstat_record(stMonitorReport *pstmonitorrpt);
void sync_fs_event(stMonitorReport *pstmonitorrpt);
void set_err_exist(unsigned int uiErr);
void monitor_report(void);
bool check_error_need_report(void);
void get_sys_time(void *pTime);
void set_fs_fd(int fd, int * wd);
void set_socket_id(int id);
int read_white_process_list(void);
void send_event_to_application(void);
void reset_event_log(void);
void get_security_param(stSecurityParam * pstparam);
void sync_dir_changed_event(stEventDetails * pstevent);

/**
* send dangerous command operations event 
*/
#define SEND_DANGEROUS_CMD_EVENT \
	set_bit(gstEventDetails.ucDangerCmd,0);\
	set_bit(gstEventDetails.szFlag[0],1);\
	send_event_to_application();\
	reset_event_log();

/**
* send file system which application defined changed event
*/
#define SEND_FILESYSTEN_EVENT \
	send_event_to_application();\
	set_bit(gstEventDetails.szFlag[1],6);\
	reset_event_log();
/**
* send net which are not in whitelist connected event 
*/
#define SEND_NETEST_EVENT \
	send_event_to_application();\
	set_bit(gstEventDetails.szFlag[1],1);\
	reset_event_log();

/**
* send net port opened event.
*/
#define SEND_NETPORT_EVENT(exp) \
	memcpy(gstEventDetails.szNetPort,exp,MAX_PORT_LEN);\
	set_bit(gstEventDetails.szFlag[1],0); \
	send_event_to_application();\
	reset_event_log();

/**
* send usb detected event
*/
#define SEND_USB_PLUG_EVENT \
	send_event_to_application();\
	set_bit(gstEventDetails.szFlag[1],4); \
	reset_event_log();

/**
* serial port login on  
*/
#define SEND_SERIAL_LOGON_EVENT \
	set_bit(gstEventDetails.ucSerialLoginOn,0);\
	set_bit(gstEventDetails.szFlag[0],0); \
	send_event_to_application();\
	reset_event_log();

/**
* net  login on 
*/
#define SEND_NET_LOGON_EVENT \
	set_bit(gstEventDetails.ucNetLoginOn,0);\
	set_bit(gstEventDetails.szFlag[1],7); \
	send_event_to_application();\
	reset_event_log();

/**
* serial port login exit 
*/
#define SEND_SERIAL_LOG_EXIT_EVENT \
	set_bit(gstEventDetails.ucSerialLoginOn,2);\
	set_bit(gstEventDetails.szFlag[0],0); \
	send_event_to_application();\
	reset_event_log();

/**
* net login exit 
*/
#define SEND_NET_LOG_EXIT_EVENT \
	set_bit(gstEventDetails.ucNetLoginOn,2);\
	set_bit(gstEventDetails.szFlag[1],7); \
	send_event_to_application();\
	reset_event_log();

/**
* ssh login failed 
*/
#define SEND_SSH_LOG_FAILED_EVENT \
	set_bit(gstEventDetails.ucNetLoginOn,1);\
	set_bit(gstEventDetails.szFlag[1],7); \
	send_event_to_application();\
	reset_event_log();


/**
* register monitor content object to code segment
*/

#define RegisterMonitorItem(modulename, arg, priv)	\
	static stMonitorItem Object_##modulename     \
	__attribute_used __attribute_section__(.module.rodata) = \
	{\
		.name               = #modulename,                \
		.userconf           = priv,                    \
		.start_monitor		= modulename ##_start_monitor,\
		.stop_monitor		= modulename ##_stop_monitor, \
	};



#endif

