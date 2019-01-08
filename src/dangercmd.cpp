
#include "monitor.hpp"

typedef struct {
	pthread_t	   tid;
	unsigned char  ucFlag;
} stDangerCmdCheck;

static stDangerCmdCheck stdangercmdcheck = {
	.tid	            = 0,
	.ucFlag	            = 0x00,
};


static void * verify_dangerous_cmd(void *arg)
{
    #define    HISTORY_ROOT_PATH    "/home/root/.ash_history"
	#define    HISTORY_MAIN_PATH    "/home/et1000/.ash_history"

    char * filename = NULL;
    if (file_exist(HISTORY_ROOT_PATH))
		filename = HISTORY_ROOT_PATH;
	else if (file_exist(HISTORY_MAIN_PATH))
		filename = HISTORY_MAIN_PATH;
	else 
		filename = HISTORY_ROOT_PATH;	
    int fd = file_open(filename,O_RDONLY);
	if (fd < 0) {
        //TRACE("open error(%d)\n",errno);
		return NULL;
	}
    char buf[1024];
	int  read_bytes;
	memset(buf,0,sizeof(buf));
	int size = file_size(filename);
	if (size > sizeof(buf)) {
        read_bytes = sizeof(buf);
	} else {
        read_bytes = size;
	}
	int rd = file_read(fd,buf,read_bytes);
	file_close(fd);
	if (rd != read_bytes) {
        TRACE("read error(%d)\n",errno);
		return NULL;
	}
	if (NULL != strstr(buf,"rm")
		|| NULL != strstr(buf,"mv")
		|| NULL != strstr(buf,"cp")
		|| NULL != strstr(buf,"kill")
		|| NULL != strstr(buf,"pkill")) {
		
		TRACE("got dangerous command event!!!!!!!!\n");
		SEND_DANGEROUS_CMD_EVENT;
	}
	remove(filename);
    return NULL;
	#undef    HISTORY_ROOT_PATH
	#undef    HISTORY_MAIN_PATH
}

static ExitMonitorCode 
	danger_command_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stDangerCmdCheck * pstcmdchk   = (stDangerCmdCheck  *)pstMonitorItem->userconf;

    __ASSERT(NULL != pstMonitorItem);
	if (0 != pstcmdchk->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pstcmdchk->tid);
		pthread_join(pstcmdchk->tid,NULL);
		pstcmdchk->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	danger_command_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stDangerCmdCheck *pstcmdchk = (stDangerCmdCheck  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_DANGERCMD){
		pstcmdchk->ucFlag = 0x00;
		if (pthread_create(&pstcmdchk->tid,NULL,verify_dangerous_cmd,&(pstcmdchk->ucFlag)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}



RegisterMonitorItem(danger_command,0,&stdangercmdcheck)

