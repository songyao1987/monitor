#ifndef _PARAM_H_
#define _PARAM_H_



#define    MONITOR_PARAM_FILE       "/data1/conf/monitorparam.txt"


#define CHK_PARAM_OPTION(ext,option) do{ \
	return (ext[option >> 8] & (option & 0xff)); \
	}while(0)


#define  STR_PARAMS(A)              (unsigned char *)A,	    sizeof(A)
#define  CHAR_PARAMS(A) 		    (unsigned char *)&A,	sizeof(char)
#define  INT_PARAMS(A)	            (unsigned char *)&A,	sizeof(int)
#define  LONG_PARAMS(A)	            (unsigned char *)&A,	sizeof(int)

#define  STRING_ITEM				0x00
#define  ASSCII_ITEM 				0x01	
#define  HEX_ITEM					0x02	
#define  BIN_ITEM					0x03
#define  BCD_ITEM					0x04


#define  PARAM_ENABLE		        '1'
#define  PARAM_DISABLE	            '0'

typedef struct{
	unsigned char     name[40];
	void              *ini_ptr;
	unsigned char     len;
	unsigned char     type;
}stParamConf;



typedef struct{	
	unsigned int      uiTimeInterval;         //(Second),exec monitor if time out
	
    unsigned char     ucCheckProcess;		  //only enable can check the following processes	
    unsigned char     ucMenu;				  //check menu process? 	
	unsigned char     ucTaskmanage;           //check taskmanager process?
	unsigned char     ucControl;              //check control process?
	unsigned char     ucHarmonic;             //check harmonic process?
	unsigned char     ucEventRecord;          //check eventrecord process?
	unsigned char     ucManage6513;           //check manage6513 process?
	unsigned char     ucAlarm;                //check alarm process?
	unsigned char     ucEventGrade;           //check eventgrade process?
	unsigned char     ucReadmeter;            //check readmeter process?
	unsigned char     ucAnalysis;             //check analysis process?
	unsigned char     ucProparse;             //check proparse process?
	unsigned char     ucDevmon;               //check devmon process?
	unsigned char     ucVoicetask;            //check voicetask process?   
	unsigned char     ucPwmanage;             //check pwmanage process?
	unsigned char     ucRelayTask;            //check realytask process?
	unsigned char     ucMeasureGather;        //check measuregather process?
	unsigned char     ucExptProc;             //check exptproc process?
	unsigned char     ucWatchGprs;            //check watchgprs process?

	unsigned int      uiMemUsedLimit;         //param configure limit
	unsigned int      uiFlashUsedLimit;       //param configure limit
	unsigned int      uiCPUStatLimit;         //param configure limit
	unsigned int      uiLoadAvgLimit;         //param configure limit
	unsigned int      uiBadblkLimit;          //nand bad block num limit
	unsigned int      uiClockChangeLimit;     //unit (second)

	unsigned char     ucCheckSysRes;          //only enable can check the following items
	unsigned char     ucChkMemUsed;           //only enable can check the sub items
	unsigned char     ucChkMemTotal;          //check memory total?
	unsigned char     ucChkMemFree;           //check memmory free?
	unsigned char     ucChkMemShare;          //check share memory?
	unsigned char     ucChkBuffer;            //check buffer?
	unsigned char     ucChkCached;            //check cached?
	unsigned char     ucChkFlashUsed;         //check used flash?
	unsigned char     ucChkCPUStat;           //check cpu usage rate?
	unsigned char     ucChkLoadAvg;           //check system load average?	
    unsigned char     ucChkNandBad;           //check nand flash bad block?
	unsigned char     ucChkClockRecord;       //check clock change record?

    unsigned char     ucChkMsgQue;            //check message queue?
	unsigned char     ucChkSema;              //check sema ?
	unsigned char     ucChkShrm;              //check shar memory
    unsigned char     ucChkFileSystem;        //check terminal file system?
    unsigned char     ucChkBlackProcess;      //check process for black list
	unsigned char     ucChkNetStat;           //check net stat?
	unsigned char     ucChkLoginEvent;        //check serial/telnet/ssh login event?	
	unsigned char     ucChkUsb;               //check usb or not?
	unsigned char     ucChkDangerCmd;         //check dangerous command or not?
	//updating now......
}stMonitorParam;

