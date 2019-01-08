#include "monitor.hpp"
#include <arpa/inet.h>  
#include <netinet/in.h> 
#include <linux/types.h>  
#include <linux/netlink.h>   


extern stMonitorItem __monitor_start[], __monitor_end[];

static ExitMonitorCode start_monitor(void)
{
    ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	
	for (stMonitorItem *pstMonitorItem = __monitor_start; \
		pstMonitorItem < __monitor_end; \
		pstMonitorItem++) {
		rc = pstMonitorItem->start_monitor(pstMonitorItem);
	}
	return rc;
}

static ExitMonitorCode stop_monitor(void)
{
	ExitMonitorCode 	  rc = E_MONITOR_PROCESS_OK;
	for (stMonitorItem *pstMonitorItem = __monitor_start; \
		pstMonitorItem < __monitor_end; \
		pstMonitorItem++) {
		rc = pstMonitorItem->stop_monitor(pstMonitorItem);
	}
	return rc;
}


static stErrMessage sterrmsg[] = {
    {NONE, E_MENU_STAT_ERROR,    	 	"menu process error",		"",NOTREPORT},
	{NONE, E_TASKMNG_STAT_ERROR, 	 	"taskmanage process error",	"",NOTREPORT},
	{NONE, E_CONTROL_STAT_ERROR,	 	"control process error",	"",NOTREPORT},
	{NONE, E_HOMONIC_STAT_ERROR, 	 	"homonic process error",	"",NOTREPORT},
	{NONE, E_EVENTRECORD_STAT_ERROR,	"eventrecord process error","",NOTREPORT},
	{NONE, E_MNG513_STAT_ERROR,         "manage6513 process error",	"",NOTREPORT},
	{NONE, E_ALARM_STAT_ERROR,	        "alarm process error",		"",NOTREPORT},
	{NONE, E_EVENTGRADE_STAT_ERROR,     "eventgrade process error",	"",NOTREPORT},
	{NONE, E_READMETER_STAT_ERROR,	    "readmeter process error",	"",NOTREPORT},
	{NONE, E_ANALYSIS_STAT_ERROR,       "analysis process error",	"",NOTREPORT},
	{NONE, E_PROPARSE_STAT_ERROR,	    "proparse process error",	"",NOTREPORT},
	{NONE, E_DEVMON_STAT_ERROR,         "devmon process error",		"",NOTREPORT},
	{NONE, E_VOICETASK_STAT_ERROR,	    "voicetask process error",	"",NOTREPORT},
	{NONE, E_PWMNG_STAT_ERROR,          "pwmanage process error",	"",NOTREPORT},
	{NONE, E_RELAYTASK_STAT_ERROR,	    "relaytask process error",	"",NOTREPORT},
	{NONE, E_MEASGATHER_STAT_ERROR,     "measuregat process error", "",NOTREPORT},
	{NONE, E_EXPTPROC_STAT_ERROR,	    "exptproc process error",	"",NOTREPORT},
	{NONE, E_WATCHGPRS_STAT_ERROR,      "watchgprs process error",	"",NOTREPORT},

	{NONE, E_MEMUSED_EXCEED_LIMIT,      "memused exceed limit",		"",NOTREPORT},
	{NONE, E_FLASHUSED_EXCEED_LIMIT,	"flashused exceed limit",	"",NOTREPORT},
	{NONE, E_CPUUSAGE_EXCEED_LIMIT,     "CPUusage exceed limit", 	"",NOTREPORT},
	{NONE, E_LOADAVG_EXCEED_LIMIT,	    "loadavg exceed limit",		"",NOTREPORT},
	{NONE, E_BADFLASH_EXCEED_LIMIT,     "badflash exceed limit",	"",NOTREPORT},
	{NONE, E_FS_EVENT_TRIGGER,          "got filesystem event",		"",NOTREPORT},	
	{NONE, E_SYS_CLOCK_CHANGED,         "terminal time changed",	"",NOTREPORT},
    /**
    * the following events(error) just for terminal security requirements
	*/
    {NONE, E_DIRCETORY_CHANGED,         "key directory changed",    "",NOTREPORT},
    {NONE, E_NETPORT_OPEND,             "ethernet port opened",     "",NOTREPORT},   
    {NONE, E_EXTERNAL_NET_CONNECTED,    "external net connected",   "",NOTREPORT},    
    {NONE, E_PPP_PORT_OPENED,           "ppp0 port opened",         "",NOTREPORT},   
    {NONE, E_EXTERNAL_PPP_CONNECTED,    "external ppp connected",   "",NOTREPORT},  
    {NONE, E_NET_LOGIN_ON,              "ethernet login on",        "",NOTREPORT},   
    {NONE, E_SERIAL_LOGIN_ON,           "serial port login on",     "",NOTREPORT},
    {NONE, E_NET_LOGIN_EXIT,            "ethernet login exit",      "",NOTREPORT},
    {NONE, E_SERIAL_LOGIN_EXIT,         "serial port login exit",   "",NOTREPORT},
};

