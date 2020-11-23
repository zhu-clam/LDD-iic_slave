#ifndef __EPF_CALL_USER_APP_H
#define __EPF_CALL_USER_APP_H


#define CP_FILE_HOST2GV  0x40
#define CP_FILE_GV2HOST  0x80

struct GV_ShellHead
{
	int  argc;
	char argv[6][128];  //aa r/w file1 file2 123
};


extern int __attribute__((optimize("-O0")))call_user_app(char *path,char *ip,char *mask,char *gw,char *dns);
extern int __attribute__((optimize("-O0")))call_user_shell(int argc,char argv[6][128]);


#endif
