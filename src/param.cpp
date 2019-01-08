#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "param.hpp"
#include "monitor.hpp"

static int         read_cnt;
stMonitorParam     gstMonitorParam;

const static stParamConf stParam[] = {	
	   {"time interval", 	   INT_PARAMS(gstMonitorParam.uiTimeInterval),		BIN_ITEM},
	
	   {"check process",	   CHAR_PARAMS(gstMonitorParam.ucCheckProcess),		ASSCII_ITEM},
	   {"check menu", 	       CHAR_PARAMS(gstMonitorParam.ucMenu),   			ASSCII_ITEM},
	   {"check taskmanage",	   CHAR_PARAMS(gstMonitorParam.ucTaskmanage),   	ASSCII_ITEM},
	   {"check control",	   CHAR_PARAMS(gstMonitorParam.ucControl),   		ASSCII_ITEM},
	   {"check hamonic",	   CHAR_PARAMS(gstMonitorParam.ucHarmonic),   		ASSCII_ITEM},
	   {"check eventgrade",	   CHAR_PARAMS(gstMonitorParam.ucEventGrade),   	ASSCII_ITEM},
	   {"check eventrecord",   CHAR_PARAMS(gstMonitorParam.ucEventRecord),  	ASSCII_ITEM},
	   {"check manage6513",	   CHAR_PARAMS(gstMonitorParam.ucManage6513),   	ASSCII_ITEM},
	   {"check alarm",	       CHAR_PARAMS(gstMonitorParam.ucAlarm),   			ASSCII_ITEM},
	   {"check readmeter",	   CHAR_PARAMS(gstMonitorParam.ucReadmeter),    	ASSCII_ITEM},
	   {"check analysis",	   CHAR_PARAMS(gstMonitorParam.ucAnalysis),     	ASSCII_ITEM},
	   {"check proparse",	   CHAR_PARAMS(gstMonitorParam.ucProparse),     	ASSCII_ITEM},
	   {"check devmon",	   	   CHAR_PARAMS(gstMonitorParam.ucDevmon),       	ASSCII_ITEM},	   
	   {"check voicetask",	   CHAR_PARAMS(gstMonitorParam.ucVoicetask),    	ASSCII_ITEM},
	   {"check pwmanage",	   CHAR_PARAMS(gstMonitorParam.ucPwmanage),     	ASSCII_ITEM},
	   {"check RelayTask",	   CHAR_PARAMS(gstMonitorParam.ucRelayTask),    	ASSCII_ITEM},   
	   {"check MeasureGather", CHAR_PARAMS(gstMonitorParam.ucMeasureGather),	ASSCII_ITEM},
	   {"check exptproc",	   CHAR_PARAMS(gstMonitorParam.ucExptProc),     	ASSCII_ITEM},   
	   {"check watchgprs",     CHAR_PARAMS(gstMonitorParam.ucWatchGprs),    	ASSCII_ITEM},

	   {"memory used limit",   INT_PARAMS(gstMonitorParam.uiMemUsedLimit),	    BIN_ITEM},
	   {"flash used limit",    INT_PARAMS(gstMonitorParam.uiFlashUsedLimit),    BIN_ITEM},
   	   {"cpu rate limit",      INT_PARAMS(gstMonitorParam.uiCPUStatLimit),	    BIN_ITEM},
	   {"loadavg limit",       INT_PARAMS(gstMonitorParam.uiLoadAvgLimit),      BIN_ITEM},	   
	   {"badblock limit",      INT_PARAMS(gstMonitorParam.uiBadblkLimit),       BIN_ITEM},
	   {"check sysRes",        CHAR_PARAMS(gstMonitorParam.ucCheckSysRes),  	ASSCII_ITEM},	   
	   {"check memtotal",      CHAR_PARAMS(gstMonitorParam.ucChkMemTotal),  	ASSCII_ITEM},
	   {"check memfree", 	   CHAR_PARAMS(gstMonitorParam.ucChkMemFree),		ASSCII_ITEM},
	   {"check shrmem", 	   CHAR_PARAMS(gstMonitorParam.ucChkMemShare),		ASSCII_ITEM},
	   {"check buffer", 	   CHAR_PARAMS(gstMonitorParam.ucChkBuffer),		ASSCII_ITEM},
       {"check cached", 	   CHAR_PARAMS(gstMonitorParam.ucChkCached),		ASSCII_ITEM},
	   {"check cpu", 	       CHAR_PARAMS(gstMonitorParam.ucChkCPUStat),		ASSCII_ITEM},
	   {"check loadavg", 	   CHAR_PARAMS(gstMonitorParam.ucChkLoadAvg),		ASSCII_ITEM},
	   {"check memused", 	   CHAR_PARAMS(gstMonitorParam.ucChkMemUsed),		ASSCII_ITEM},
       {"check flashused", 	   CHAR_PARAMS(gstMonitorParam.ucChkFlashUsed),		ASSCII_ITEM},
	   {"check badblock",	   CHAR_PARAMS(gstMonitorParam.ucChkNandBad),   	ASSCII_ITEM},
	   {"check clkrecord",	   CHAR_PARAMS(gstMonitorParam.ucChkClockRecord),	ASSCII_ITEM},
	   {"check msgqueue",	   CHAR_PARAMS(gstMonitorParam.ucChkMsgQue), 		ASSCII_ITEM},
	   {"check semaphore",	   CHAR_PARAMS(gstMonitorParam.ucChkSema),			ASSCII_ITEM},
	   {"check shrm",	       CHAR_PARAMS(gstMonitorParam.ucChkShrm),			ASSCII_ITEM},
	   {"check netstat",	   CHAR_PARAMS(gstMonitorParam.ucChkNetStat),		ASSCII_ITEM},
	   {"check filesystem",	   CHAR_PARAMS(gstMonitorParam.ucChkFileSystem),	ASSCII_ITEM},	   
	   {"check blackprocess",  CHAR_PARAMS(gstMonitorParam.ucChkBlackProcess),	ASSCII_ITEM},	   
	   {"check loginevent",    CHAR_PARAMS(gstMonitorParam.ucChkLoginEvent),	ASSCII_ITEM},	   
	   {"check usb plug",      CHAR_PARAMS(gstMonitorParam.ucChkUsb),	        ASSCII_ITEM},   
	   {"check dangercmd",     CHAR_PARAMS(gstMonitorParam.ucChkDangerCmd),	    ASSCII_ITEM},
};

