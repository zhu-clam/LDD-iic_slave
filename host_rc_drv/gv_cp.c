
#include "gv_host_channel_api.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>


#define CP_FILE_HOST2GV  0x40
#define CP_FILE_GV2HOST  0x80

struct GV_ShellHead
{
	int  argc;
	char argv[6][128];  //aa r/w file1 file2 123
};

#define CP_PHY_ARRAY_MEM 1

__GV_HOST_CHANNEL  gv9531_cp;

static int RW_BUF_SIZE=(1024*1024*2);
static int READ_SIZE=(1024*1024*1);

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
	int show_file_flag_host = -1;
	//int transfer_direction = -1; //-1 initilization status; 0 to ep; 1 from ep.
	int Transfer_SIZE = (100*1024*1024);
	
	struct timeval t_start,t_end;
	
	
	printf("Build: %s %s\n",__DATE__,__TIME__);
	
	printf("__%s,__%d,argc=%d\n",__FILE__,__LINE__,argc);
	for(i=0;i<argc;i++)
	{
		printf("argc[%d]=%s\n",i,argv[i]);
	}
	
	if(argc<3)
	{
	//host-to-ep	
	// argv[0]:./gv-cp ;argv[0]: local_file_path; argv[1]:gv-[id]:remote_file;argv[2]:file_size;argv[3]-i;
	//ep-to-host
	// argv[0]: ./gv-cp ; argv[0]: gv-id:remote_file; argv[1]:local_file_path;argv[2]:file_size;argv[3]-i;
	// ./gv-cp to_ep gv-id size-number
	// ./gv-cp from_ep gv-id size-number
		ERR1:
		printf("%s [file]  gv-[0-4]:[file] \n",argv[0]);
		printf("%s gv-[0-4]:[file]  [file] \n",argv[0]);
		printf("%s to (ep)/from(ep)  gv-[0-4] size-number\n",argv[0]);//if not specify size,default is 100M
		
		return 1;
	}
	
	memset(&cp_Head,0,sizeof(struct GV_ShellHead));
	strncpy(cp_Head.argv[0],"/usr/local/sbin/gv_ep_cp",128);
	
	if((argv[1][0]=='/')||(argv[1][1]=='/')&&(argv[1][0]=='.'))
	{
		LocalFile=argv[1];
		sscanf(argv[2],"gv-%d:%s",&gv_chip_id,RemoteFile);
		cp_flag=CP_FILE_HOST2GV;
		show_file_flag_host =1;
	}
	else if((argv[2][0]=='/')||(argv[2][1]=='/')&&(argv[2][0]=='.'))
	{
		LocalFile=argv[2];
		sscanf(argv[1],"gv-%d:%s",&gv_chip_id,RemoteFile);
		cp_flag=CP_FILE_GV2HOST;
		show_file_flag_host =1;
	}
	//./gv-cp to gv-[0-4] size_number;
	else if(strncmp("to",argv[1],2) == 0) 
	{
		cp_flag=CP_FILE_HOST2GV; 
		sscanf(argv[2],"gv-%d",&gv_chip_id);
		if(argc >=4)
			Transfer_SIZE =  atoi(argv[3]);
		show_file_flag_host =0;
	}// ./gv-cp from gv-[0-4] size_number;
	else if( strncmp("from",argv[1],4) == 0) 
	{
		cp_flag=CP_FILE_GV2HOST; 
		sscanf(argv[2],"gv-%d",&gv_chip_id);
		if(argc >=4)
			Transfer_SIZE =  atoi(argv[3]);
		show_file_flag_host =0;
	}
	else
	{
		printf("ERROR:__%s,__%d\n",__FILE__,__LINE__);
		goto ERR1;
	}

	if(cp_flag == -1)
	{
		if ((gv_chip_id<0)||(RemoteFile[0]==0))
		{
			printf("ERROR:__%s,__%d\n",__FILE__,__LINE__);
			goto ERR1;
		}
	}
	// ./gv-cp to/from gv-[0-4] *size*.
	if (show_file_flag_host == 0) 
	{			
		strncpy(cp_Head.argv[1],argv[1],128);
		
		sprintf(cp_Head.argv[2],"%d",gv_chip_id);
		sprintf(cp_Head.argv[3],"%d",Transfer_SIZE);

		printf("ChipID:%d\n",gv_chip_id);
		printf("Transfer_SIZE:%d\n",Transfer_SIZE);

		for(i=0;i<6;i++)
		{
			if (0!=cp_Head.argv[i][0])
			{
				cp_Head.argc++;
			}
		}

		printf("Remote-shell:%s %s %s %s\n",cp_Head.argv[0],cp_Head.argv[1],cp_Head.argv[2],cp_Head.argv[3]);

	    test_buf=malloc(RW_BUF_SIZE);
	    if (NULL!=test_buf)
	    	memset(test_buf,0x55,RW_BUF_SIZE);

		gettimeofday(&t_start, NULL);
		if(cp_flag&CP_FILE_HOST2GV) //cp_to_ep
		{
			time_t Wait_ts;
			start_ts=time(NULL);
			
			if (gv_HostChannel_Open(&gv9531_cp,gv_chip_id,(NUM_DATA_CHANNELS-2))<0)
				goto ES_EXIT;
				
			gv_SetRemote_ShellMode(&gv9531_cp);

			ret=write(gv9531_cp.fd,&cp_Head,sizeof(struct GV_ShellHead));
			//printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
			if(ret<=0)
				goto ERR_EXIT;
			gv_ClrRemote_ShellMode(&gv9531_cp);
			
			do
			{
				int Flag;
				usleep(100000);
				ret = ioctl(gv9531_cp.fd,IOCTL_GET_FLAG,&Flag);
				printf("__%s,__%d,ret=%d,Flag=%x\n",__FILE__,__LINE__,ret,Flag);
				if(Flag&0x08)
				{//EP
					start_ts=time(NULL);
					gettimeofday(&t_start, NULL);
					break;
				}
			}while(1);
			
			
			while(gv9531_cp.fd>0)
			{
				
				if (SendByteCount == Transfer_SIZE)
					ret = 0;
				else if ((SendByteCount + RW_BUF_SIZE)>Transfer_SIZE)
					ret = Transfer_SIZE - SendByteCount; 
				else
					ret = RW_BUF_SIZE;
				
				if( ret>0)
				{		
					int write_len=ret;
					int index_i=0;
					Wait_ts=time(NULL);
					do
					{
						int w_ret;
						w_ret=write(gv9531_cp.fd,test_buf+index_i,write_len-index_i);
						//printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
						if(w_ret>0)
						{
							index_i+=w_ret;
						}
						else
						{
							if((time(NULL)-Wait_ts)>10)
							{
								printf("Error:Copy timeout\n");
								break;
							}
							//printf("__%s,__%d,w_ret=%d\n",__FILE__,__LINE__,w_ret);
							usleep(1);
						}
					}while(index_i<write_len);
						SendByteCount+=write_len;
					//printf("__%s,__%d,SendByteCount=%d,ret=%d\n",__FILE__,__LINE__,SendByteCount,ret);
				}
				else
				{
					int t;
					int duration_t;
					float duration_ms;
					gettimeofday(&t_end, NULL);
					duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
					duration_t=time(NULL)-start_ts;
					
					printf("ByteCount=%lu\n",SendByteCount);
					printf("duration_ms=%.2f[ms]\n",duration_ms);
					if (duration_ms<=0)
						duration_ms=0.001;
					
					printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
					printf("%.1fB/ms\n",SendByteCount/duration_ms);
					break;
				}		
			}
		}
		else if(cp_flag&CP_FILE_GV2HOST)//EP ---> host
		{
			__GV_HOST_CHANNEL  gv9531_cp_head;
			
			time_t Wait_ts=0;
			start_ts=time(NULL);

			//RC side,first open channel 26 for launch gv_ep_cp ,beside transfer command argument.
			if (gv_HostChannel_Open(&gv9531_cp_head,gv_chip_id,(NUM_DATA_CHANNELS-2))<0)
				goto ES_EXIT;
			
			gv_SetRemote_ShellMode(&gv9531_cp_head);
			sprintf(cp_Head.argv[3],"%d",Transfer_SIZE);
			//将CP_HEAD 的参数传递给gv_ep_cp
		    ret=write(gv9531_cp_head.fd,&cp_Head,sizeof(struct GV_ShellHead));
		    if(ret<=0)
		    {
		    	perror("write");
		    	goto ERR_EXIT;
		    }
		    gv_ClrRemote_ShellMode(&gv9531_cp_head);
		    gv_HostChannel_Close(&gv9531_cp_head);
		
			//then,RC side open channel 7 to read file data from ep side. final generate file.
			gv_HostChannel_Open(&gv9531_cp,gv_chip_id,7);
			gv_SetRemote_ShellMode(&gv9531_cp);
			gv_ClrRemote_ShellMode(&gv9531_cp);
			
			do
			{   
		    	ret=read(gv9531_cp.fd,&cp_Head,sizeof(struct GV_ShellHead));
		    	if(ret<0)
		    		goto ERR_EXIT;
		    	else
		    		usleep(1000);
		    } while(ret!=sizeof(struct GV_ShellHead));

			
		    Transfer_SIZE=atoi(cp_Head.argv[3]);
		    printf("\tgv9531-Transfer_SIZE=%d\n",Transfer_SIZE);

			printf("\targc:%d\n",cp_Head.argc);
			
			if (cp_Head.argc>6)
				cp_Head.argc=0;
			for(i=0;i<cp_Head.argc;i++)
			{
				printf("\targv[%d]=%s\n",i,cp_Head.argv[i]);
			}
			
		    start_ts=time(NULL);
	    	gettimeofday(&t_start, NULL);
			
		    while(gv9531_cp.fd>0)
		    {
		    	struct pollfd fds[1];
				fds[0].fd	  = gv9531_cp.fd;
				fds[0].events = POLLIN;
				ret = poll(fds,1,5000);
				if (ret<0)
				{
					printf("poll err\n");
					return -1;
				}
				else  if (0==ret)
				{
					int flag=0;
					printf("timeout\n");
				}
				
				ret = read(gv9531_cp.fd,test_buf,RW_BUF_SIZE);
				//printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
				if( ret>0)
		    	{
		            int w_ret;
					w_ret =  ret;  
					if (w_ret>0)
						SendByteCount+=w_ret;
					else
					{
						perror("write");
						break;
					}
					//gettimeofday(&t_end, NULL);
					//float print_ms;
					//print_ms = (t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
					//printf("__%s,__%d,ret=%d,SendByteCount:%lu,print_ms:%.2f[ms]\n",__FILE__,__LINE__,ret,SendByteCount,print_ms);
		    		if (SendByteCount>=Transfer_SIZE)
		    		{
		    			//SendByteCount = Transfer_SIZE;
			    		int t;
			    		int duration_t;
			    		float duration_ms;
			    		gettimeofday(&t_end, NULL);
			    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
			    		duration_t=time(NULL)-start_ts;
			    		
			    		printf("ReceiveByteCount=%lu\n",SendByteCount);
			    		printf("duration_ms=%.2f[ms]\n",duration_ms);
			    		if (duration_ms<=0)
			    			duration_ms=0.001;
			    		
			    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
						usleep(100);
						break;
					}					
					
		    	}


				#if 1
		    	else if(ret==0)
		    	{
		    		if (SendByteCount>=Transfer_SIZE)
		    		{
			    		int t;
			    		int duration_t;
			    		float duration_ms;
			    		gettimeofday(&t_end, NULL);
			    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
			    		duration_t=time(NULL)-start_ts;
			    		
			    		printf("ReceiveByteCount=%lu\n",SendByteCount);
			    		printf("duration_ms=%.2f[ms]\n",duration_ms);
			    		if (duration_ms<=0)
			    			duration_ms=0.001;
			    		
			    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
						usleep(100);
						break;
					}
					//printf("__%s,__%d,ret=%d,SendByteCount:%d\n",__FILE__,__LINE__,ret,SendByteCount);
					continue;
		    	}
				#endif
		    	else if (ret<0)
		    	{
		    		perror("read");
		    		break;
		    	}
		    				
			}
	
		}
	}
	else if(show_file_flag_host == 1)
	{
		if(cp_flag&CP_FILE_HOST2GV)
		{
			strncpy(cp_Head.argv[1],"host:",128);
			strncat(cp_Head.argv[1],LocalFile,128);
			strncpy(cp_Head.argv[2],RemoteFile,128);
		}
		else
		{
			strncpy(cp_Head.argv[2],"host:",128);
			strncat(cp_Head.argv[2],LocalFile,128);
			strncpy(cp_Head.argv[1],RemoteFile,128);
		}
		

		printf("ChipID:%d\n",gv_chip_id);
		printf("Local:Local-FILE:%s\n",LocalFile);
		printf("Local:Remote-FILE:%s\n",RemoteFile);
    

		es_fd=open(LocalFile,O_CREAT|O_RDWR);
		if (es_fd<0)
		{
			perror("open");
			return -1;
		}
	
		if(fstat(es_fd,&file_stat)<0)
			return -2;
			
		printf("Localfile-size:%ld\n",file_stat.st_size);
		sprintf(cp_Head.argv[3],"%ld",file_stat.st_size);//argv[3]:file-size

		for(i=0;i<6;i++)
		{
			if (0!=cp_Head.argv[i][0])
			{
				cp_Head.argc++;
			}
		}

		printf("Remote-shell:%s %s %s %s\n",cp_Head.argv[0],cp_Head.argv[1],cp_Head.argv[2],cp_Head.argv[3]);

	    test_buf=malloc(RW_BUF_SIZE);
	    if (NULL!=test_buf)
	    	memset(test_buf,0x55,RW_BUF_SIZE);
		
		gettimeofday(&t_start, NULL);

		if(cp_flag&CP_FILE_HOST2GV)
		{
			time_t Wait_ts;
			start_ts=time(NULL);
			
			//if (gv_HostChannel_Open(&gv9531_cp,gv_chip_id,15)<0)
			if (gv_HostChannel_Open(&gv9531_cp,gv_chip_id,(NUM_DATA_CHANNELS-2))<0)
				goto ES_EXIT;
				
			gv_SetRemote_ShellMode(&gv9531_cp);

		    ret=write(gv9531_cp.fd,&cp_Head,sizeof(struct GV_ShellHead));
			printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
		    if(ret<=0)
		    	goto ERR_EXIT;
		    gv_ClrRemote_ShellMode(&gv9531_cp);

			/*wait for ep set phy_mem_array into kfifo. */
			#ifdef CP_PHY_ARRAY_MEM
		    do
		    {
		    	int Flag;
		    	usleep(100000);
		    	ret = ioctl(gv9531_cp.fd,IOCTL_GET_FLAG,&Flag);
		    	printf("__%s,__%d,ret=%d,Flag=%x\n",__FILE__,__LINE__,ret,Flag);
		    	if(Flag&0x08)
		    	{//EP
		    		start_ts=time(NULL);
		    		gettimeofday(&t_start, NULL);
		    		break;
		    	}
			}while(1);
		    #endif
		    
		    while(gv9531_cp.fd>0)
		    {
				

				if (!show_file_flag_host)
				{
					if (SendByteCount == file_stat.st_size)
						ret = 0;
					else if ((SendByteCount + RW_BUF_SIZE)>file_stat.st_size)
						ret = file_stat.st_size - SendByteCount; 
					else
						ret = RW_BUF_SIZE;
				}
				else
					ret = read(es_fd,test_buf,RW_BUF_SIZE);
				if( ret>0)
		    	{		
				    int write_len=ret;
				    int index_i=0;
				    Wait_ts=time(NULL);
				    do
				    {
				            int w_ret;
				            w_ret=write(gv9531_cp.fd,test_buf+index_i,write_len-index_i);
							//printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
				            if(w_ret>0)
				            {
				                index_i+=w_ret;
				            }
				            else
				            {
				            	if((time(NULL)-Wait_ts)>10)
				            	{
				            		printf("Error:Copy timeout\n");
				            		break;
				            	}
				            	//printf("__%s,__%d,w_ret=%d\n",__FILE__,__LINE__,w_ret);
				            	usleep(1);
				            }
				    }while(index_i<write_len);
					SendByteCount+=write_len;
					//printf("__%s,__%d,SendByteCount=%d,ret=%d\n",__FILE__,__LINE__,SendByteCount,ret);
		    	}
		    	else
		    	{
		    		int t;
		    		int duration_t;
		    		float duration_ms;
		    		gettimeofday(&t_end, NULL);
		    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
		    		duration_t=time(NULL)-start_ts;
		    		
		    		printf("ByteCount=%lu\n",SendByteCount);
		    		printf("duration_ms=%.2f[ms]\n",duration_ms);
		    		if (duration_ms<=0)
		    			duration_ms=0.001;
		    		
		    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
					printf("%.1fB/ms\n",SendByteCount/duration_ms);
					break;
		    	}
		
			}
		}

		else if(cp_flag&CP_FILE_GV2HOST)//EP ---> host
		{
			__GV_HOST_CHANNEL  gv9531_cp_head;
			
			time_t Wait_ts=0;
			start_ts=time(NULL);

			//打开ch_15 读通道(NUM_DATA_CHANNELS-2)
			if (gv_HostChannel_Open(&gv9531_cp_head,gv_chip_id,(NUM_DATA_CHANNELS-2))<0)
				goto ES_EXIT;
			//if (gv_HostChannel_Open(&gv9531_cp_head,gv_chip_id,15)<0)
			//	goto ES_EXIT;
			
			gv_SetRemote_ShellMode(&gv9531_cp_head);
			sprintf(cp_Head.argv[3],"%d",-1);//-1 EP--->Host argv[3]=-1;size  argv[3]=-1
			//向gv_ep_cp 写,将CP_HEAD 的参数传递给gv_ep_cp
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
			
			do
			{   //
		    	ret=read(gv9531_cp.fd,&cp_Head,sizeof(struct GV_ShellHead));
		    	if(ret<0)
		    		goto ERR_EXIT;
		    	else
		    		usleep(1000);
		    }while(ret!=sizeof(struct GV_ShellHead));
		    file_stat.st_size=atoi(cp_Head.argv[3]);
		    printf("\tgv9531-file_stat.st_size=%ld\n",file_stat.st_size);

			printf("\targc:%d\n",cp_Head.argc);
			
			if (cp_Head.argc>6)
				cp_Head.argc=0;
			for(i=0;i<cp_Head.argc;i++)
			{
				printf("\targv[%d]=%s\n",i,cp_Head.argv[i]);
			}
		    	
		    while(gv9531_cp.fd>0)
		    {
				struct pollfd fds[1];
				fds[0].fd	  = gv9531_cp.fd;
				fds[0].events = POLLIN;
				ret = poll(fds,1,5000);
				if (ret<0)
				{
					printf("poll err\n");
					return -1;
				}
				else  if (0==ret)
				{
					int flag=0;
					printf("timeout\n");
				}
			
				ret = read(gv9531_cp.fd,test_buf,RW_BUF_SIZE);
				//printf("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
				if( ret>0)
		    	{
		       			int w_ret;
		            		w_ret=write(es_fd,test_buf,ret);
					if (w_ret>0)
						SendByteCount+=w_ret;
					else
					{
						perror("write");
						break;
					}

					if (SendByteCount>=file_stat.st_size)
		    		{
			    		int t;
			    		int duration_t;
			    		float duration_ms;
			    		gettimeofday(&t_end, NULL);
			    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
			    		duration_t=time(NULL)-start_ts;
			    		
			    		printf("ReceiveByteCount=%lu\n",SendByteCount);
			    		printf("duration_ms=%.2f[ms]\n",duration_ms);
			    		if (duration_ms<=0)
			    			duration_ms=0.001;
			    		
			    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
						usleep(100);
						break;
					}
		    	}
				#if 0
	    		if (SendByteCount>=file_stat.st_size)
	    		{
		    		int t;
		    		int duration_t;
		    		float duration_ms;
		    		gettimeofday(&t_end, NULL);
		    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
		    		duration_t=time(NULL)-start_ts;
		    		
		    		printf("ReceiveByteCount=%u\n",SendByteCount);
		    		printf("duration_ms=%.2f[ms]\n",duration_ms);
		    		if (duration_ms<=0)
		    			duration_ms=0.001;
		    		
		    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
					usleep(100);
					break;
				}	
				#endif
				#if 1
		    	else if(ret==0)
		    	{
		    		
		    		if (0==Wait_ts)
		    			Wait_ts=time(NULL);
		    		else
		    		{
		    			if((time(NULL)-Wait_ts)>10)
		    			{
		    				printf("Error:copy timeout\n");
		    				break;
		    			}
		    		}
		    		
		    		if (SendByteCount>=file_stat.st_size)
		    		{
			    		int t;
			    		int duration_t;
			    		float duration_ms;
			    		gettimeofday(&t_end, NULL);
			    		duration_ms=(t_end.tv_sec - t_start.tv_sec)*1000+(t_end.tv_usec - t_start.tv_usec)/1000;
			    		duration_t=time(NULL)-start_ts;
			    		
			    		printf("ReceiveByteCount=%lu\n",SendByteCount);
			    		printf("duration_ms=%.2f[ms]\n",duration_ms);
			    		if (duration_ms<=0)
			    			duration_ms=0.001;
			    		
			    		printf("%.1fB/s\n",1000.0*SendByteCount/duration_ms);
						usleep(100);
						break;
					}
		    	}
				#endif
		    	else if (ret<0)
		    	{
		    		perror("read");
		    		break;
		    	}
		    		
		
			}
				
		}

	}


ERR_EXIT:
	gv_HostChannel_Close(&gv9531_cp);
ES_EXIT:
	close(es_fd);
	if (NULL!=test_buf)
		free(test_buf);
	show_file_flag_host = -1;
    return 0;
}


