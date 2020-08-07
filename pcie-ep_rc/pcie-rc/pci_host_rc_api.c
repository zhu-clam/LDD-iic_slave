/**
 * Userspace PCI Endpoint Test Module
 *
 * Copyright (C) 2017 Texas Instruments
 * Author: Kishon Vijay Abraham I <kishon@ti.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "pci_host_rc_api.h"


#define PCITEST_BAR		_IO('P', 0x1)
#define PCITEST_LEGACY_IRQ	_IO('P', 0x2)
#define PCITEST_MSI		_IOW('P', 0x3, int)
#define PCITEST_WRITE		_IOW('P', 0x4, unsigned long)
#define PCITEST_READ		_IOW('P', 0x5, unsigned long)
#define PCITEST_COPY		_IOW('P', 0x6, unsigned long)
#define PCITEST_MSIX		_IOW('P', 0x7, int)
#define PCITEST_SET_IRQTYPE	_IOW('P', 0x8, int)
#define PCITEST_GET_IRQTYPE	_IO('P', 0x9)
#define PCITEST_DMA_WRITE           _IOW('P', 0xa, unsigned long)
#define PCITEST_DMA_READ            _IOW('P', 0xb, unsigned long)

#define IRQ_TYPE_LEGACY				0
#define IRQ_TYPE_MSI				1
#define IRQ_TYPE_MSIX				2


static int pci_rc_fd = -1;



/*
*  pcie-rc 初始化函数：中断号配置
*/
GV_S32 gv_host_pci_rc_init()
{
    int ret;
    
    if(pci_rc_fd > 0)
        return -1;
    pci_rc_fd = open("/dev/pci-endpoint-test.0", O_RDWR);
    if(pci_rc_fd < 0)
    {
        perror("/dev/pci-endpoint-test.0");
        return -1;
    }
	//设置中断类型为 IRQ_TYPE_MSI
    ret = ioctl(pci_rc_fd, PCITEST_SET_IRQTYPE, IRQ_TYPE_MSI);
    if (ret < 0)
    {
        perror("ioctl PCITEST_SET_IRQTYPE");
        return ret;
    }
	//获取中断类型
	ret = ioctl(pci_rc_fd, PCITEST_GET_IRQTYPE);
	if (ret != IRQ_TYPE_MSI)
	    return- 1;

	/*
	* 调用驱动 pci_endpoint_test_msi_irq(test,1,0);
	*/
	ret = ioctl(pci_rc_fd, PCITEST_MSI, 1);
	if (ret < 0)
    {
        perror("ioctl PCITEST_MSI");
        return ret;
    }
    

    return 0;
}


GV_S32 gv_host_pci_rc_write(GV_PCI_TRANSMIT_S *pst_pci_st)
{
    int ret;
    
    if(NULL == pst_pci_st || NULL == pst_pci_st->ptr)
        return -1;
    
	if (pst_pci_st->dma == 0) {
		ret = ioctl(pci_rc_fd, PCITEST_WRITE, pst_pci_st);
	} else {
		ret = ioctl(pci_rc_fd, PCITEST_DMA_WRITE, pst_pci_st);
	}

    if(ret < 0)
    {
        perror("pci_pci_rc_write");
        return -1;
    }

    return 0;
}




GV_S32 gv_host_pci_rc_read(GV_PCI_TRANSMIT_S *pst_pci_st)
{
    int ret;
    
    if(NULL == pst_pci_st || NULL == pst_pci_st->ptr)
        return -1;
    
	if (pst_pci_st->dma == 0) {
		ret = ioctl(pci_rc_fd, PCITEST_READ, pst_pci_st);
	} else {
		ret = ioctl(pci_rc_fd, PCITEST_DMA_READ, pst_pci_st);
	}

    if(ret < 0)
    {
        perror("pci_pci_rc_write");
        return -1;
    }

    return 0;
}