static void init_default_param(void)
{
    gstMonitorParam.uiTimeInterval       = 60;
	gstMonitorParam.ucMenu               = PARAM_ENABLE;
	gstMonitorParam.ucAlarm              = PARAM_ENABLE;
	gstMonitorParam.ucAnalysis           = PARAM_ENABLE;
	gstMonitorParam.ucControl            = PARAM_ENABLE;
	gstMonitorParam.ucDevmon             = PARAM_ENABLE;
	gstMonitorParam.ucEventGrade         = PARAM_ENABLE;
	gstMonitorParam.ucEventRecord        = PARAM_ENABLE;
	gstMonitorParam.ucExptProc           = PARAM_ENABLE;
	gstMonitorParam.ucHarmonic           = PARAM_ENABLE;
	gstMonitorParam.ucManage6513         = PARAM_ENABLE;
	gstMonitorParam.ucMeasureGather      = PARAM_ENABLE;
	gstMonitorParam.ucProparse           = PARAM_ENABLE;
	gstMonitorParam.ucPwmanage           = PARAM_ENABLE;
	gstMonitorParam.ucReadmeter          = PARAM_ENABLE;
	gstMonitorParam.ucRelayTask          = PARAM_ENABLE;
	gstMonitorParam.ucTaskmanage         = PARAM_ENABLE;
	gstMonitorParam.ucVoicetask          = PARAM_ENABLE;
	gstMonitorParam.ucWatchGprs          = PARAM_ENABLE;

	gstMonitorParam.uiMemUsedLimit       = 80;
	gstMonitorParam.uiFlashUsedLimit     = 60;
	gstMonitorParam.uiCPUStatLimit       = 80;
	gstMonitorParam.uiLoadAvgLimit       = 3;
	gstMonitorParam.uiBadblkLimit        = 6;
	gstMonitorParam.uiClockChangeLimit   = 10 ;
    gstMonitorParam.ucCheckSysRes        = PARAM_DISABLE;
	gstMonitorParam.ucChkMemTotal        = PARAM_DISABLE;
	gstMonitorParam.ucChkMemFree         = PARAM_DISABLE;
	gstMonitorParam.ucChkMemShare        = PARAM_DISABLE;
	gstMonitorParam.ucChkBuffer          = PARAM_DISABLE;
	gstMonitorParam.ucChkCached          = PARAM_DISABLE;
    gstMonitorParam.ucChkCPUStat         = PARAM_DISABLE;
	gstMonitorParam.ucChkLoadAvg         = PARAM_DISABLE;
	gstMonitorParam.ucChkMemUsed         = PARAM_DISABLE;
	gstMonitorParam.ucChkFlashUsed       = PARAM_DISABLE;	
    gstMonitorParam.ucChkNandBad         = PARAM_DISABLE;
	gstMonitorParam.ucChkClockRecord     = PARAM_DISABLE;	
	gstMonitorParam.ucChkMsgQue          = PARAM_DISABLE;
	gstMonitorParam.ucChkSema            = PARAM_DISABLE;
	gstMonitorParam.ucChkShrm            = PARAM_DISABLE;
	gstMonitorParam.ucChkFileSystem      = PARAM_ENABLE;
	gstMonitorParam.ucCheckProcess       = PARAM_ENABLE;
	gstMonitorParam.ucChkBlackProcess    = PARAM_ENABLE;
	gstMonitorParam.ucChkLoginEvent      = PARAM_ENABLE;	
	gstMonitorParam.ucChkNetStat         = PARAM_ENABLE;
	gstMonitorParam.ucChkUsb             = PARAM_ENABLE;
	gstMonitorParam.ucChkDangerCmd       = PARAM_ENABLE;
}


