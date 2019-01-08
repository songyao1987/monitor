#include "monitor.hpp"
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <linux/types.h>  
#include <linux/netlink.h>   


/**
* state : 0x0A means established, and 0x01 means LISTEN
*/
#define     ESTABLISHED      0x0A
#define     LISTEN           0x01


typedef struct {
	pthread_t	        tid;
	stNetEventReport    stneteventrpt;
} stNetEvent;

static stNetEvent stnetevent = {
	.tid	            = 0,
	.stneteventrpt	    = {"*net event*",0},
};

static unsigned char gNetPortIndex = 0;
static unsigned char gNetIpIndex = 0;
static unsigned char port_array[MAX_PORT_RECORD][2] ;
static unsigned char net_array[MAX_NET_RECORD][6] ;


typedef union iaddr iaddr;

union iaddr {    
	unsigned u;    
	unsigned char b[4];
}; 
union iaddr6 {    
	struct {        
		unsigned a;        
		unsigned b;        
		unsigned c;        
		unsigned d;    
	} u;    
	unsigned char b[16];
};

#define ADDR_LEN    (INET6_ADDRSTRLEN + 1 + 5 + 1) 

void _Api_ToolLong2Char(unsigned long ulSource, unsigned int uiTCnt, unsigned char *psTarget)
{
    unsigned int  i;
    for(i = 0; i < uiTCnt; i++){
        psTarget[i] = (char)(ulSource >> (8 * (uiTCnt - i - 1)));
    }
}

static void insert_port_record(const unsigned char * port)
{
    memcpy(port_array[gNetPortIndex],port,2);
	gNetPortIndex++;
	if (gNetPortIndex > MAX_PORT_RECORD) {
		gNetPortIndex = 0;
		memset(port_array,0,sizeof(port_array));
	}
}

static bool check_port_send_record_ex(const unsigned char * pbuf)
{
    for (int index = 0; index < ARRAY_SIZE(port_array); index++) {
		if (0 == memcmp(port_array[index],pbuf,2)) {
            return true;
		}
		if (index > gNetPortIndex)
			break;
	}	
	return false;
}

static void insert_net_record(const void * net)
{
    memcpy(net_array[gNetIpIndex],net,6);
	gNetIpIndex++;
	if (gNetIpIndex >= MAX_NET_RECORD) {
		gNetIpIndex = 0;
		memset(net_array,0,sizeof(net_array));
	}
}

static bool check_net_send_record_ex(const void * pbuf)
{
    for (int index = 0; index < ARRAY_SIZE(net_array); index++) {
		if (0 == memcmp(net_array[index],pbuf,6)) {
            return true;
		}
		if (index > gNetIpIndex)
			break;
	}
	return false;
}

static int get_section_by_delimeter(char *pOutMsg,
                                         const char *pInMsg,
                                         int iLen,
                                         char c,
                                         int iCharCnt)
{
    int    i;
    int    iCnt ;
    char   *p = (char *)pInMsg;

    if (iLen <= 0
        || NULL == pOutMsg
        || NULL == pInMsg){
        TRACE("attention,got fatal error!");
        return ERROR;
    }
    iCnt = 0;
    while (iLen--){
        if (0 == iCharCnt)
            break;
        if (*p++ == c){
            iCnt++;
            if (iCnt == iCharCnt)
                break;
        }
    }
    for (i = 0; i < iLen; i++){
        if (p[i] == c)
            break;
    }
    strncpy(pOutMsg,p,i);
	pOutMsg[i] = 0;
    return OK;
}

static bool is_connected_black_list(const void *pip,unsigned long ulPort)
{
    char          tmp[12] = {0};
	unsigned char buf[6]  = {0};

    __ASSERT(NULL != pip );
    char * ip = (typeof(ip))pip;
    stSecurityParam stparam;
	memset(&stparam, 0, sizeof(stSecurityParam));	
	get_security_param(&stparam);
	__VERIFY(get_section_by_delimeter(tmp,ip,strlen(ip),0x2E,0) == OK);
	_Api_ToolLong2Char(atol(tmp),1,buf);
	memset(tmp,0,sizeof(tmp));
	__VERIFY(get_section_by_delimeter(tmp,ip,strlen(ip),0x2E,1) == OK);
	_Api_ToolLong2Char(atol(tmp),1,buf + 1);
	memset(tmp,0,sizeof(tmp));
	__VERIFY(get_section_by_delimeter(tmp,ip,strlen(ip),0x2E,2) == OK);
	_Api_ToolLong2Char(atol(tmp),1,buf + 2);
	memset(tmp,0,sizeof(tmp));
	__VERIFY(get_section_by_delimeter(tmp,ip,strlen(ip),0x2E,3) == OK);
	_Api_ToolLong2Char(atol(tmp),1,buf + 3); 				
	_Api_ToolLong2Char(ulPort,2,buf + 4);
	unsigned char c = *(buf + 4);
	*(buf + 4) = *(buf + 5);
	*(buf + 5) = c;
    for (int i = 0; i < stparam.ucNetNum; i++) {
        if ((0 == memcmp(stparam.stNetList[i].szRemoteIp,buf,MAX_IP_LEN))
			&& (0 == memcmp(stparam.stNetList[i].szRemotePort,buf + 4,MAX_PORT_LEN))) {
            return false;
		}
	}
	memcpy(gstEventDetails.szNetInfo,buf,MAX_IP_LEN + MAX_PORT_LEN);
    return true;
}

