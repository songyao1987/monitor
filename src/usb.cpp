
#include "monitor.hpp"

typedef struct {
	pthread_t	   tid;
	unsigned char  ucUsb;
} stUsbDetect;

static stUsbDetect stusbdetect = {
	.tid	            = 0,
	.ucUsb       	    = 0,
};

static int g_id;

void set_socket_id(int id)
{
    g_id = id;
}

static bool check_usb_plug_detect(void)
{
    stSecurityParam stparam;
	memset(&stparam, 0, sizeof(stSecurityParam));	
	get_security_param(&stparam);
    if (0 == stparam.ucUsbAuthFlag) {
        return true;
	}
	return false;
}

static void * verify_usb_detect_event(void *arg)
{
	static struct timeval tx;
	struct timeval ti,ts,tn;
    ti.tv_sec = 5;
	ti.tv_usec = 0;
    static unsigned char flag = 0;
	while (1) {
	    char buf[UEVENT_BUFFER_SIZE * 2] = {0};  
	    recv(g_id, &buf, sizeof(buf), 0);  
		if (NULL != strstr(buf,"add@")) {
			if (0 == flag) {
                gettimeofday(&tn, NULL);  
				timeradd(&tn, &ti,&tx);
			}
			gettimeofday(&ts, NULL);  
			if (timercmp(&ts, &tx, <=)) {
				if (0 == flag) {
					if (check_usb_plug_detect()) {
						set_bit(gstEventDetails.ucUsb,0);
					}					
					TRACE("got usb insert event!!!!!!!!\n");
					SEND_USB_PLUG_EVENT;
					flag= 1;
				}
			} else {
                flag = 0;
			}
		}	
		usleep(400*1000);
	}
	return NULL;
}


static ExitMonitorCode 
	usb_detect_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stUsbDetect           * pstusbdetect = (stUsbDetect  *)pstMonitorItem->userconf;

    __ASSERT(NULL != pstMonitorItem);
	if (0 != pstusbdetect->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pstusbdetect->tid);
		pthread_join(pstusbdetect->tid,NULL);
		pstusbdetect->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	usb_detect_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stUsbDetect *pstusbdetect = (stUsbDetect  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_USB){
		pstusbdetect->ucUsb = 0;
		if (pthread_create(&pstusbdetect->tid,NULL,verify_usb_detect_event,&(pstusbdetect->ucUsb)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}




RegisterMonitorItem(usb_detect,0,&stusbdetect)

