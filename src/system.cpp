#include "monitor.hpp"



typedef struct {
	pthread_t	   tid;
	stSysResReport stsysresrptxx;
} stSystemRes;

static stSystemRes stresprivdata = {
	.tid	= 0,
	.stsysresrptxx	    = {"*Memory/flash*",0},
};

static stSysResReport stsysresrpt;




static int get_flash_used(void)
{
	char buf[32] = {0};
	/**
	* flash used,also will delete percentage char 
	*/
	__VERIFY(__System("df / | tail -n 1 | awk '{print $5}'", buf, sizeof(buf)) == OK);
	char *p = strchr(buf, '%');
	if(p != NULL){
		*p = '\0';
	}
	return (strtoul(buf, NULL, 0));
}



static ExitMonitorCode 
	system_resource_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stSystemRes  *pSysRes = (stSystemRes  *)pstMonitorItem->userconf;

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

void * verify_sysres_thread(void *arg)
{
	int             iMemTotal = 0;
	int             iMemFree  = 0;
	int 			iShmem	  = 0 ;
	int 			iBuffers  = 0;
	int 			iCached   = 0;

	__ASSERT(NULL != arg);
	stSysResReport *pstsysres = (stSysResReport *)arg;
	if (__IF_CHECK_MEMUSED){
	    /**
		* get memory info
		*/
		char buf[32]  = {0};
		
		/**
		* Total physics memory space
		*/
		if (__IF_CHECK_MEMTOTAL){
			__VERIFY(__System("cat /proc/meminfo | awk '/MemTotal/ {print $2}'", \
				buf, sizeof(buf)) == OK);
			iMemTotal = strtoul(buf, NULL, 0);
			pstsysres->uiMemTotal = iMemTotal;
			//printf("Memory Total: 	%d\n",iMemTotal);			
			usleep(100 * 1000);
		}
		/**
		* Free memory space
		*/
		if (__IF_CHECK_MEMFREE){
			memset(buf, 0, sizeof(buf));
			__VERIFY(__System("cat /proc/meminfo | awk '/MemFree/ {print $2}'",  \
				buf, sizeof(buf)) == OK);
			iMemFree = strtoul(buf, NULL, 0);
			pstsysres->uiMemFree = iMemFree;
			//printf("Memory Free : 	%d\n",iMemFree);			
			usleep(100 * 1000);
		}
		/**
		* Share memory space for multiple processes
		*/
		if (__IF_CHECK_MEMSHARE){
			memset(buf, 0, sizeof(buf));
			__VERIFY(__System("cat /proc/meminfo | awk '/Shmem/ {print $2}'",  \
				buf, sizeof(buf)) == OK);
			iShmem = strtoul(buf, NULL, 0); 
			pstsysres->uiMemShar = iShmem;
			//printf("Memeory Share:	%d\n",iShmem);			
			usleep(100 * 1000);
		}
		/**
		* Buffers
		*/
		if (__IF_CHECK_BUFFER){
			memset(buf, 0, sizeof(buf));
			__VERIFY(__System("cat /proc/meminfo | awk '/Buffers/ {print $2}'", \
				buf, sizeof(buf)) == OK);
			iBuffers = strtoul(buf, NULL, 0);
			pstsysres->uiBuffer = iBuffers;
			//printf("Buffer	   :	%d\n",iBuffers);			
			usleep(100 * 1000);
		}
		/**
		* Cached
		*/
		if (__IF_CHECK_CACHED){
			memset(buf, 0, sizeof(buf));
			__VERIFY(__System("cat /proc/meminfo | awk '/Cached/ {print $2}'", \
				buf, sizeof(buf)) == OK);
			iCached = strtoul(buf, NULL, 0);
			pstsysres->uiCached = iCached;
			//printf("Cached	   :	%d\n",iCached);			
			usleep(100 * 1000);
		}		
		/**
		* get memory used
		*/
		pstsysres->uiMemUsed = ((iMemTotal - iMemFree - iBuffers - iCached)*100 / iMemTotal);

		if (pstsysres->uiMemUsed >= gstMonitorParam.uiMemUsedLimit)
			set_err_exist(E_MEMUSED_EXCEED_LIMIT);
	}
    /**
	* get flash info
	*/
	if (__IF_CHECK_FLASHUSED){
		int iFlashUsed = get_flash_used();
		pstsysres->uiFlashUsed = iFlashUsed;		
		if (iFlashUsed > gstMonitorParam.uiFlashUsedLimit)
			set_err_exist(E_FLASHUSED_EXCEED_LIMIT);
		usleep(100 * 1000);
	}	
	return NULL;
}

static ExitMonitorCode 
	system_resource_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	stSystemRes  *pSysRes = (stSystemRes  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);

	if (__IF_CHECK_SYSRES) {
		if (pthread_create(&pSysRes->tid,NULL,verify_sysres_thread,&(pSysRes->stsysresrptxx)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}

void sync_sysres_data(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stsysresrpt,&(stresprivdata.stsysresrptxx),sizeof(stSysResReport));
}


//RegisterMonitorItem(system_resource,0,&stresprivdata)