static void __netstat(const char *filename, const char *label) 
{   
    unsigned char tmp[8];
	FILE *fp = fopen(filename, "r");    
	if (fp == NULL) {        
		return;    
	}    
	char buf[BUFSIZ];    
	fgets(buf, BUFSIZ, fp);   
	while (fgets(buf, BUFSIZ, fp)){        
		char lip[ADDR_LEN];        
		char rip[ADDR_LEN];        
		iaddr laddr, raddr;        
		unsigned lport, rport, state, txq, rxq, num;        
		int n = sscanf(buf, " %d: %x:%x %x:%x %x %x:%x", \
			&num, &laddr.u, &lport, &raddr.u, &rport,&state, &txq, &rxq);
		if (n == 8) {  
			if (ESTABLISHED == state) {
				memset(tmp, 0, sizeof(tmp));
				_Api_ToolLong2Char(lport,MAX_PORT_LEN,tmp);	
				if (!check_port_send_record_ex(tmp)) {
					TRACE("net port opened event!!!!\n");				
					insert_port_record(tmp);
					SEND_NETPORT_EVENT(tmp);
				}
			}
			if (LISTEN == state) {
                char ip[INET6_ADDRSTRLEN + 1] = {0};
				char port[6] = {0};
				__VERIFY(inet_ntop(AF_INET,	&raddr, ip, INET6_ADDRSTRLEN) != NULL) ;
				if (is_connected_black_list(ip,rport)) {
					#if 0 //sy 20181224 14:30
					for(int i = 0; i< 6; i++){
                        printf("%02x ",gstEventDetails.szNetInfo[i]);
					}
					#endif
					if (!check_net_send_record_ex(gstEventDetails.szNetInfo)) {
						TRACE("external net connected event!!!!!!\n");
						insert_net_record(gstEventDetails.szNetInfo);
                        SEND_NETEST_EVENT;
						//update_ip_event_log(&stnetip,gNetIpIndex);
					}
				}
			}
		}    
	}    
	fclose(fp);
}



/**
* first of all, some ports opened event detected and send message to application already
* later,it also detected these ports opened by shell command scanning, at this time 
* do not need to send message to application again,so ,terminal need to save event record.
*/

static int update_port_event_log(void *pstLog, unsigned int uiIndex)
{
	int fd = file_open(NET_PORT_EVENT_LOG, O_RDWR|O_CREAT);
	if (fd < 0)
		return ERROR;
	int ret = file_seek(fd,sizeof(stNetPortRecord) * uiIndex,SEEK_SET);
    if (ret < 0)
		return ERROR;

	int iWriteBytes = file_write(fd, pstLog, sizeof(stNetPortRecord));
	file_close(fd);
	
	if (iWriteBytes != sizeof(stNetPortRecord))
        return ERROR;
	
	return OK;
}

static int update_ip_event_log(void *pstLog, unsigned int uiIndex)
{
	int fd = file_open(NET_IP_EVENT_LOG, O_RDWR|O_CREAT);
	if (fd < 0)
		return ERROR;

	int ret = file_seek(fd,(long)(sizeof(stNetIpRecord) * uiIndex),SEEK_SET);
    if (ret < 0)
		return ERROR;

	int iWriteBytes = file_write(fd, pstLog, sizeof(stNetIpRecord));
	file_close(fd);
	
	if (iWriteBytes != sizeof(stNetIpRecord))
        return ERROR;
	
	return OK;
}


static int load_port_event_log(void *pstLog, unsigned int uiIndex)
{
	int fd = file_open(NET_PORT_EVENT_LOG, O_RDWR);
	if (fd < 0) 
		return ERROR;

	int ret = file_seek(fd,sizeof(stNetPortRecord) * uiIndex,SEEK_SET);
    if (ret < 0) 
		return ERROR;

	int iReadBytes = file_read(fd, pstLog, sizeof(stNetPortRecord));
	file_close(fd);
	
	if (iReadBytes != sizeof(stNetPortRecord)) 
        return ERROR;
	return OK;
}

static int load_ip_event_log(void *pstLog, unsigned int uiIndex)
{
	int fd = file_open(NET_IP_EVENT_LOG, O_RDWR);
	if (fd < 0)
		return ERROR;

	int ret = file_seek(fd,(long)(sizeof(stNetIpRecord) * uiIndex),SEEK_SET);
    if (ret < 0)
		return ERROR;


	int iReadBytes = file_read(fd, pstLog, sizeof(stNetIpRecord));
	file_close(fd);
	
	if (iReadBytes != sizeof(stNetIpRecord))
        return ERROR;
	
	return OK;
}


