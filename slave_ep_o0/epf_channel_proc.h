
#ifndef __PCI_CHANNEL_PROC_H
#define __PCI_CHANNEL_PROC_H

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


extern int __attribute__((optimize("-O0"))) pci_gv9531_proc_init(char *proc_file_name,struct epf_data_transfer  *channel_base,int channel_nums);
extern void __attribute__((optimize("-O0")))pci_gv9531_proc_exit(void);


#endif

