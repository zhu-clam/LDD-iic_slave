#include <unistd.h>
#include <stdio.h>
//#include <curses.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>


#include "gv_host_channel_api.h"


int gv_machine_id=-1;

//芯片ID
int gv_chip_id=-1;

__GV_HOST_CHANNEL  reset_chn_tx;

/*当tun 服务没有使用时,可以使用. reset special chip-id's chip */
int main(int argc, char** argv)
{
	int i;
	int ret;


	printf("Be carefull,you could not execute it when tun serve is running!\n");
	
	if(argc<1)
	{
		ERR1:
		printf("%s [gv_chip_id]\n",argv[0]);
		return 1;
	}

	//gv_machine_id=atol(argv[1]);
	gv_chip_id=atol(argv[1]);
	if (gv_chip_id<0)
	{
		goto ERR1;
	}


	
    if (gv_HostChannel_Open(&reset_chn_tx,gv_chip_id,(NUM_DATA_CHANNELS-1))<0)//25
	goto ERR1;

	/*Step1.enable wdt */
	gv_clr_wdt(reset_chn_tx.fd,1);

	/*Step2. stop kick wdt*/
	gv_HostChannel_Close(&reset_chn_tx);
	
	return 0;
}
