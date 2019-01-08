#include "monitor.hpp"
#include <utmp.h>
#include <time.h>
//#include "list.h"

typedef struct {
	pthread_t	          tid;
	stLoginEventReport    stlogeventrpt;
} stLoginEvent;

static stLoginEvent stloginevent = {
	.tid	            = 0,
	.stlogeventrpt	    = {"*login event*",0},
};

typedef struct {
    char dev[4];
    char time[21];
}stLoginRecord;


static stLoginRecord gszLoginRecord[MAX_LOGIN_NUM];
static unsigned char gucIndex = 0;


/**
 * strnstr - Find the first substring in a length-limited string
 * @s1: The string to be searched
 * @s2: The string to search for
 * @len: the maximum number of characters to search
 */
static char *strnstr(const char *s1, const char *s2, size_t len)
{
	size_t l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	while (len >= l2) {
		len--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

static int __login_exit(int a, int b)
{
    static int c = 0;
	if (0 != a) {
		c = b;
	}
	return c;
}

static void set_login_exit(int val)
{
    __login_exit(1,val);
}

static int get_login_exit(void)
{
    return __login_exit(0, 0);
}

static int __tty_login_flag(int a, int b)
{
    static int x = 0;
	if (0 != a) {
		x = b;
	}
	return x;
}

static void set_tty_login_flag(int val)
{
    __tty_login_flag(1,val);
}

static int get_tty_login_flag(void)
{
    return __tty_login_flag(0, 0);
}


static void reset_login_record(void)
{
	gucIndex = 0;
	memset(gszLoginRecord, 0, sizeof(gszLoginRecord));
	set_login_exit(1);
}

static bool check_time_diff(const char * timestr)
{
    for (int i = 0; i < ARRAY_SIZE(gszLoginRecord); i++) {
        if (strncmp(gszLoginRecord[i].time,(char *)timestr, \
			strlen((char *)timestr)) == 0) {
            return false;
		}
	}
	return true;
}

static void insert_login_record(const char *dev, const char * timestr)
{
    __ASSERT(NULL != dev && NULL != timestr);
    if (check_time_diff(timestr)) {
        strncpy(gszLoginRecord[gucIndex].dev,(char *)dev,strlen((char *)dev));
		strncpy(gszLoginRecord[gucIndex].time,(char *)timestr,strlen((char *)timestr));
		gucIndex++;
		if (gucIndex >= MAX_LOGIN_NUM) {
			gucIndex = 0;		
			memset(gszLoginRecord,0,sizeof(gszLoginRecord));
		}
	}
}



void * verify_login_event_activity(void *arg)
{ 
    long           ulTime;
    struct utmp    *putent;
	int            num = 0;
	bool           btty = false;
	setutent();
	while ((putent = getutent()) != NULL) {
        if (putent->ut_type != USER_PROCESS) {
            continue;
		}
        ulTime = putent->ut_tv.tv_sec;
		if (!get_login_exit()) {
			if (NULL != strstr(putent->ut_line,"tty")) {
				btty = true;
				if (0 == get_tty_login_flag()) {
					TRACE("serial port login on event\n");
					SEND_SERIAL_LOGON_EVENT;
					set_tty_login_flag(1);
				}
			}
			if (NULL != strstr(putent->ut_line,"pts")
				&& check_time_diff(ctime(&ulTime) + 4)) {
				TRACE("telnet/ssh login on event\n");
				SEND_NET_LOGON_EVENT;
			}	
		}
		insert_login_record(putent->ut_line,ctime(&ulTime) + 4);
		num++;
	}
	endutent(); 
	set_login_exit(0);
    if (num < gucIndex) {
		if (!btty && get_tty_login_flag()) {
			TRACE("serial port login exit event!!!\n");
			set_tty_login_flag(0);			
		    SEND_SERIAL_LOG_EXIT_EVENT;
		} else {
            TRACE("telnet/ssh login exit event!!!\n");
			SEND_NET_LOG_EXIT_EVENT;
		}  
	    reset_login_record();
	}
    return NULL;
}


static ExitMonitorCode 
	login_event_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stLoginEvent *p = (stLoginEvent * )pstMonitorItem->userconf;
	if (0 != p->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(p->tid);
		pthread_join(p->tid,NULL);
		p->tid = 0;
	}
	return rc;
}



static ExitMonitorCode
	login_event_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
    __ASSERT(NULL != pstMonitorItem);
	stLoginEvent  *p = (stLoginEvent * )pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_LOGINEVENT){
		if (pthread_create(&p->tid,NULL,verify_login_event_activity, \
			&(p->stlogeventrpt)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(800*1000);
	}
	return rc;
}

RegisterMonitorItem(login_event,0,&stloginevent)