stProcessMap stEnProcessMap[] = {
	{"menu",		 gstMonitorParam.ucMenu,        STAT_UNKNOW, E_MENU_STAT_ERROR},
	{"taskmanage",	 gstMonitorParam.ucTaskmanage,  STAT_UNKNOW, E_TASKMNG_STAT_ERROR},
	{"control", 	 gstMonitorParam.ucControl,     STAT_UNKNOW, E_CONTROL_STAT_ERROR},
	{"harmonic",	 gstMonitorParam.ucHarmonic,    STAT_UNKNOW, E_HOMONIC_STAT_ERROR},
	{"eventrecord",  gstMonitorParam.ucEventRecord, STAT_UNKNOW, E_EVENTRECORD_STAT_ERROR},
	{"manage6513",	 gstMonitorParam.ucManage6513,  STAT_UNKNOW, E_MNG513_STAT_ERROR},
	{"alarm",		 gstMonitorParam.ucAlarm,       STAT_UNKNOW, E_ALARM_STAT_ERROR},
	{"eventgrade",	 gstMonitorParam.ucEventGrade,  STAT_UNKNOW, E_EVENTGRADE_STAT_ERROR},
	{"readmeter",	 gstMonitorParam.ucReadmeter,   STAT_UNKNOW, E_READMETER_STAT_ERROR},
	{"analysis",	 gstMonitorParam.ucAnalysis,    STAT_UNKNOW, E_ANALYSIS_STAT_ERROR},

	{"proparse",	 gstMonitorParam.ucProparse,    STAT_UNKNOW, E_PROPARSE_STAT_ERROR},
	{"devmon",		 gstMonitorParam.ucDevmon,      STAT_UNKNOW, E_DEVMON_STAT_ERROR},
	{"voicetask",	 gstMonitorParam.ucVoicetask,   STAT_UNKNOW, E_VOICETASK_STAT_ERROR},
	{"pwmanage",	 gstMonitorParam.ucPwmanage,    STAT_UNKNOW, E_PWMNG_STAT_ERROR},
	{"RelayTask",	 gstMonitorParam.ucRelayTask,   STAT_UNKNOW, E_RELAYTASK_STAT_ERROR},
	{"MeasureGather",gstMonitorParam.ucMeasureGather,STAT_UNKNOW,E_MEASGATHER_STAT_ERROR},
	{"exptproc",	 gstMonitorParam.ucExptProc,    STAT_UNKNOW, E_EXPTPROC_STAT_ERROR},
	{"watchgprs",	 gstMonitorParam.ucWatchGprs,   STAT_UNKNOW, E_WATCHGPRS_STAT_ERROR},
};




