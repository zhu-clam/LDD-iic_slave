#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


int __attribute__((optimize("-O0"))) call_user_app(char *path,char *ip,char *mask,char *gw,char *dns){
	int ret=-1;
	//char *argv[]={path,ip,mask,gw,dns};
	char *argv[]={"/bin/sh",path,ip,mask,gw,dns,NULL};
	//char *argv[]={"/bin/sh","/usr/local/sbin/gv_tun_start.sh",NULL};
	char* envp[]={"HOME=/", "PATH=/sbin:/bin:/usr/bin:/usr/sbin:/usr/local/sbin",NULL};
	
	/*
	printk("argv[0]=%s\n",argv[0]);
	printk("argv[1]=%s\n",argv[1]);
	printk("argv[2]=%s\n",argv[2]);
	printk("argv[3]=%s\n",argv[3]);
	printk("argv[4]=%s\n",argv[4]);
	printk("argv[5]=%s\n",argv[5]);
	*/
	//ret = call_usermodehelper(path,argv,envp,UMH_WAIT_PROC);
	ret = call_usermodehelper(path,argv,envp,UMH_WAIT_EXEC);
	printk("ret=%d\n", ret);
	return 0;
}




int __attribute__((optimize("-O0"))) call_user_shell(int shell_argc,char shell_argv[6][128]){
	int ret=-1;
	char *argv[]={shell_argv[0],shell_argv[1],shell_argv[2],shell_argv[3],shell_argv[4],shell_argv[5],NULL};
	char* envp[]={"HOME=/", "PATH=/sbin:/bin:/usr/bin:/usr/sbin:/usr/local/sbin",NULL};
	ret = call_usermodehelper(shell_argv[0],argv,envp,UMH_WAIT_EXEC);
	printk("ret=%d\n", ret);
	return 0;
}


