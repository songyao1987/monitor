#include "monitor.hpp"

typedef struct {
	pthread_t	       tid;
	stShmStatReport    stmshmstatrpt;
} stShmStatCheck;

static stShmStatCheck stshmprivdata = {
	.tid	= 0,
	.stmshmstatrpt	    = {"*Shrm status*",0},
};

static void check_shrm_stat(void *arg)
{
    char buf[32];

	__ASSERT(NULL != arg);
	stShmStatReport *stshmstatrpt = (stShmStatReport *)arg;
    /**
    * allocated segments
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -mu | grep 'segments allocated' | awk '{print $3}'", \
    buf, sizeof(buf)) == OK);
	stshmstatrpt->uiAllocatSegment = strtoul(buf, NULL, 0);
    /**
    * allocated pages
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -mu | grep 'pages allocated' | awk '{print $3}'", \
    buf, sizeof(buf)) == OK);
    stshmstatrpt->ulAllocatPages = strtoul(buf, NULL, 0);
    /**
    * pages resident
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -mu | grep 'pages resident' | awk '{print $3}'", \
    buf, sizeof(buf)) == OK);
	stshmstatrpt->ulResidentPages = strtoul(buf, NULL, 0);
    /**
    * max segments
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -ml | grep 'max number of segments' | awk '{print $6}'", \
    buf, sizeof(buf)) == OK);
    stshmstatrpt->uiMaxSegment = strtoul(buf, NULL, 0);
    /**
    * max pages
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -ml | grep 'max total shared' | awk '{print $7}'", \
    buf, sizeof(buf)) == OK);
    stshmstatrpt->ulMaxPages = strtoul(buf, NULL, 0);
}

static ExitMonitorCode 
	shrm_check_stop_monitor(stMonitorItem * pstMonitorItem)
{
	return E_MONITOR_PROCESS_OK;
}

static ExitMonitorCode 
	shrm_check_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	if (__IF_CHECK_SHARM) {
		stShmStatCheck *pstshrchk = (stShmStatCheck *)pstMonitorItem->userconf;
		check_shrm_stat(&(pstshrchk->stmshmstatrpt));
		usleep(200*1000);
	}
	return rc;
}

void sync_shrm_data(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stshmstatrpt,&(stshmprivdata.stmshmstatrpt),sizeof(stShmStatReport));
}


//RegisterMonitorItem(shrm_check,0,&stshmprivdata)