extern    stMonitorParam     gstMonitorParam;


typedef struct{
	const char *name;
	unsigned char en;
	unsigned char stat;
	unsigned int  uiErr;
}stProcessMap;

extern stProcessMap stEnProcessMap[];


enum{STAT_UNKNOW, STAT_OK, STAT_ERR};

#define __SET_PROCESS_MAP_STAT(idx,st) do { \
	stEnProcessMap[idx].stat = st ;         \
	}while(0)

#define __GET_PROCESS_ERR(idx) (stEnProcessMap[idx].uiErr)

#define __MONITOR_TIME_INT     (gstMonitorParam.uiTimeInterval)
#define __CLOCK_CHANGE_LIMIT   (gstMonitorParam.uiClockChangeLimit)


#define __IF_CHECK_PROCESS     (gstMonitorParam.ucCheckProcess == PARAM_ENABLE)
#define __IF_CHECK_SYSRES      (gstMonitorParam.ucCheckSysRes  == PARAM_ENABLE)
#define __IF_CHECK_MEMSHARE    (gstMonitorParam.ucChkMemShare  == PARAM_ENABLE)
#define __IF_CHECK_BUFFER      (gstMonitorParam.ucChkBuffer    == PARAM_ENABLE)
#define __IF_CHECK_CACHED      (gstMonitorParam.ucChkCached    == PARAM_ENABLE)
#define __IF_CHECK_CPU         (gstMonitorParam.ucChkCPUStat   == PARAM_ENABLE)
#define __IF_CHECK_LOADAVG     (gstMonitorParam.ucChkLoadAvg   == PARAM_ENABLE)
#define __IF_CHECK_MEMUSED     (gstMonitorParam.ucChkMemUsed   == PARAM_ENABLE)
#define __IF_CHECK_FLASHUSED   (gstMonitorParam.ucChkFlashUsed == PARAM_ENABLE)		
#define __IF_CHECK_MEMTOTAL    (gstMonitorParam.ucChkMemTotal  == PARAM_ENABLE)
#define __IF_CHECK_MEMFREE     (gstMonitorParam.ucChkMemFree   == PARAM_ENABLE)
#define __IF_CHECK_NAND        (gstMonitorParam.ucChkNandBad   == PARAM_ENABLE)
#define __IF_CHECK_CLOCK_RECORD (gstMonitorParam.ucChkClockRecord == PARAM_ENABLE)
#define __IF_CHECK_MSGQUE      (gstMonitorParam.ucChkMsgQue    == PARAM_ENABLE)
#define __IF_CHECK_SEMAPHORE   (gstMonitorParam.ucChkSema      == PARAM_ENABLE) 
#define __IF_CHECK_SHARM       (gstMonitorParam.ucChkShrm      == PARAM_ENABLE) 
#define __IF_CHECK_FS          (gstMonitorParam.ucChkFileSystem== PARAM_ENABLE)
#define __IF_CHECK_NET_STAT    (gstMonitorParam.ucChkNetStat   == PARAM_ENABLE)
#define __IF_CHECK_BP          (gstMonitorParam.ucChkBlackProcess == PARAM_ENABLE)
#define __IF_CHECK_LOGINEVENT  (gstMonitorParam.ucChkLoginEvent== PARAM_ENABLE)
#define __IF_CHECK_USB         (gstMonitorParam.ucChkUsb == PARAM_ENABLE)
#define __IF_CHECK_DANGERCMD   (gstMonitorParam.ucChkDangerCmd == PARAM_ENABLE)



int read_param_file(void);
int sync_process_enable_map(const char *process, unsigned char *pIdx);
int file_read(int fd, void *buf, size_t count);
int file_open(const char *filename, int oflag, ...);
int file_seek(int fd, off_t offset, int whence);
int file_write(int fd, const void *buf, size_t count);
int file_close(int fd);
int file_size(const char *filename);
bool file_exist(const char *filename);

#endif 

