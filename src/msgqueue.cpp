#include "monitor.hpp"

typedef struct {
	pthread_t	       tid;
	stMsgqueStatReport stmsgqueuerpt;
} stMsgqueueCheck;

static stMsgqueueCheck stmsgprivdata = {
	.tid	= 0,
	.stmsgqueuerpt	    = {"*Msgqueue status*",0},
};

static void check_msgqueue_stat(void *arg)
{
    char buf[32];

	__ASSERT(NULL != arg);
	stMsgqueStatReport *stmsgqueuerpt = (stMsgqueStatReport *)arg;
    /**
    * used message num
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -qu | grep 'used headers' | awk '{print $4}'", \
    buf, sizeof(buf)) == OK);
	stmsgqueuerpt->uiUsedMsgNum = strtoul(buf, NULL, 0);
    /**
    * allocated queues
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -qu | grep 'allocated queues' | awk '{print $4}'", \
    buf, sizeof(buf)) == OK);
    stmsgqueuerpt->uiAllocatQueues = strtoul(buf, NULL, 0);
    /**
    * used space
	*/
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -qu | grep 'used space' | awk '{print $4}'", \
    buf, sizeof(buf)) == OK);
	stmsgqueuerpt->uiUsedSpace = strtoul(buf, NULL, 0);
    /**
    * max queues
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -ql | grep 'max queues system wide' | awk '{print $6}'", \
    buf, sizeof(buf)) == OK);
    stmsgqueuerpt->uiMaxQueues= strtoul(buf, NULL, 0);
    /**
    * max message size of one single queue
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -ql | grep 'max size of message' | awk '{print $7}'", \
    buf, sizeof(buf)) == OK);
    stmsgqueuerpt->uiMaxMsgSize = strtoul(buf, NULL, 0);
    /**
    * max size of queue
	*/	
	memset(buf, 0, sizeof(buf));
    __VERIFY(__System("ipcs -ql | grep 'default max size of queue' | awk '{print $8}'", \
    buf, sizeof(buf)) == OK);
    stmsgqueuerpt->uiMaxQueSize = strtoul(buf, NULL, 0);
}

static ExitMonitorCode 
	msgqueue_check_stop_monitor(stMonitorItem * pstMonitorItem)
{
	return E_MONITOR_PROCESS_OK;
}

static ExitMonitorCode 
	msgqueue_check_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
    
	__ASSERT(NULL != pstMonitorItem);
	if (__IF_CHECK_MSGQUE) {
		stMsgqueueCheck *pstmsgchk = (stMsgqueueCheck *)pstMonitorItem->userconf;
		check_msgqueue_stat(&(pstmsgchk->stmsgqueuerpt));
		usleep(200*1000);
	}
	return rc;
}

void sync_msgqueue_data(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stmsgquerpt,&(stmsgprivdata.stmsgqueuerpt),sizeof(stMsgqueStatReport));
}


//RegisterMonitorItem(msgqueue_check,0,&stmsgprivdata)