static ssize_t safety_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

static ssize_t safety_full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;
	total = 0;

	while (len) {
		cc = safety_write(fd, buf, len);
		if (cc < 0) {
			if (total) {
				/* we already wrote some! */
				/* user can do another write to know the error code */
				return total;
			}
			return cc;	/* write() returns -1 on failure. */
		}
		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}
	return total;
}

static ssize_t safety_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

int file_open(const char *filename, int oflag, ...)
{
	int fd;

	if (oflag & O_CREAT) {
		int mode = 0644;
		fd = open(filename, oflag, mode);
	} else {
		fd = open(filename, oflag);
	}
	return fd;
}

int file_seek(int fd, off_t offset, int whence)
{
	return lseek(fd, offset, whence);
}

int file_close(int fd)
{
	return close(fd);
}

int file_read(int fd, void *buf, size_t count)
{
    return safety_read(fd, buf, count);
}

int file_write(int fd, const void *buf, size_t count)
{
	return safety_full_write(fd, buf, count);
}

int file_size(const char *filename)
{
	struct stat	stat;
	int			retval;

	retval = lstat(filename, &stat);
	if (! retval)
		return stat.st_size;

	return ERROR;
}

bool file_exist(const char *filename)
{
	return ((access(filename, F_OK) == 0) ? true : false);
}


static int read_line(int fd, void *vptr, int maxlen)
{
	int n;
	unsigned char c, *ptr;
	static unsigned char *read_ptr, read_buf[400];

	ptr = (typeof(ptr))vptr;
	for(n=1; n<maxlen; n++)
	{
		if( read_cnt<=0 )
		{
			if( (read_cnt=safety_read(fd, read_buf, sizeof(read_buf)))<0 )
			{
				return -1;
			}
			else if( read_cnt==0 )
			{
				return 0;
			}
			read_ptr = read_buf;
		}
		read_cnt--;
		c = *read_ptr++;

		*ptr++ = c;
		if( c==0x0a )
		{
			break;
		}
	}
	*ptr = 0x00;
	return (n);
}

static void AllTrim(unsigned char *str)
{
	int len, count;
	unsigned char *p_str;

	len = strlen((char *)str);
	if (0 == len) return;

	p_str = str;
	p_str += len-1;
	while( (*p_str==' ')||(*p_str==0x09)||(*p_str==0x0d)||(*p_str==0x0a) )
	{
		*p_str = 0x00;
		if(p_str == str)
		{
			break;
		}
		else
		{
			--p_str;
		}
	}
	p_str = str;
	len = strlen((char *)str);
	count = 0;
	while( (*p_str==' ')||(*p_str==0x09)||(*p_str==0x0d)||(*p_str==0x0a) )
	{
		if( p_str<(str+len) )
		{
			p_str++;
			count++;
		}
		else
		{
			break;
		}
	}
	if( count>0 )
	{
		memmove(str, p_str, len-count);
		memset(str+(len-count), 0x00, count);
	}
}

static int next_param(int fd, unsigned char *name, unsigned char *value)
{
	unsigned char aline[200+1];
	unsigned char *ptr;
	int           ret;

	memset(aline, 0x00, sizeof(aline));
	ret = read_line(fd, aline, sizeof(aline)-1);
	if( ret<=0 )
	{
		return -1;
	}
	if( aline[0]=='&' )
	{
		return -1;
	}

	ptr = (unsigned char *)memchr(aline, '#', strlen((char *)aline));
	if ( ptr!=NULL )
	{
		if( aline[0]=='#')
    	{
   			*ptr = 0x00;
    	}
	}

	AllTrim(aline);
	ptr = (unsigned char *)memchr(aline, '=', strlen((char *)aline));
	if( ptr==NULL )
	{
		return 1;
	}

	memcpy(name, aline, ptr-aline);
	strcpy((char *)value, (char *)(ptr+1));
	AllTrim(name);
	AllTrim(value);
	return 0;
}