static int msg_id = 0;
stEventDetails gstEventDetails;


static int get_msg_id(void)
{
    return msg_id;
}
static void set_msg_id(int id)
{
    msg_id = id;
}


void set_err_exist(unsigned int uiErr)
{
    for (int i = 0; i < ARRAY_SIZE(sterrmsg); i++) {
        if (sterrmsg[i].uiErrCode == uiErr) {
            sterrmsg[i].ucflag = EXIST;
			break;
		}   
	}
}

bool check_error_need_report(void)
{
    for (int i = 0; i < ARRAY_SIZE(sterrmsg); i++) {
        if (sterrmsg[i].ucflag == EXIST
			&& sterrmsg[i].ucrpt == NOTREPORT) {
            return true;
    	}
	}
	return false;
}

static unsigned int get_timer_count(void)
{
	struct timeval	tn;

	gettimeofday(&tn, NULL);
	return (tn.tv_sec *1000) + (tn.tv_usec/1000);
}

static void sync_module_report(stMonitorReport * pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
	sync_process_report(pstmonitorrpt);
	sync_sysres_data(pstmonitorrpt);
	sync_cpu_info(pstmonitorrpt);
	sync_clock_record(pstmonitorrpt);
	sync_nand_block_bad_info(pstmonitorrpt);
	sync_netstat_record(pstmonitorrpt);
	sync_sema_data(pstmonitorrpt);
	sync_shrm_data(pstmonitorrpt);
	sync_msgqueue_data(pstmonitorrpt);
	sync_fs_event(pstmonitorrpt);
}

static void sync_module_event(stEventDetails * pstevent)
{
    __ASSERT(NULL != pstevent);
    sync_dir_changed_event(pstevent);
}

static int __add_report_num(int iCtrl)
{
    static int report_id = 0;
	if (iCtrl == REPORT_ID_ADD) {
        report_id++;
		if (report_id > MAX_REPORT_NUM)
			report_id = 0;
	} 
	return report_id;
}

static int get_cur_report_num(void)
{
    return __add_report_num(REPORT_ID_GET);
}

static void add_report_num(void)
{
    __add_report_num(REPORT_ID_ADD);
}