static bool check_port_send_record(const void * pbuf)
{
	#define MAX_LOG_INDEX    100
	#if 1 //sy 20181213 14:20
	unsigned char buf[3];
	unsigned char * port = (typeof(port))pbuf;
    for (int index = 0; index < MAX_LOG_INDEX; index++) {
		memset(buf, 0, sizeof(buf));
        int ret = load_port_event_log(buf,index);
		if (ret != OK) {
            remove(NET_PORT_EVENT_LOG);
			return false;
		}
		if (0 == memcmp(buf,pbuf,2)) {
            return true;
		}
	}
	#endif
	return false;
    #undef MAX_LOG_INDEX
}

static bool check_ip_send_record(const void * pbuf)
{
	#define MAX_LOG_INDEX    100
	stNetIpRecord stnetip;
	
    for (int index = 0; index < MAX_LOG_INDEX; index++) {
		memset(&stnetip, 0, sizeof(stnetip));
        int ret = load_ip_event_log(&stnetip,index);
		if (ret != OK) {
            remove(NET_IP_EVENT_LOG);
			return false;
		}
		if (0 == memcmp(stnetip.szBlackNet,pbuf,6)) {
            return true;
		}
	}
	return false;
    #undef MAX_LOG_INDEX
}

static void get_ip_port(char * ip,char * port,const char * pSrc)
{
    int i ;
	int len = strlen(pSrc);
    for (i = 0; i < len; i++) {
		if (pSrc[i] == 0x3A) {
		    strncpy(ip,pSrc,i);	
			ip[i] = 0;
			break;
		}
	}	
	strncpy(port,pSrc + i + 1, len - i - 1);
	port[len - i - 1] = 0;
}

#if 0 //sy 20181224 13:49
void * verify_net_event_activity(void *arg)
{
    #define ISENTER(c)	  ((c == '\r') || (c == '\n'))
	int  i = 0,j = 0;
	char buf[128] = {0};
    __ASSERT(NULL != arg);
    stNetEventReport * pstneteventrpt = (stNetEventReport * )arg;
    while (1) {
		/**
		* 1.net port open will generate event 
		* 2.black list net connected will generate event
		* 3.ppp0 poprt open will generate event
		* 4.ppp0 connected will generate event
		*/
        __VERIFY(__System("netstat -atn | grep LISTEN | awk '{print $4}' | cut -d : -f 2",\
        buf,sizeof(buf)) == OK);
		for (i = 0,j = 0; i < strlen(buf); i++) {
			if (ISENTER(buf[i])) {
				char temp[6];
				memset(temp,0,sizeof(temp));
				int len = (i - j);
				strncpy(temp,buf + j,len);
				temp[len] = '\0';
				_Api_ToolLong2Char(atol(temp),2,gstEventDetails.szNetPort);	
				if (!check_port_send_record_ex(gstEventDetails.szNetPort)) {
					send_event_to_application();
					insert_port_record(gstEventDetails.szNetPort);
					reset_event_log();
				}
				j = (i + 1);
			}
		}	
		usleep(400*1000);
		memset(buf,0,sizeof(buf));
        __VERIFY(__System("netstat -atn | grep ESTABLISHED | awk '{print $5}'",\
			buf,sizeof(buf)) == OK);
		for (i = 0,j = 0; i < strlen(buf); i++) {
			if (ISENTER(buf[i])) {
                char temp[32] = {0};
                char ip[16] = {0};
				char port[6] = {0};
				int len = (i - j);
				strncpy(temp,buf + j,len);
				temp[len] = '\0';				
				get_ip_port(ip,port,temp);
				if (is_connected_black_list(ip,port)) {
					if (!check_net_send_record_ex(gstEventDetails.szNetInfo)) {
                        send_event_to_application();
						insert_net_record(gstEventDetails.szNetInfo);
						reset_event_log();
						//update_ip_event_log(&stnetip,gNetIpIndex);
					}
				}
				j = (i + 1);
			}
		}				
		usleep(400*1000);				
	}
    #undef ISENTER
    return NULL;
}
#endif


void * verify_net_event_activity(void *arg)
{
    __netstat("/proc/net/tcp",  "tcp");
    return NULL;
}


static ExitMonitorCode 
	net_event_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stNetEvent *pne = (stNetEvent * )pstMonitorItem->userconf;
	if (0 != pne->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pne->tid);
		pthread_join(pne->tid,NULL);
		pne->tid = 0;
	}
	return rc;
}



static ExitMonitorCode
	net_event_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

    __ASSERT(NULL != pstMonitorItem);
	stNetEvent  *pne = (stNetEvent * )pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_NET_STAT){
		if (pthread_create(&pne->tid,NULL,verify_net_event_activity, \
			&(pne->stneteventrpt)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}		
		usleep(200*1000);
	}
	return rc;
}


RegisterMonitorItem(net_event,0,&stnetevent)