static void str_convert(unsigned char *dest, unsigned char *srcStr, unsigned char len)
{
	memset(dest, 0x00, len);
	if( strlen((char *)srcStr)<len )
	{
		strcpy((char *)dest, (char *)srcStr);
	}
	else
	{
		memcpy((char *)dest, (char *)srcStr, len-1);
	}
}

static void ascii_convert(unsigned char *dest, unsigned char *srcStr,unsigned char len)
{
	memset(dest, 0x00, len);
	memcpy(dest, srcStr, len);
}

static void bin_convert(unsigned char *dest, unsigned char *srcStr, unsigned char len)
{
	unsigned char  achar;
	unsigned short  i;
	int  l;

	switch(len)
	{
		case sizeof(achar):
			achar = (unsigned char)atoi((char *)srcStr);
			*dest = achar;
			break;
		case sizeof(i):
			i = atoi((char *)srcStr);
			memcpy(dest, (unsigned char *)&i, len);
			break;
		case sizeof(l):
			l = atol((char *)srcStr);
			memcpy(dest, (unsigned char *)&l, len);
			break;
		default:
			break;
	}
}



static int get_param_file(const char *filename, stParamConf *paramconf)
{
	int          i;
	stParamConf *q;
	unsigned char name[80], value[80];

	if (NULL == filename || NULL == paramconf){
		return -1;
	}

	int fd = file_open(filename, O_RDWR);
	if (fd < 0){
		return -2;
	}
	
	read_cnt = 0;
	for (; ;){
		memset(name, 0x00, sizeof(name));
		memset(value, 0x00, sizeof(value));
		int j = next_param(fd, name, value);
		if (j < 0){
			break;
		}
		if (j > 0){
			continue;
		}
		if (value[0] == 0x0a){
			continue;
		}
		for (i = 0, q = (stParamConf *)stParam; ; i++, q++){
			if (q->name[0] == '*'){
				break;
			}
			if (strcmp((char *)name, (char *)q->name)){
				continue;
			}
			switch(q->type){
				case STRING_ITEM:
					str_convert((unsigned char *)q->ini_ptr, value, q->len);
					break;
				case ASSCII_ITEM:
					ascii_convert((unsigned char *)q->ini_ptr, value, q->len);
					break;
    			case BIN_ITEM:
					bin_convert((unsigned char *)q->ini_ptr, value, q->len);
				break;
				default:
					break;
			}
			break;
		}
	}
	file_close(fd);
	return 0;
}

int sync_process_enable_map(const char *process, unsigned char *pIdx)
{
	for (int i = 0; i < ARRAY_SIZE(stEnProcessMap); i++){
		if (NULL != strstr((char *)process,stEnProcessMap[i].name)){
			*pIdx = i;
			return stEnProcessMap[i].en;
		}
	}
	return PARAM_DISABLE;	
}

void sync_process_report(stMonitorReport *pstmonitorrpt)
{    
	for (int i = 0; i < ARRAY_SIZE(stEnProcessMap); i++){
	    strcpy(pstmonitorrpt->stprocessrpt[i].process_name,stEnProcessMap[i].name);	
		snprintf(pstmonitorrpt->stprocessrpt[i].check_rst,MAX_RESULT_SIZE, "%s",
			((stEnProcessMap[i].stat == STAT_OK) ? "OK" : ((stEnProcessMap[i].stat == STAT_ERR) \
			? "ERROR" : "UNKNOW")));
	}
}


int read_param_file(void)
{
	int iRet;

	iRet = get_param_file(MONITOR_PARAM_FILE,(stParamConf *)stParam);
	if (iRet != 0) {
		init_default_param();
	}
	return OK;
}