static void print_monitor_report(stMonitorReport *pstmonitorrpt)
{
    int  i;
	int  j;
	int  report_id;
	char buf[14];
	static const char *dimm = "===========================================================\n";

    report_id = get_cur_report_num();
	memset(buf,0,sizeof(buf));
    snprintf(buf,sizeof(buf),"%s_%d\n",REPORT_PATH,report_id);	
	buf[strlen(buf)] = '\0';
	if (access(buf,F_OK) == 0) 
		remove(buf);
	FILE *fp = fopen(buf,"a+");	
    memset(buf,0,sizeof(buf));
	get_sys_time(buf);
    fprintf(fp,"TIME:20%02x/%02x/%02x %02x:%02x:%02x\n",\
		buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	fprintf(fp,dimm);
	fprintf(fp,"%s\n",pstmonitorrpt->title);
	fprintf(fp,dimm);
	for (i = 0; i < ARRAY_SIZE(sterrmsg); i++) {
	    if (sterrmsg[i].ucflag == EXIST) {
            fprintf(fp,"Error code[%-5d]:   %s\n",sterrmsg[i].uiErrCode,sterrmsg[i].enMsg);
			sterrmsg[i].ucrpt = REPORTED;
		}  
    }
	fprintf(fp,dimm);
	fprintf(fp,"%s\n",pstmonitorrpt->procss_title);
    for (j = 0; j < MAX_PROCESS_NUM; j++) {
		if (strlen(pstmonitorrpt->stprocessrpt[j].process_name) <= 0)
			break;
        fprintf(fp,"[%-15.15s]                    [%s]\n", \
			pstmonitorrpt->stprocessrpt[j].process_name, \
			pstmonitorrpt->stprocessrpt[j].check_rst);  
	}
    fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stsysresrpt.sub_title);
	fprintf(fp,"\n[%-15.15s]                    [%d]\n",  "Memory total",\
		pstmonitorrpt->stsysresrpt.uiMemTotal);	
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Memory free",\
		pstmonitorrpt->stsysresrpt.uiMemFree);
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Memory share",\
		pstmonitorrpt->stsysresrpt.uiMemShar);	
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Buffer",\
		pstmonitorrpt->stsysresrpt.uiBuffer);
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Cached",\
		pstmonitorrpt->stsysresrpt.uiCached);
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Memory used",\
		pstmonitorrpt->stsysresrpt.uiMemUsed);	
	fprintf(fp,"[%-15.15s]                    [%d]\n",  "Flash used",\
		pstmonitorrpt->stsysresrpt.uiFlashUsed);
	fprintf(fp,dimm);	
	fprintf(fp,pstmonitorrpt->stcpuinfo.sub_title);
	fprintf(fp,"\n[%-15.15s]                    [%d]\n",  "CPU stat",\
		pstmonitorrpt->stcpuinfo.uiCPUStat);
	fprintf(fp,"[%-15.15s]                    [%.2f]\n","loadavg(1m)",\
		pstmonitorrpt->stcpuinfo.load_avg1);	
	fprintf(fp,"[%-15.15s]                    [%.2f]\n","loadavg(5m)",\
		pstmonitorrpt->stcpuinfo.load_avg2);
	fprintf(fp,"[%-15.15s]                    [%.2f]\n","loadavg(10m)",\
		pstmonitorrpt->stcpuinfo.load_avg3);
	fprintf(fp,dimm);
	fprintf(fp,"*Nand flash bad block*\n");
	fprintf(fp,"[%-15.15s]                    [%d]\n","Bad block", \
		pstmonitorrpt->inandbadblocknum);
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stnetstatrpt.sub_title);
	fprintf(fp,"\n[%-15.15s]					 [%s]\n",  "Eth0 dev", \
		(pstmonitorrpt->stnetstatrpt.ucEth0Stat == 1 ? "UP" : "DOWN"));
	fprintf(fp,"[%-15.15s]					 [%s]\n",  "Eth1 dev", \
		(pstmonitorrpt->stnetstatrpt.ucEth1Stat == 1 ? "UP" : "DOWN"));
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stmsgquerpt.sub_title);
    fprintf(fp,"\n[%-15.15s]                    [%d]\n", "UsedMsgNum", \
		pstmonitorrpt->stmsgquerpt.uiUsedMsgNum);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "UsedSpace", \
		pstmonitorrpt->stmsgquerpt.uiUsedSpace);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "AllocatedQues", \
		pstmonitorrpt->stmsgquerpt.uiAllocatQueues);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxQueues", \
		pstmonitorrpt->stmsgquerpt.uiMaxQueues);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxQueueSize", \
		pstmonitorrpt->stmsgquerpt.uiMaxQueSize);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxMsgSize", \
		pstmonitorrpt->stmsgquerpt.uiMaxMsgSize);
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stsemastatrpt.sub_title);
    fprintf(fp,"\n[%-15.15s]                    [%d]\n", "AllocatedSemas", \
		pstmonitorrpt->stsemastatrpt.uiAllocatSemas);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "UsedArrays", \
		pstmonitorrpt->stsemastatrpt.uiUsedArrays);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxSemas", \
		pstmonitorrpt->stsemastatrpt.uiMaxSemas);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxArrays", \
		pstmonitorrpt->stsemastatrpt.uiMaxArrays);
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stshmstatrpt.sub_title);
    fprintf(fp,"\n[%-15.15s]                    [%d]\n", "AllocatedSegm", \
		pstmonitorrpt->stshmstatrpt.uiAllocatSegment);
    fprintf(fp,"[%-15.15s]                    [%d]\n", "MaxSegm", \
		pstmonitorrpt->stshmstatrpt.uiMaxSegment);
    fprintf(fp,"[%-15.15s]                    [%ld]\n", "AllocatedPages", \
		pstmonitorrpt->stshmstatrpt.ulAllocatPages);
    fprintf(fp,"[%-15.15s]                    [%ld]\n", "ResidentPages", \
		pstmonitorrpt->stshmstatrpt.ulResidentPages);
    /*fprintf(fp,"[%-15.15s]                    [%ld]\n", "MaxPages", \
		pstmonitorrpt->stshmstatrpt.ulMaxPages);*/
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stclkchangerpt.sub_title);
	fprintf(fp,"\n[%-15.15s]                    [%d]\n",  "change times",\
		pstmonitorrpt->stclkchangerpt.uiClockChangeIdx);
	if (pstmonitorrpt->stclkchangerpt.uiClockChangeIdx > 0) {
		unsigned char buf[LEN_TIME + 1] = {0};
		memcpy(buf,pstmonitorrpt->stclkchangerpt.prev_time,LEN_TIME);
        fprintf(fp,"[%-15.15s]:                   [20%02x/%02x/%02x %02x:%02x:%02x]\n","prev time", \
			buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
		memset(buf,0,sizeof(buf));
		memcpy(buf,pstmonitorrpt->stclkchangerpt.next_time,LEN_TIME);
        fprintf(fp,"[%-15.15s]:                   [20%02x/%02x/%02x %02x:%02x:%02x]\n","next time", \
			buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	}
	fprintf(fp,dimm);
	fprintf(fp,pstmonitorrpt->stfsrpt.sub_title);
	fprintf(fp,"\n[%-15.15s]                    [%d]\n",  "CurEventCount",\
		pstmonitorrpt->stfsrpt.uiEventNum);
	if (pstmonitorrpt->stfsrpt.uiEventNum > 0) {
		fprintf(fp,"[%-15.15s]                    [%s]\n",  "CurEventName",\
			pstmonitorrpt->stfsrpt.event_name);
		fprintf(fp,"[%-15.15s]                    [%s]\n",  "CurEventDescrip",\
			pstmonitorrpt->stfsrpt.event);	
	}
	fprintf(fp,dimm);
	fflush(fp);
	fclose(fp);	
	add_report_num();
}

static stSecurityParam gstsecurityparam;



int read_security_param(void)
{
    memset(&gstsecurityparam, 0, sizeof(stSecurityParam));
    int fd = file_open(SECURITY_PARAM,O_RDWR);
	if (fd < 0) {
        TRACE("%s open error(%d)\n",SECURITY_PARAM,errno);
		return ERROR;
	}
	int ret = file_read(fd, &gstsecurityparam, sizeof(stSecurityParam));
	file_close(fd);
	if (ret != sizeof(stSecurityParam)) {
        TRACE("%s read error(%d)\n",SECURITY_PARAM,errno);
		return ERROR;
	}
	return OK;
}

void get_security_param(stSecurityParam * pstparam)
{
    memcpy(pstparam,&gstsecurityparam,sizeof(stSecurityParam));
}

static int init_message_queue(void)
{
	int msg_id = msgget(( key_t ) 0x30, 0660 | IPC_CREAT );
	if (msg_id < 0) {
        return ERROR;
	}
	set_msg_id(msg_id);
    return msg_id;		
}

static int send_message(int msg_id,const void * buf, int buf_len)
{
    __ASSERT(NULL != buf);

	if (msg_id < 0) {
        return ERROR; 
	}
	int res;
	res = msgsnd( msg_id, ( void * )buf, buf_len, IPC_NOWAIT);
	if (res < 0) {
        return ERROR;
	}
	return OK;
}


void send_event_to_application(void)
{
    SSpotData_Out stpotdata;
	memset(&stpotdata,0,sizeof(SSpotData_Out));
    stpotdata.PREDEF = 0XFEFEFEFE;

	int msg_id = get_msg_id();
	stpotdata.EventTime = time(NULL);
	stpotdata.id = 1;
	stpotdata.bufLen = 55;
	int len = 0;
	memcpy(&stpotdata.buf[len],gstEventDetails.szFlag,2);
	len += 2;	
	memcpy(&stpotdata.buf[len],gstEventDetails.szNetPort,2);
	len += 2;	
	memcpy(&stpotdata.buf[len],gstEventDetails.szNetInfo,6);
	len += 6;
	memcpy(&stpotdata.buf[len],gstEventDetails.szPPPPort,2);
	len += 2;	
	memcpy(&stpotdata.buf[len],gstEventDetails.szPPPInfo,6);
	len += 6;
	stpotdata.buf[len++] = gstEventDetails.ucUsb;	
	stpotdata.buf[len++] = gstEventDetails.ucRs232;
	char temp[33];
	memset(temp,'0',sizeof(temp));
	int dirlen = strlen((char *)gstEventDetails.szKeyDirectory);
	if (dirlen < 32) {
        memcpy(&temp[32 - dirlen],gstEventDetails.szKeyDirectory,dirlen);		
        memcpy(&stpotdata.buf[len],temp,32);
	}else {
        memcpy(&stpotdata.buf[len],gstEventDetails.szKeyDirectory,32);
	}
	stpotdata.buf[len++] = gstEventDetails.ucNetLoginOn;	
	stpotdata.buf[len++] = gstEventDetails.ucSerialLoginOn;
	stpotdata.buf[len++] = gstEventDetails.ucDangerCmd;

	my_msgbuf msgbuf;
	memset(&msgbuf, 0, sizeof(my_msgbuf));
	msgbuf.msgtype = 1;
	memcpy(msgbuf.pbuf,&stpotdata,sizeof(SSpotData_Out));	
	__VERIFY(send_message(msg_id,(void *)&msgbuf,sizeof(SSpotData_Out)) == OK);	
	TRACE("send message to application OK\n");
	usleep(200 * 1000);	
}

void reset_event_log(void)
{
	memset(&gstEventDetails,0,sizeof(stEventDetails));
}

void monitor_report(void)
{
    /**
    * stop monitor first
	*/
    __ASSERT(stop_monitor() == E_MONITOR_PROCESS_OK);
    /**
    * sync monitor data
	*/  
	stMonitorReport stmonitorrpt;
	memset(&stmonitorrpt,0,sizeof(stMonitorReport));
	strcpy(stmonitorrpt.title,"**********************CHECKING REPORT**********************");
	strcpy(stmonitorrpt.procss_title,"*Process status*");
	sync_module_report(&stmonitorrpt);
	/**
    * print monitor report
	*/
	print_monitor_report(&stmonitorrpt);
	/**
    * reset error code
	*/
	//reset_err_stat();
	__set_errno(OK);
}

static void init_report(void)
{
	for (int i = 0; i < MAX_REPORT_NUM; i++) {
        char buf[32] = {0};
        snprintf(buf,sizeof(buf),"%s%d",REPORT_PATH,i);
		if (access(buf,F_OK) == 0) 
			remove(buf);
	}
}


static int do_inotify_init(void)
{
    int   fd; 
    int   wd[MAX_DIR_NUM];

	/**
    * init inotify 
	*/
	fd	=  inotify_init();
	if (fd < 0) {
		TRACE("inotify_init error\n");
		return ERROR;
	}
	for (int i = 0; i < gstsecurityparam.ucDirNum; i++) {
        wd[i] = inotify_add_watch(fd, (char *)gstsecurityparam.szFileDir[i],INOTIFY_MASK);
		if (wd[i] < 0){
			TRACE("inotify_add_watch error\n");
			close(fd);
			return ERROR;
		}
	}
	set_fs_fd(fd,wd);
	return OK;
}

static int init_hotplug_sock(void)  
{  
	const int buffersize = 1024;  
	int ret;  
    struct sockaddr_nl snl;  
    bzero(&snl, sizeof(struct sockaddr_nl));  
    snl.nl_family = AF_NETLINK;  
    snl.nl_pid = getpid();  
    snl.nl_groups = 1;  

    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);  
    if (s == -1)   
    {  
        perror("socket");  
	    return -1;  
	}  
    setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));  
    ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));  
    if (ret < 0)   
    {  
        perror("bind");  
	    close(s);  
	    return -1;  
	}  
	set_socket_id(s);
    return s;  
}  

