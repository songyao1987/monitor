
#include "monitor.hpp"


typedef struct {
	unsigned char  en;
	pthread_t	   tid;
	stConfItem     cf;
} stProcessCheck;

static stProcessCheck stprivdata = {
	.en		= gstMonitorParam.ucCheckProcess,
	.tid	= 0,
	.cf	    = {0, "*Process status*"},
};

static bool checking_process(const void *pPathName)
{
	pid_t    pid ;
	char     cmd[64];
	char     *p = NULL;
	FILE     *pf = NULL;

	__ASSERT(NULL != pPathName);
	if (NULL == pPathName)
		goto err_exit;	
	
	p = strrchr((char *)pPathName, '/');
	if (NULL == p)
		p = (char *)pPathName;
	else
		p++;
	
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd,sizeof(cmd),"pidof %s",p);
	pf = popen(cmd,"r");
	if (NULL == pf)
		goto err_exit;
	
	pid = -1;
	fscanf(pf, "%d", &pid);
	pclose(pf);
	if (pid < 0)
		goto err_exit;
	
	if (0 != kill(pid,0))
		goto err_exit;
	
	return true;	
err_exit:
	return false;
}

static void * verify_process_thread(void *arg)
{
	stMonitorItem  *pstMonitorItem = (stMonitorItem *)arg;
	class ConfigFile configFile;

	configFile.Clear();
	configFile.Read("");	
	struct Program * pTemp;
	pTemp = configFile.FirstProg();
	unsigned char  c = 0;
	while (pTemp != 0){
		if (strlen(pTemp->path) > 0){			
			if (sync_process_enable_map(pTemp->path,&c) == PARAM_DISABLE){
				pTemp = configFile.NextProg();
				continue;
			}
			if (checking_process(pTemp->path)) {
               __SET_PROCESS_MAP_STAT(c,STAT_OK);
			}
			else {
			   __SET_PROCESS_MAP_STAT(c,STAT_ERR);	
			   set_err_exist(__GET_PROCESS_ERR(c));
			}
		}
		pTemp = configFile.NextProg();		
		usleep(300 * 1000);
	}
	return NULL;
}

static ExitMonitorCode 
	process_status_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stProcessCheck  *pSysRes = (stProcessCheck  *)pstMonitorItem->userconf;

	if (0 != pSysRes->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pSysRes->tid);
		pthread_join(pSysRes->tid,NULL);
		pSysRes->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	process_status_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stMonitorReport       stmonitorrpt;

	memset(&stmonitorrpt, 0, sizeof(stMonitorReport));
	stProcessCheck  *process_chk = (stProcessCheck  *)pstMonitorItem->userconf;
	
	pstMonitorItem->stop_monitor(pstMonitorItem);

	if (__IF_CHECK_PROCESS){
		if (pthread_create(&process_chk->tid,NULL,verify_process_thread,pstMonitorItem) != 0){
			rc = E_MONITOR_THREAD_ERR;
			//TRACE("errno = %d\n",errno);
		}
		usleep(1000*1000);
	}
//	monitor_process_stat(pstMonitorItem);
	return rc;
}



//RegisterMonitorItem(process_status,0,&stprivdata)





