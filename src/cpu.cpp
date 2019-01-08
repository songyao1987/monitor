
#include "monitor.hpp"

typedef struct {
	pthread_t	   tid;
	stCpuInfo      stcpuinfo;
} stCpuCheck;

static stCpuCheck stcpuprivdata = {
	.tid	= 0,
	.stcpuinfo	    = {"*CPU usage*",0},
};

static stCPUStat total_stat[2];
static char      cpu_stat_swap = 0;

static int get_cpu_stat(stCPUStat *pstcpustat)
{
#define     FILE_CPU_STAT     "/proc/stat"
	int idx=0;
	__ASSERT(NULL != pstcpustat);
	int fd = file_open(FILE_CPU_STAT,O_RDONLY);
	if (fd < 0) {
		TRACE("open /proc/stat fail.\n");
		return ERROR;
	}
	char buf[200] = {0};
	int iLen = file_read(fd, buf, sizeof(buf) - 1);
	if(iLen <= 0){
		close(fd);
		TRACE("read /proc/stat fail.\n");
		return ERROR;
	}
	buf[iLen] = 0;
	close(fd);

	char *p = strtok(buf, " ");
	while(NULL != p){
		switch(idx){
			case 1:
				pstcpustat->user    = strtoul(p,NULL,10);
				break;
			case 2:
				pstcpustat->system  = strtoul(p,NULL,10);
				break;
			case 3:
				pstcpustat->nice    = strtoul(p,NULL,10);
				break;
			case 4:
				pstcpustat->idle    = strtoul(p,NULL,10);
				break;
			case 5:
				pstcpustat->iowait  = strtoul(p,NULL,10);
				break;
			case 6:
				pstcpustat->hardirq = strtoul(p,NULL,10);
				break;
			case 7:
				pstcpustat->softirq = strtoul(p,NULL,10);
				break;
		    default:
				break;
		}
		p = strtok(NULL, " ");
		idx++;
	}
	return OK;
#undef     FILE_CPU_STAT
}

static unsigned int get_cpu_use_statistics(const stCPUStat * pstcpustat)
{
    __ASSERT(NULL != pstcpustat);
	unsigned int cpu_use_stat = pstcpustat->user \
		+ pstcpustat->system \
		+ pstcpustat->idle   \
		+ pstcpustat->nice   \
		+ pstcpustat->iowait \
		+ pstcpustat->hardirq\
		+ pstcpustat->softirq\
		+ pstcpustat->st;
	return cpu_use_stat;
}

static int get_cpu_usage_rate(void)
{
	int iRet = get_cpu_stat(&total_stat[cpu_stat_swap % 2]);
	if (iRet != OK)
		return iRet;
	
	if (cpu_stat_swap) {
		unsigned int cpu_use_stat1 = get_cpu_use_statistics(&total_stat[(cpu_stat_swap - 1) % 2]);
		unsigned int cpu_use_stat2 = get_cpu_use_statistics(&total_stat[cpu_stat_swap % 2]);
		unsigned int cpu_diff	   = cpu_use_stat2 - cpu_use_stat1;
		unsigned int cpu_idle_diff = total_stat[cpu_stat_swap % 2].idle - total_stat[(cpu_stat_swap - 1) % 2].idle;
		unsigned int cpu_usage = 0;
		if (cpu_diff){
			cpu_usage = (int) (cpu_diff - cpu_idle_diff) * 100 / cpu_diff;
		}
		return cpu_usage;
	}
	cpu_stat_swap++;
	if (cpu_stat_swap > 1)
		cpu_stat_swap = 0;
	return ERROR;
}

static int get_sys_loadavg(stSysLoadAvg *pstSysLoadAvg)
{
#define   FILE_LOADAVG     "/proc/loadavg"
    __ASSERT(NULL != pstSysLoadAvg);
	int fd = file_open(FILE_LOADAVG,O_RDONLY);
	if (fd < 0)
		return ERROR;

	char buf[64] = {0};
	int iLen = file_read(fd,buf,sizeof(buf) - 1);
	if (iLen <= 0)
		return ERROR;

	buf[iLen] = 0;
	close(fd);
	char *p = strtok(buf, " ");
	int  idx = 0;
	while (NULL != p){
		switch (idx){
			case 0:
				pstSysLoadAvg->load_avg1 = atof(p);
				break;
		    case 1:
				pstSysLoadAvg->load_avg2 = atof(p);
				break;
		    case 2:
				pstSysLoadAvg->load_avg3 = atof(p);
				break;
			default:
				break;
		}
		if (idx == 3)
			break;
		p = strtok(NULL, " ");
		idx++;
	}
	return OK;
#undef    FILE_LOADAVG
}


void * verify_cpu_thread(void *arg)
{
    __ASSERT(NULL != arg);
    stCpuInfo * pstcpuinfo = (stCpuInfo * )arg;

     int iCpuUsageRate = get_cpu_usage_rate();
	 
     if (iCpuUsageRate > 0) {    
         pstcpuinfo->uiCPUStat = iCpuUsageRate;	
		 
		 if (iCpuUsageRate > gstMonitorParam.uiCPUStatLimit) {
			set_err_exist(E_CPUUSAGE_EXCEED_LIMIT);
		 }
 	 }
    /**
	* get system load average
	*/
	stSysLoadAvg stsysloadavg;

	memset(&stsysloadavg, 0, sizeof(stSysLoadAvg));
	if (__IF_CHECK_LOADAVG) {
		get_sys_loadavg(&stsysloadavg);
		pstcpuinfo->load_avg1 = stsysloadavg.load_avg1;
		pstcpuinfo->load_avg2 = stsysloadavg.load_avg2;
	    pstcpuinfo->load_avg3 = stsysloadavg.load_avg3;
	}	

	if (stsysloadavg.load_avg1 > gstMonitorParam.uiLoadAvgLimit
		|| stsysloadavg.load_avg2 > gstMonitorParam.uiLoadAvgLimit
		|| stsysloadavg.load_avg3 > gstMonitorParam.uiLoadAvgLimit) {
        set_err_exist(E_LOADAVG_EXCEED_LIMIT);
	}
	return NULL;
}

static ExitMonitorCode 
	cpu_info_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stCpuCheck * pstcpuchk   = (stCpuCheck  *)pstMonitorItem->userconf;

    __ASSERT(NULL != pstMonitorItem);
	if (0 != pstcpuchk->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pstcpuchk->tid);
		pthread_join(pstcpuchk->tid,NULL);
		pstcpuchk->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	cpu_info_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stCpuCheck *pstcpuchk = (stCpuCheck  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_CPU){
		if (pthread_create(&pstcpuchk->tid,NULL,verify_cpu_thread,&(pstcpuchk->stcpuinfo)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(800*1000);
	}
	return rc;
}

void sync_cpu_info(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stcpuinfo,&(stcpuprivdata.stcpuinfo),sizeof(stCpuInfo));
}


//RegisterMonitorItem(cpu_info,0,&stcpuprivdata)


