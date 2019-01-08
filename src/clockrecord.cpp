
#include "monitor.hpp"


typedef struct {
	pthread_t	      tid;
	stClkChangeReport stclkchgrpt;
} stClockRecord;

static stClockRecord stclkprivdata = {
	.tid	= 0,
	.stclkchgrpt	    = {"*clock record*",0},
};


void get_sys_time(void *pTime)
{
	#define bin2bcd(x)	((((x) / 10) << 4) | ((x) % 10))
	time_t		        t;
	struct tm	        ltm;
	unsigned char 		*str = (typeof(str))pTime;
	int			        year, month, day, hour, minute, second;

	time(&t);
	localtime_r(&t, &ltm);
	year   = ltm.tm_year + 1900 - 2000;
	month  = ltm.tm_mon + 1;
	day    = ltm.tm_mday;
	hour   = ltm.tm_hour;
	minute = ltm.tm_min;
	second = ltm.tm_sec;
	
	str[0] = bin2bcd(year);
	str[1] = bin2bcd(month);
	str[2] = bin2bcd(day);
	str[3] = bin2bcd(hour);
	str[4] = bin2bcd(minute);
	str[5] = bin2bcd(second);

	#undef bin2bcd
}



void * verify_clock_record_thread_ex(void *arg)
{
	struct timeval    tn,ti,ts,tx,td;
    unsigned char     buf[32] = {0};
	unsigned char     temp[10] = {0};
	
	__ASSERT(NULL != arg);
	printf("verify_clock_record_thread_ex \n");
	stClkChangeReport *pstclkrecord = (stClkChangeReport *)arg;
    gettimeofday(&tn, NULL); 
	/**
	* get the time(s) which user configured for clock change record
	*/
	if (__CLOCK_CHANGE_LIMIT < 10) {
        ts.tv_sec = 10;
	}else {
        ts.tv_sec = __CLOCK_CHANGE_LIMIT;
	}
	ts.tv_usec = 0;
	timeradd(&tn, &ts,&tx);
	memset(buf,0,sizeof(buf));
    while (1) {
		gettimeofday(&tn,NULL);
		if (timercmp(&tn, &tx, >=)
			|| timercmp(&tn, &td, <=)) {
			get_sys_time(buf);
			printf("clock changed!!!\n");
			//printf("prev time:20%02x/%02x/%02x %02x:%02x:%02x\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);			
			//printf("next time:20%02x/%02x/%02x %02x:%02x:%02x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
			pstclkrecord->uiClockChangeIdx++;			
			memcpy(pstclkrecord->prev_time,temp,LEN_TIME);
			memcpy(pstclkrecord->next_time,buf,LEN_TIME);
			set_err_exist(E_SYS_CLOCK_CHANGED);
			break;
		} else {
            timeradd(&tn, &ts,&tx);
			timersub(&tn, &ts,&td);
			get_sys_time(temp);
		}
		usleep(400 * 1000);
	}
	return NULL;
}


static ExitMonitorCode 
	clock_record_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stClockRecord  *pClkRecord = (stClockRecord  *)pstMonitorItem->userconf;
	if (0 != pClkRecord->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pClkRecord->tid);
		pthread_join(pClkRecord->tid,NULL);
		pClkRecord->tid = 0;
	}
	return rc;
}



static ExitMonitorCode
	clock_record_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

    __ASSERT(NULL != pstMonitorItem);
	stClockRecord  *pClkRecord = (stClockRecord  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_CLOCK_RECORD){
		if (pthread_create(&pClkRecord->tid,NULL,verify_clock_record_thread_ex, \
			&(pClkRecord->stclkchgrpt)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}

void sync_clock_record(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stclkchangerpt,&(stclkprivdata.stclkchgrpt),sizeof(stClkChangeReport));
}


//RegisterMonitorItem(clock_record,0,&stclkprivdata)


