
#include "gv_host_channel_api.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>


struct GV_ShellHead
{
	int  argc;
	char argv[6][128];  //aa r/w file1 file2 123
};



__GV_HOST_CHANNEL  gv9531_cp;

static int RW_BUF_SIZE=(1024*1024*2);


struct GV_ShellHead   cp_Head;

int main(int argc, char **argv)
{

	int ret,i;
	char *test_buf;
	char *LocalFile=NULL;
	char RemoteFile[256]={0};

	int gv_chip_id=-1;
	int es_fd=-1;
	
	time_t  start_ts=0;
	unsigned long SendByteCount=0;
	
	struct stat  file_stat;
	int cp_flag=-1;
	
	struct timeval t_start,t_end;
	
	
	printf("Build: %s %s\n",__DATE__,__TIME__);
		
	if(argc<3)
	{
		ERR1:
		printf("example:\n");
		printf("\t%s chip-id command  argv...\n",argv[0]);
		return 1;
	}


	memset(&cp_Head,0,sizeof(struct GV_ShellHead));
    strncpy(cp_Head.argv[0],"/usr/local/sbin/gv_ep_shell",128);
	
	sscanf(argv[1],"gv%d",&gv_chip_id);
	if(gv_chip_id<0)
	{
		printf("chipid-error!\n");
		goto ERR1;
	}
	
	printf("ChipID:%d\n",gv_chip_id);
	printf("\tcommand:%s\n",argv[2]);
	for(i=2;i<argc;i++)
	{
		strncpy(cp_Head.argv[i-1],argv[i],128);
		printf("\targc[%d]:%s\n",i-1,cp_Head.argv[i-1]);
	}
	

	printf("Remote-shell:");
	for(i=0;i<6;i++)
	{
		if (0!=cp_Head.argv[i][0])
		{
			printf(" %s",cp_Head.argv[i]);
			cp_Head.argc++;
		}
	}
	printf("\n");
	


    test_buf=malloc(RW_BUF_SIZE);
    if (NULL!=test_buf)
    	memset(test_buf,0,RW_BUF_SIZE);
	


	{
		__GV_HOST_CHANNEL  gv9531_cp_head;
		
		time_t Wait_ts=0;
		start_ts=time(NULL);
		
		if (gv_HostChannel_Open(&gv9531_cp_head,gv_chip_id,(NUM_DATA_CHANNELS-2))<0)
			goto ES_EXIT;
		
		gv_SetRemote_ShellMode(&gv9531_cp_head);
	    ret=write(gv9531_cp_head.fd,&cp_Head,sizeof(struct GV_ShellHead));
	    if(ret<=0)
	    {
	    	perror("write");
	    	goto ERR_EXIT;
	    }
	    gv_ClrRemote_ShellMode(&gv9531_cp_head);
	    gv_HostChannel_Close(&gv9531_cp_head);
	
		//打开读取通道
		gv_HostChannel_Open(&gv9531_cp,gv_chip_id,7);
		gv_SetRemote_ShellMode(&gv9531_cp);
		gv_ClrRemote_ShellMode(&gv9531_cp);

		printf(">>>>>>>>>>>>>>>>>>>>>gv%d-shell-result>>>>>>>>>>>>>>>>>>>>>\n",gv_chip_id);
	    sleep(1);
	    while(gv9531_cp.fd>0)
	    {
	    	
			ret = read(gv9531_cp.fd,test_buf,RW_BUF_SIZE); 
			if( ret>0)
	    	{
				printf("%s/n",test_buf);
				break;
	    	}
	    	else if(ret==0)
	    	{
	    		
	    		if (0==Wait_ts)
	    			Wait_ts=time(NULL);
	    		else
	    		{
	    			if((time(NULL)-Wait_ts)>3)
	    			{
	    				printf("Error:shell timeout\n");
	    				break;
	    			}
	    		}
	    	}
	    	else if (ret<0)
	    	{
	    		perror("read");
	    		break;
	    	}
	    		
	
		}
			
	}
	printf("<<<<<<<<<<<<<<<<<<<<<gv%d-shell-result<<<<<<<<<<<<<<<<<<<<<\n",gv_chip_id);

ERR_EXIT:
	gv_HostChannel_Close(&gv9531_cp);
ES_EXIT:
	close(es_fd);
	if (NULL!=test_buf)
		free(test_buf);

    return 0;
}