GV_S32 gv_host_pci_rc_deinit()
{
    int ret;
    
    if(pci_rc_fd < 0)
        return -1;

    close(pci_rc_fd);

    pci_rc_fd = -1;

    return 0;
}


#if 0
#define BILLION 1E9

static char *result[] = { "NOT OKAY", "OKAY" };
static char *irq[] = { "LEGACY", "MSI", "MSI-X" };

struct pci_test {
	char		*device;
	char		barnum;
	bool		legacyirq;
	unsigned int	msinum;
	unsigned int	msixnum;
	int		irqtype;
	bool		set_irqtype;
	bool		get_irqtype;
	bool		read;
	bool		write;
	bool		copy;
	unsigned long	size;
	unsigned int	dma;
};


static void run_test(struct pci_test *test)
{
	long ret;
	int fd;

	fd = open(test->device, O_RDWR);
	if (fd < 0) {
		perror("can't open PCI Endpoint Test device");
		return;
	}

	if (test->barnum >= 0 && test->barnum <= 5) {
		ret = ioctl(fd, PCITEST_BAR, test->barnum);
		fprintf(stdout, "BAR%d:\t\t", test->barnum);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->set_irqtype) {
		ret = ioctl(fd, PCITEST_SET_IRQTYPE, test->irqtype);
		fprintf(stdout, "SET IRQ TYPE TO %s:\t\t", irq[test->irqtype]);
		if (ret < 0)
			fprintf(stdout, "FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->get_irqtype) {
		ret = ioctl(fd, PCITEST_GET_IRQTYPE);
		fprintf(stdout, "GET IRQ TYPE:\t\t");
		if (ret < 0)
			fprintf(stdout, "FAILED\n");
		else
			fprintf(stdout, "%s\n", irq[ret]);
	}

	if (test->legacyirq) {
		ret = ioctl(fd, PCITEST_LEGACY_IRQ, 0);
		fprintf(stdout, "LEGACY IRQ:\t");
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->msinum > 0 && test->msinum <= 32) {
		ret = ioctl(fd, PCITEST_MSI, test->msinum);
		fprintf(stdout, "MSI%d:\t\t", test->msinum);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->msixnum > 0 && test->msixnum <= 2048) {
		ret = ioctl(fd, PCITEST_MSIX, test->msixnum);
		fprintf(stdout, "MSI-X%d:\t\t", test->msixnum);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->write) {
		if (test->dma == 0) {
			ret = ioctl(fd, PCITEST_WRITE, test->size);
		} else {
			ret = ioctl(fd, PCITEST_DMA_WRITE, test->size);
		}
		fprintf(stdout, "WRITE (%7ld bytes):\t\t", test->size);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->read) {
		if (test->dma == 0) {
			ret = ioctl(fd, PCITEST_READ, test->size);
		} else {
			ret = ioctl(fd, PCITEST_DMA_READ, test->size);
		}
		fprintf(stdout, "READ (%7ld bytes):\t\t", test->size);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	if (test->copy) {
		ret = ioctl(fd, PCITEST_COPY, test->size);
		fprintf(stdout, "COPY (%7ld bytes):\t\t", test->size);
		if (ret < 0)
			fprintf(stdout, "TEST FAILED\n");
		else
			fprintf(stdout, "%s\n", result[ret]);
	}

	fflush(stdout);
}
int main(int argc, char **argv)
{
	int c;
	struct pci_test *test;

	test = calloc(1, sizeof(*test));
	if (!test) {
		perror("Fail to allocate memory for pci_test\n");
		return -ENOMEM;
	}

	/* since '0' is a valid BAR number, initialize it to -1 */
	test->barnum = -1;

	/* set default size as 100KB */
	test->size = 0x19000;

	/* set default endpoint device */
	test->device = "/dev/pci-endpoint-test.0";

	while ((c = getopt(argc, argv, "D:b:m:x:i:Ilr:w:cs:")) != EOF)
	switch (c) {
	case 'D':
		test->device = optarg;
		continue;
	case 'b':
		test->barnum = atoi(optarg);
		if (test->barnum < 0 || test->barnum > 5)
			goto usage;
		continue;
	case 'l':
		test->legacyirq = true;
		continue;
	case 'm':
		test->msinum = atoi(optarg);
		if (test->msinum < 1 || test->msinum > 32)
			goto usage;
		continue;
	case 'x':
		test->msixnum = atoi(optarg);
		if (test->msixnum < 1 || test->msixnum > 2048)
			goto usage;
		continue;
	case 'i':
		test->irqtype = atoi(optarg);
		if (test->irqtype < 0 || test->irqtype > 2)
			goto usage;
		test->set_irqtype = true;
		continue;
	case 'I':
		test->get_irqtype = true;
		continue;
	case 'r':
		test->read = true;
		test->dma = atoi(optarg);
		continue;
	case 'w':
		test->write = true;
		test->dma = atoi(optarg);
		continue;
	case 'c':
		test->copy = true;
		continue;
	case 's':
		test->size = strtoul(optarg, NULL, 0);
		continue;
	case '?':
	case 'h':
	default:
usage:
		fprintf(stderr,
			"usage: %s [options]\n"
			"Options:\n"
			"\t-D <dev>		PCI endpoint test device {default: /dev/pci-endpoint-test.0}\n"
			"\t-b <bar num>		BAR test (bar number between 0..5)\n"
			"\t-m <msi num>		MSI test (msi number between 1..32)\n"
			"\t-x <msix num>	\tMSI-X test (msix number between 1..2048)\n"
			"\t-i <irq type>	\tSet IRQ type (0 - Legacy, 1 - MSI, 2 - MSI-X)\n"
			"\t-I			Get current IRQ type configured\n"
			"\t-l			Legacy IRQ test\n"
			"\t-r <dma>		Read buffer test\n"
			"\t-w <dma>		Write buffer test\n"
			"\t-c			Copy buffer test\n"
			"\t-s <size>		Size of buffer {default: 100KB}\n",
			argv[0]);
		return -EINVAL;
	}

	run_test(test);
	return 0;
}

#else
#define READ_WRITE_SIZE 256
int main(int argc, char **argv)
{
	int ret,i;
    char writeBuf[READ_WRITE_SIZE];
    char ReadBuf[READ_WRITE_SIZE];
    GV_PCI_TRANSMIT_S pci_st; //定义数据传输的结构体
    pci_st.dma = 0;           //是否使用DMA 传输
    pci_st.size = READ_WRITE_SIZE;//传输数据的长度

    //将 writeBuf：全部设置成0x55; 将 ReadBuf 全部设置成0x00.
    memset(writeBuf,0x55, sizeof(writeBuf));
    memset(ReadBuf,0x00, sizeof(ReadBuf));
	
	/*
	* 1. 打开 RC 设备,  为后面file_operation 提供设备文件
	*/
	ret = gv_host_pci_rc_init();
    if(ret < 0 )
    {
        return -1;
    }
    pci_st.ptr = writeBuf;// 写入的 ptr 指向的地址
	/*
	* 2. ioctl(pci_rc_fd, PCITEST_DMA_WRITE, pst_pci_st);
	*     发送 COMMAND_DMA_READ
	*/
    if(gv_host_pci_rc_write(&pci_st) < 0)
        printf("pci_pci_rc_write err\n");
	
	/*
	* 3. 将 ptr 指向 ReadBuf
	*/
    pci_st.ptr = ReadBuf;
	/* ioctl(pci_rc_fd, PCITEST_DMA_READ, pst_pci_st);
    *  发送 COMMAND_DMA_WRITE
	*/
	if(gv_host_pci_rc_read(&pci_st) < 0)
        printf("pci_pci_rc_read err\n");
	
	
    for(i=0; i<100; i++)
    {
        printf("0x%02x ", ReadBuf[i]);
        if((i+1)%10 == 0)
            printf("\n");
    }

    gv_host_pci_rc_deinit();

    return 0;
}

#endif

