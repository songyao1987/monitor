#include "monitor.hpp"

typedef struct {
	pthread_t	   tid;
	unsigned char  ucFlag;
} stSSHLoginChk;

static stSSHLoginChk stsshlogincheck = {
	.tid	            = 0,
	.ucFlag	            = 0x00,
};

static bool check_ssh_auth_flag(void)
{
    stSecurityParam stparam;
	memset(&stparam, 0, sizeof(stSecurityParam));	
	get_security_param(&stparam);
    if (0 == stparam.ucSSHAuthFlag) {
        return true;
	}
	return false;
}


static void * verify_ssh_login_failed(void *arg)
{
    #define    SSH_LOGIN_FILE    "/mnt/log/sys_messages"	
    int fd = file_open(SSH_LOGIN_FILE,O_RDONLY);
	if (fd < 0) {
        //TRACE("open error(%d)\n",errno);
		return NULL;
	}
    char buf[1024];
	int  read_bytes;
	memset(buf,0,sizeof(buf));
	int size = file_size(SSH_LOGIN_FILE);
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
	if (NULL != strstr(buf,"login rejected")) {
		if (check_ssh_auth_flag()) {
			TRACE("got ssh login failed event!!!!!!!!\n");
			SEND_SSH_LOG_FAILED_EVENT;
		}
	}
	remove(SSH_LOGIN_FILE);
    return NULL;
	#undef    SSH_LOGIN_FILE
}

static ExitMonitorCode 
	ssh_login_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stSSHLoginChk         * pstloginchk   = (stSSHLoginChk  *)pstMonitorItem->userconf;

    __ASSERT(NULL != pstMonitorItem);
	if (0 != pstloginchk->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pstloginchk->tid);
		pthread_join(pstloginchk->tid,NULL);
		pstloginchk->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	ssh_login_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stSSHLoginChk *pstloginchk = (stSSHLoginChk  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_DANGERCMD){
		pstloginchk->ucFlag = 0x00;
		if (pthread_create(&pstloginchk->tid,NULL,verify_ssh_login_failed,&(pstloginchk->ucFlag)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}



RegisterMonitorItem(ssh_login,0,&stsshlogincheck)

