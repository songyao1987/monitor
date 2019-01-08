
#include "monitor.hpp"


typedef struct {
	pthread_t	       tid;
	stFilsSystemReport stfsrpt;
} stFileSystem;

static stFileSystem stfsprivdata = {
	.tid	= 0,
	.stfsrpt	    = {"*file system*",0},
};

typedef struct {
    unsigned int uiMask;
    char         event[64];
}stEventMap;

static stEventMap steventmap[] = {
    {IN_ACCESS,            "File was accessed"},
	{IN_MODIFY, 	       "File was modified"},
	{IN_ATTRIB, 	       "Metadata changed"},
	{IN_CLOSE_WRITE, 	   "Writtable file was closed"},
	{IN_CLOSE_NOWRITE, 	   "Unwrittable file closed"},
	{IN_OPEN, 	           "File was opened"},
	{IN_MOVED_FROM, 	   "File was moved from X"},
	{IN_MOVED_TO, 	       "File was moved to Y"},
	{IN_CREATE, 	       "Subfile was created"},
	{IN_DELETE, 	       "Subfile was deleted"},
	{IN_DELETE_SELF, 	   "Self was deleted"},
	{IN_MOVE_SELF, 	       "Self was moved"},
	{IN_UNMOUNT, 	       "Backing fs was unmounted"},
	{IN_Q_OVERFLOW, 	   "Event queued overflowed"},
	{IN_IGNORED, 	       "File was ignored"},
};

static int ifd = 0;
static int iwd[MAX_DIR_NUM];

void set_fs_fd(int fd, int * wd)
{
    stSecurityParam stparam;
	memset(&stparam, 0, sizeof(stSecurityParam));
	
    ifd = fd;
	get_security_param(&stparam);
	for (int i = 0; i < stparam.ucDirNum; i++) {
        iwd[i] = wd[i]; 
	}
}

void get_event_by_mask(unsigned int uiMask, char *pOut)
{
    __ASSERT(NULL != pOut);
    for (int i = 0; i < ARRAY_SIZE(steventmap); i++) {
        if (uiMask == steventmap[i].uiMask) {
            memcpy(pOut, steventmap[i].event, strlen(steventmap[i].event));
			return;
		}          
	}
}

static bool check_file_filter(const void * pName)
{
    #define    MAX_FILE_LEN   32
    __ASSERT(NULL != pName);
    /**
    * please write your filter files by here 
	*/
	const static unsigned char szFilter[][MAX_FILE_LEN] = {
        "comminfo",
        "channeltypeflg",
	};

    char *pname = (typeof(pname))pName;
	int  len = strlen(pname);
	if (len > MAX_FILE_LEN) {
		len = MAX_FILE_LEN;
	}
	
	for (int i = 0; i < ARRAY_SIZE(szFilter); i++) {
        if (0 == strncmp(pname,(char *)szFilter[i],len)) {
            return true;
		}
	}
	return false;
	#undef    MAX_FILE_LEN
}




int read_inotify_fd(int fd, void *p)
{
    int res;
    char event_buf[256];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;

	__ASSERT((fd > 0) && (NULL != p));	
    stSecurityParam stparam;
	memset(&stparam, 0, sizeof(stSecurityParam));	
	get_security_param(&stparam);
	stFilsSystemReport *stfsrpt = (stFilsSystemReport *)p;
    res = file_read(fd, event_buf, sizeof(event_buf));
    if(res < (int)sizeof(*event)) {
        TRACE("could not get event, %s\n", strerror(errno));
        return ERROR;
    }
    while(res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);		
        if(event->len
			&& (!check_file_filter(event->name))) {
            //printf("event->mask = %04x",event->mask);
			TRACE("file system(%s) changed event!!!!!!!!\n",event->name);
			for (int i = 0; i < stparam.ucDirNum; i++) {
                if (iwd[i] == event->wd) {
                    //printf("dir:%s\n",stparam.szFileDir[i]);
					snprintf(stfsrpt->event_name,sizeof(stfsrpt->event_name), \
						"%s",(char *)stparam.szFileDir[i]);					
					strncpy((char *)gstEventDetails.szKeyDirectory,(char *)stparam.szFileDir[i],\
						strlen((char *)stparam.szFileDir[i]));
					break;
				}  
			}
            stfsrpt->uiEventNum++;
            /**
            * get file system changed event,then send message to application
			*/ 
			SEND_FILESYSTEN_EVENT;
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
    return OK;
}

void * verify_fs_activity(void *arg)
{
    while (1) {
	    read_inotify_fd(ifd,arg);   	
		usleep(200*1000);
	}
    return NULL;
}


static ExitMonitorCode 
	file_system_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stFileSystem  *pfs = (stFileSystem  *)pstMonitorItem->userconf;
	if (0 != pfs->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pfs->tid);
		pthread_join(pfs->tid,NULL);
		pfs->tid = 0;
	}
	return rc;
}



static ExitMonitorCode
	file_system_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

    __ASSERT(NULL != pstMonitorItem);
	stFileSystem  *pfs = (stFileSystem  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_FS){
		if (pthread_create(&pfs->tid,NULL,verify_fs_activity, \
			&(pfs->stfsrpt)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}

void sync_dir_changed_event(stEventDetails * pstevent)
{
    __ASSERT(NULL != pstevent);
	snprintf((char *)pstevent->szKeyDirectory,sizeof(pstevent->szKeyDirectory), \
		"%s",stfsprivdata.stfsrpt.event_name);
}

void sync_fs_event(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    memcpy(&pstmonitorrpt->stfsrpt,&(stfsprivdata.stfsrpt),sizeof(stFilsSystemReport));
}

RegisterMonitorItem(file_system,0,&stfsprivdata)


