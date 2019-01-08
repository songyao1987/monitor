
#include "monitor.hpp"

typedef struct {
	pthread_t	   tid;
	int            bad_block_num;
} stNandBlockCheck;

static stNandBlockCheck stnandblockcheck = {
	.tid	            = 0,
	.bad_block_num	    = 0,
};

static int nand_block_bad(const char * pDev, int * pBadBlock)
{
    #if 0 //sy 20181205 09:59
    int           fd,ret ;
    loff_t        offset;
    mtd_info_t    meminfo;
    unsigned long ofs, end_addr = 0;
	
    __ASSERT(NULL != pDev && NULL != pBadBlock);
	
	fd = open(pDev,O_RDONLY);
	if (fd < 0) {
        printf("open error");
		goto err_exit;
	}

	if (ioctl(fd, MEMGETINFO, &meminfo) != OK) {
        close(fd);
		printf("ioctl error");
		goto err_exit;
	}
	
	if (!(meminfo.oobsize == 128) &&
	    !(meminfo.oobsize == 64)  &&
	    !(meminfo.oobsize == 32)  &&
	    !(meminfo.oobsize == 16)  && 
	    !(meminfo.oobsize == 8)) {
	    printf ("Unknown type of flash (not normal NAND)\n");
		close(fd);
	    goto err_exit;
	}

	for (ofs = 0; ofs < meminfo.size; ofs += meminfo.erasesize) {
        offset = ofs;
        ret = ioctl(fd, MEMGETBADBLOCK, &offset);
		if (ret > 0) {
            (*pBadBlock)++;
		} else if (ret < 0) {
            goto err_exit;
		}
	}
err_exit:
	if (fd > 0) {
		close(fd);
		fd = -1;
	}
	return ret;
	#else
    #endif
}


static void * verify_nand_block_bad(void *arg)
{
    char buf[16] ;
	int * bad_block = (int *)arg;
#define MTD_NUM    9
    for (int i = 0; i < MTD_NUM; i++) {
        memset(buf,0,sizeof(buf));
		snprintf(buf,sizeof(buf),"/dev/mtd%d",i);
		nand_block_bad(buf,bad_block);
	}
	if (*bad_block > gstMonitorParam.uiBadblkLimit) {
        set_err_exist(E_BADFLASH_EXCEED_LIMIT);
	}
#undef  MTD_NUM
    return NULL;
}

static ExitMonitorCode 
	nand_block_bad_stop_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;
	stNandBlockCheck * pstnandchk   = (stNandBlockCheck  *)pstMonitorItem->userconf;

    __ASSERT(NULL != pstMonitorItem);
	if (0 != pstnandchk->tid){
		/**
		* send cancel signal to this thread and
		* also waitting for the thread exit completely 
		*/
	    pthread_cancel(pstnandchk->tid);
		pthread_join(pstnandchk->tid,NULL);
		pstnandchk->tid = 0;
	}
	return rc;
}

static ExitMonitorCode 
	nand_block_bad_start_monitor(stMonitorItem * pstMonitorItem)
{
	ExitMonitorCode       rc = E_MONITOR_PROCESS_OK;

	__ASSERT(NULL != pstMonitorItem);
	stNandBlockCheck *pstnandchk = (stNandBlockCheck  *)pstMonitorItem->userconf;
	pstMonitorItem->stop_monitor(pstMonitorItem);
	if (__IF_CHECK_NAND){
		pstnandchk->bad_block_num = 0;
		if (pthread_create(&pstnandchk->tid,NULL,verify_nand_block_bad,&(pstnandchk->bad_block_num)) != 0){
			rc = E_MONITOR_THREAD_ERR;
			TRACE("errno = %d\n",errno);
		}
		usleep(400*1000);
	}
	return rc;
}

void sync_nand_block_bad_info(stMonitorReport *pstmonitorrpt)
{
    __ASSERT(NULL != pstmonitorrpt);
    pstmonitorrpt->inandbadblocknum = stnandblockcheck.bad_block_num;
}


//RegisterMonitorItem(nand_block_bad,0,&stnandblockcheck)