static void sig_chld(int signo)
{
    pid_t pid;
    int   stat;
    pid = wait(&stat);    
    return;
}

void sys_init(void)
{
    /**
    * read user parameter file 
    * program will set default value if this file is not exist
	*/
	read_param_file();
    /**
    * check report files
    * program will delete it if these files exist
	*/
	init_report();
	/**
    * read process white list file that application level wrote
	*/	
	//__VERIFY(read_white_process_list() == OK);
	/**
    * read security param file that application level wrote
	*/	
	__VERIFY(read_security_param() == OK);    
	/**
    * set monitor objects
	*/
	__VERIFY(do_inotify_init() == OK);
    /**
    * init message queue
	*/
    __VERIFY(init_message_queue() > 0); 
	/**
    * init usb listen event
	*/
	__VERIFY(init_hotplug_sock() > 0);

	/**
    * reset error code 
	*/
	__set_errno(OK);
    /**
    * proc special signal
    * because this monitor process used many popen
	*/
	//signal(SIGCHLD, SIG_IGN); 
	signal(SIGCHLD, &sig_chld);
}

/*
* this function wrote in application level
* just be an example for application developer
*/
int save_security_param(void)
{
    #if 0 //sy 20181221 15:23
    const static stSecurityParam stsecurityparam = {
        1, 
		1,
        4,
        {DATA1,DATA1CFG,HOME,HOMECFG},
        5,
        {{"\xC0\xA8\x01\x01","\xA6\x13"},     //192.168.1.1 5030
         {"\xC0\xA8\x01\x02","\xA7\x13"},     //192.168.1.2 5031
         {"\xC0\xA8\x01\x03","\xA8\x13"},     //192.168.1.3 5032
         {"\xC0\xA8\x01\x04","\xA9\x13"},     //192.168.1.4 5033
         {"\xC0\xA8\x01\x05","\xAA\x13"}},    //192.168.1.5 5034
	};
    int fd = file_open(SECURITY_PARAM,O_RDWR | O_CREAT);
	if (fd < 0) {
        TRACE("%s open error(%d)\n",SECURITY_PARAM,errno);
		return ERROR;
	}
	int ret = file_write(fd, &stsecurityparam, sizeof(stSecurityParam));
	file_close(fd);
	if (ret != sizeof(stSecurityParam)) {
        TRACE("%s write error(%d)\n",SECURITY_PARAM,errno);
		return ERROR;
	}
	return OK;
    #endif
}


int main(int argc, char *argv[])
{
	sys_init();
	while (1) {
		start_monitor();
	}
	return 0;
}

