
#ifndef __PCI_CHANNEL_PROC_H
#define __PCI_CHANNEL_PROC_H

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


extern int pci_gv9531_proc_init(char *proc_file_name,struct pci_epf_data_transfer  *channel_base,int chip_id,int channel_nums);
extern void pci_gv9531_proc_exit(int chip_id);


#endif

