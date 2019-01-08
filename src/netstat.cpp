#include "monitor.hpp"

typedef struct {
	pthread_t	    tid;
	stNetStatReport stnetstatreport;
} stNetStat;

static stNetStat stnetstat = {
	.tid	= 0,
	.stnetstatreport	    = {"*Eth0/Eth1 stat*", 0},
};



void * verify_netstat_thread(void * arg)
{
    __ASSERT(NULL != arg);
	stNetStatReport *pstnetstatreport = (stNetStatReport *)arg;

	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) 
		return NULL;
	
	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
		goto err_exit;
	
	if (ifr.ifr_flags & IFF_RUNNING) 
		pstnetstatreport->ucEth0Stat = 1;
	else
		pstnetstatreport->ucEth0Stat = 0;
	memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
	strcpy(ifr.ifr_name, "eth1");
	
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
		goto err_exit;
	
	if (ifr.ifr_flags & IFF_RUNNING) 
		pstnetstatreport->ucEth1Stat = 1;
	else
		pstnetstatreport->ucEth1Stat = 0;
err_exit:
	close(fd);
	return NULL;
}

static ExitMonitorCode 
	net_stat_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stNetStat  *pNetStat = (stNetStat *)pstMonitorItem->userconf;

	if (0 != pNetStat->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pNetStat->tid);
		pthread_join(pNetStat->tid,NULL);
		pNetStat->tid = 0;
	}
	return rc;
}

static ExitMonitorCode
	net_stat_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	stNetStat *pNetStat = (stNetStat *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);

	if (__IF_CHECK_NET_STAT){
		if (pthread_create(&pNetStat->tid,NULL,verify_netstat_thread, \
			&(pNetStat->stnetstatreport)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(200*1000);
	}
	return rc;
}

void sync_netstat_record(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stnetstatrpt,&(stnetstat.stnetstatreport),sizeof(stNetStatReport));
}



//RegisterMonitorItem(net_stat,0,&stnetstat)


