
#include "monitor.hpp"

typedef struct {
	pthread_t	       tid;
	stSemaStatReport   stsemarpt;
} stSemaCheck;

static stSemaCheck stsemaprivdata = {
	.tid	= 0,
	.stsemarpt	    = {"*Sema status*",0},
};

static void check_sema_stat(void *arg)
{
    char buf[32];

	__ASSERT(NULL != arg);
	stSemaStatReport *stsemarpt = (stSemaStatReport *)arg;
    /**
    * used arrays
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -su | grep 'used arrays' | awk '{print $4}'", \
    buf, sizeof(buf)) == OK);
	stsemarpt->uiUsedArrays = strtoul(buf, NULL, 0);
    /**
    * allocated semaphores
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -su | grep 'allocated semaphores' | awk '{print $4}'", \
    buf, sizeof(buf)) == OK);
    stsemarpt->uiAllocatSemas = strtoul(buf, NULL, 0);
    /**
    * max arrays
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -sl | grep 'max number of arrays' | awk '{print $6}'", \
    buf, sizeof(buf)) == OK);
	stsemarpt->uiMaxArrays = strtoul(buf, NULL, 0);
    /**
    * max semaphores
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -sl | grep 'max semaphores system wide' | awk '{print $6}'", \
    buf, sizeof(buf)) == OK);
    stsemarpt->uiMaxSemas = strtoul(buf, NULL, 0);
}

static ExitMonitorCode 
	sema_check_stop_monitor(stMonitorItem * pstMonitorItem)
{
	return E_MONITOR_PROCESS_OK;
}

static ExitMonitorCode 
	sema_check_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	if (__IF_CHECK_SEMAPHORE) {
		stSemaCheck *pstsemachk = (stSemaCheck *)pstMonitorItem->userconf;
		check_sema_stat(&(pstsemachk->stsemarpt));
		usleep(200*1000);
	}
	return rc;
}

void sync_sema_data(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stsemastatrpt,&(stsemaprivdata.stsemarpt),sizeof(stSemaStatReport));
}


//RegisterMonitorItem(sema_check,0,&stsemaprivdata)

