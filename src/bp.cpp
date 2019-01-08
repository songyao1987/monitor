#include "monitor.hpp"

#define PROCES   "/proc"
#define MAX_SIZE 256
#define INIT_PID 400

typedef struct {
	pthread_t	        tid;
	stBlackProcesReport stblkprocsrpt;
} stBlackListProcess;

static stBlackListProcess stblacklistprocess = {
	.tid	            = 0,
	.stblkprocsrpt	    = {"*black process*",0},
};


static stProcessWhiteList stproclist;



int save_white_process_list(void)
{
	const static stProcessWhiteList stwhitelist = {
        28,
        "watchdog",
		"RelayTask",
		"pwmanage",
		"voicetask",
		"devmon",
		"analysis",
		"readmeter",
		"eventgrade",
		"alarm",
		"eventrecord",
		"ct_test",
        "control",
        "MeasureGather",
        "exptproc",
        "watchgprs",
        "taskmanage",
        "proparse",
        "menu",
        "harmonic",
        "manage6513",
        "monitor",
        "sleep",
        "telnetd",
        "sh",
        "xinetd",
        "crond",
        "judge-console.s",
        "kworker",
	};	
    int fd = file_open(PROCESS_WHITE_LIST,O_RDWR | O_CREAT);
	if (fd < 0) {
        TRACE("%s open error\n",PROCESS_WHITE_LIST);
		return ERROR;
	}
	int ret = file_write(fd, &stwhitelist, sizeof(stProcessWhiteList));
	file_close(fd);
	if (ret != sizeof(stProcessWhiteList)) {
        TRACE("%s read error\n",PROCESS_WHITE_LIST);
		return ERROR;
	}
	return OK;
}


static bool is_digit(const void *p)
{
#define nodigit(c)	((c) < '0' || (c) > '9')
    __ASSERT(NULL != p);
	char * c = (typeof(c))p;
    int iLen = strlen(c);

	if (0 == iLen) {
        return false;
	}
    for (int i = 0; i < iLen; i++) {
        if (nodigit(c[i])) {
            return false;
		}
	}
	return true;
#undef nodigit
}

static int get_running_process_name(unsigned int pid,char *pName)
{
    int  ifd;
	int  ilen;
	char *p;
	char *q;
    char buf[MAX_SIZE * 10];
	char cmd[MAX_SIZE];
	
    __ASSERT(NULL != pName);
    snprintf(buf,sizeof(buf),"%d/stat",pid);
	ifd = file_open(buf, O_RDONLY);
	if (ifd < 0) {
        TRACE("open %s error(%d)\n",buf,errno);
		return ERROR;
	}
	memset(buf, 0, sizeof(buf));
	ilen = file_read(ifd,buf,sizeof(buf) - 1);
	file_close(ifd);
	if (ilen <= 0) {
        TRACE("read %s data error(%d)\n",buf,errno);
		return ERROR;
	}
	p = buf;
	p = strrchr(p, '(');
	q = strrchr(p, ')');
	ilen = q - p - 1;
	if (ilen > MAX_SIZE) {
        ilen = MAX_SIZE - 1;
	}
	memcpy(pName,p + 1, ilen);
	*(pName + ilen) = '\0';
	return OK;
}

int read_white_process_list(void)
{
    memset(&stproclist, 0, sizeof(stProcessWhiteList));
    int ifd = file_open(PROCESS_WHITE_LIST,O_RDWR);
	if (ifd < 0) {
        TRACE("%s open error\n",PROCESS_WHITE_LIST);
		return ERROR;
	}
	int iRet = file_read(ifd, &stproclist, sizeof(stProcessWhiteList));
	file_close(ifd);
	if (iRet != sizeof(stProcessWhiteList)) {
        TRACE("%s read error\n",PROCESS_WHITE_LIST);
		return ERROR;
	}
	return OK;
}

static bool is_process_black_list(const char * process)
{
    int len;
    int white_num;
    __ASSERT(NULL != process);

    white_num = stproclist.process_num;
	if (white_num > MAX_WHITE_PROCESS_NUM) {
        white_num = MAX_WHITE_PROCESS_NUM; 
	}
    for (int i = 0; i < white_num; i++) {
		len = strlen(stproclist.white_process[i]);
        if (strncmp(process,stproclist.white_process[i], \
			len < LEN_PROCESS_NAME ? len : LEN_PROCESS_NAME) == 0) {
			/**
            * this process is belongs to white list
			*/
            return false;
		}
	}
    return true;
}

static int check_process(const void *p)
{
    DIR  * dp;
	char * dir = PROCES;
    struct dirent * dirp;	
    char   process_name[MAX_SIZE] ;
    __ASSERT(NULL != p);
	stBlackProcesReport * pstbprpt = (stBlackProcesReport * )p;
    chdir(dir);
	if ((dp = opendir(dir)) == NULL) {
        TRACE("open proc dir failed\n");
        return ERROR;
	}
	while ((dirp = readdir(dp)) != NULL) {
        char buf[30] = {0};
		memset(process_name,0,sizeof(process_name));
        snprintf(buf,sizeof(buf),"%s",dirp->d_name);
		if (is_digit(buf)) {
			unsigned int ipid = atoi(dirp->d_name);
			if (ipid < INIT_PID) {
                continue;
			}
			__VERIFY(get_running_process_name(ipid,process_name) == OK);
			if (is_process_black_list(process_name)) {
                pstbprpt->BlackProcess = 1;
			}
		}
	}
	return OK;
}

void * verify_black_proces_activity(void *arg)
{
    while (1) {
		check_process(arg);		
	}
    return NULL;
}


static ExitMonitorCode 
	black_process_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stBlackListProcess  *pbs = (stBlackListProcess * )pstMonitorItem->userconf;
	if (0 != pbs->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pbs->tid);
		pthread_join(pbs->tid,NULL);
		pbs->tid = 0;
	}
	return rc;
}



static ExitMonitorCode
	black_process_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

    __ASSERT(NULL != pstMonitorItem);
	stBlackListProcess  *pbs = (stBlackListProcess * )pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_BP){
		if (pthread_create(&pbs->tid,NULL,verify_black_proces_activity, \
			&(pbs->stblkprocsrpt)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(1000*1000);
	}
	return rc;
}


//RegisterMonitorItem(black_process,0,&stblacklistprocess)

