#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define PCI_CONFIG_ADDR(bus, dev, fn, reg) (0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3))
off_t remap_target_addrs(int pci_bus, uint32_t target_addrs);

typedef enum {
    CPU_CK860,
    CPU_CK810,
    CPU_UNICORE
}CPU_ID;

void usage()
{
    printf( "Usage: \n\
            ./but  bus_number  read  [addr] \n\
            ./but  bus_number  write  [addr] [val]\n\
            ./but  bus_number  download  [addr]  [file]\n\
            ./but  bus_number  dump  [addr]  [file]  [len]\n\
            ./but  bus_number  go  [addr]  [cpu:ck810/ck860/unicore]\n\ ");
    exit(-1);
}

unsigned long get_file_size(const char *filename)
{
    struct stat buf;
    if(stat(filename, &buf) < 0) {
        return 0;
    }
    return (unsigned long)buf.st_size;
}
void atu_writel(uint32_t d, void * addr)
{
    *((uint32_t *)addr) = d;
}



int but_read_write(int pci_bus, uint32_t target_addrs, int write, unsigned long writeval, size_t len)
{
    int fd;
    int page_size = getpagesize();
    off_t host_addrs;
    void *map_base, *virt_addr;
    unsigned long read_result;
    host_addrs = remap_target_addrs(pci_bus, target_addrs);

    if (0 == host_addrs)
    {
        perror ("target_addrs error\n");
        return -1;
    }
    if((fd = open ("/dev/mem", O_RDWR)) < 0)
    {
        perror ("open error\n");
        return -1;
    }
    map_base = (char *)mmap (0,
                             page_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd,
                             host_addrs & ~(page_size - 1));

    if (map_base == MAP_FAILED)
    {
        perror ("mmap error:\n");
        close(fd);
        return -1;
    }
    virt_addr = map_base + (host_addrs & (page_size - 1));
    read_result = *((uint32_t *) virt_addr);
    printf("Value at address 0x%llx (%p): 0x%lx\n", target_addrs, virt_addr, read_result);

    if (write)
    {
        *((uint32_t *) virt_addr) = writeval;
        read_result = *((uint32_t *) virt_addr);
        printf("Written 0x%lx; readback 0x%lx\n", writeval, read_result);
    }

    munmap(map_base, page_size);
    close(fd);
    return 0;
}

int but_download(int pci_bus, uint32_t target_addrs, char * file_name)
{
    int file_size;
    FILE *file_in = NULL;
    unsigned char *file_buff = NULL;
    int fd;
    int page_size = getpagesize();
    size_t map_size;
    off_t host_addrs;
    void *map_base, *virt_addr;
    int ret;
    host_addrs = remap_target_addrs(pci_bus, target_addrs);
    if (0 == host_addrs)
    {
        perror ("target_addrs error\n");
        return -1;
    }
    if((fd = open ("/dev/mem", O_RDWR)) < 0)
    {
        perror ("open error\n");
        return -1;
    }
    file_size = get_file_size(file_name);
    map_size = file_size & ~(page_size - 1);
    if (map_size < file_size)
        map_size += page_size;
    printf("%s file_size %d, map_size %ld\n", __FUNCTION__, file_size, map_size);
    map_base = (char *)mmap (0,
                             map_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd,
                             host_addrs & ~(page_size - 1));

    if (map_base == MAP_FAILED)
    {
        perror ("mmap error:\n");
        goto error3;
    }
    virt_addr = map_base + (host_addrs & (page_size - 1));

    file_in = fopen(file_name, "rb+");
    if(!file_in) {
        perror("open file_in failed:\n");
        goto error2;
    }

    file_buff = malloc(file_size);
    if (!file_buff) {
        perror("open file failed:\n");
        goto error1;
    }

    if (fread(file_buff, 1, file_size, file_in) != file_size) {
        perror("read input file failed.\n");
            // exit(1);
    }

    memcpy(virt_addr, file_buff, file_size);
    ret = memcmp(virt_addr, file_buff, file_size);
    if (ret)
        perror("download verify failed \n");
    else
        printf("download verify succeed \n", __FUNCTION__, file_size, map_size);

    free(file_buff);
    fclose(file_in);
    munmap(map_base, map_size);
    close(fd);
    return ret;

error1:
    fclose(file_in);
error2:
    munmap(map_base, map_size);
error3:
    close(fd);
    return -1;
}

int but_dump(int pci_bus, uint32_t target_addrs, char * file_name, size_t dump_len)
{
    FILE *file_out = NULL;
    int fd;
    int page_size = getpagesize();
    off_t host_addrs;
    size_t map_size;
    void *map_base, *virt_addr;

    map_size = dump_len & ~(page_size - 1);
    if (map_size < dump_len)
        map_size += page_size;

    printf("%s file_size %d, map_size %ld\n", __FUNCTION__, dump_len, map_size);

    host_addrs = remap_target_addrs(pci_bus, target_addrs);
    if (0 == host_addrs)
    {
        perror ("target_addrs error\n");
        return -1;
    }
    if((fd = open ("/dev/mem", O_RDWR)) < 0)
    {
        perror ("open error\n");
        return -1;
    }
    map_base = (char *)mmap (0,
                             map_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd,
                             host_addrs & ~(page_size - 1));

    if (map_base == MAP_FAILED)
    {
        perror ("mmap error:\n");
        goto error2;
    }
    virt_addr = map_base + (host_addrs & (page_size - 1));

    file_out = fopen(file_name, "wb+");
    if(!file_out) {
        perror("open file_out failed:\n");
        goto error1;
    }

    fwrite(virt_addr, 1, dump_len, file_out);

    fclose(file_out);
    munmap(map_base, map_size);
    close(fd);
    return 0;

error1:
    munmap(map_base, map_size);
error2:
    close(fd);
    return -1;
}
int but_go(int pci_bus, CPU_ID cpu, uint32_t go_addrs)
{
    but_read_write(pci_bus, 0xf97010a0, 1, go_addrs, 4);

    switch (cpu)
    {
        case CPU_CK860:
            but_read_write(pci_bus, 0xf9701000, 1, 0xa501, 4);//ck860
            break;
        case CPU_CK810:
            but_read_write(pci_bus, 0xf9701004, 1, 0xa501, 4);//ck810
            break;
        case CPU_UNICORE:
            but_read_write(pci_bus, 0xf9701008, 1, 0xa501, 4);//unicore
            break;
        default : return -1;
    }

    return 0;
}

#define XILINX_PCIE_ID  0x801410ee
#define SNSY_PCIE_ID    0xabcd16c3
#define BRA_SIZE        0x8000000

int device_is_match(int pci_bus)
{
    iopl(3);
    uint32_t ID;
    outl(PCI_CONFIG_ADDR(pci_bus, 0, 0, 0x0 ), 0xCF8);
    ID = inl(0xCFC);
    printf("device id:0x%x \n",ID);
    if (SNSY_PCIE_ID != ID)
    {
        printf("device id not match ,invalid bus id\n");
        return 0;
    }
    return 1;
}

off_t remap_target_addrs(int pci_bus, uint32_t target_addrs)
{
    int i;
    uint32_t bar_base[6];
    off_t host_addrs = 0;
    int fd;
    off_t atu_reg_addrs = 0;
    void *atu_map_base;
    uint32_t target_map_base, offset;

    for (i = 0; i < 6; i++)
    {
        outl(PCI_CONFIG_ADDR(pci_bus, 0, 0, 0x10 + i*4 ), 0xCF8);
        bar_base[i] = inl(0xCFC) & 0xfffffff0;
        printf("bar_base %d 0x%x\n", i, bar_base[i]);
    }

    atu_reg_addrs = bar_base[0] + 0x2000;

    if((fd = open ("/dev/mem", O_RDWR)) < 0)
    {
        perror ("open error\n");
        return -1;
    }
    atu_map_base = (char *)mmap (0,
                             0x4000,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd,
                             atu_reg_addrs);

    if (atu_map_base == MAP_FAILED)
    {
        perror ("mmap error:\n");
        close(fd);
        return -1;
    }
    target_map_base = target_addrs & ~(uint32_t)(BRA_SIZE - 1);
    offset = target_addrs & (uint32_t)(BRA_SIZE - 1);
    atu_writel(0x0, atu_map_base + 0x304);
    atu_writel(target_map_base, atu_map_base + 0x314);
    atu_writel(0x0, atu_map_base + 0x308);
    atu_writel(0x0, atu_map_base + 0x30c);
    atu_writel(0x0, atu_map_base + 0x300);
    atu_writel(0xc0000200, atu_map_base + 0x304);

    host_addrs = offset + bar_base[2];
    return host_addrs;
}

int main(int argc, char **argv)
{
    int bus = 1;
    int value = 0;
    int len;
    int target;//addr base on polaris soc

    if(argc < 4) {
        usage();
        exit(1);
    }

    bus = strtoul(argv[1], 0, 0);

    if (!device_is_match(bus))
    {
        exit(1);
    }

    target = strtoul(argv[3], 0, 0);
    printf("target 0x%x\n", target );

    printf("host addrs 0x%lx\n", remap_target_addrs(bus, target));

    if (!strcmp("read", argv[2]))
    {
        printf("but read \n" );
        return but_read_write(bus, target, 0, 0, 1);

    }

    if(argc < 5)
    {
        usage();
        exit(1);
    }
    if (!strcmp("go", argv[2]))
    {
        printf("but go \n" );
        if (!strcmp("ck860", argv[4]))
        {
            return but_go(bus, CPU_CK860, target);
        }
        else if (!strcmp("ck810", argv[4]))
        {
            return but_go(bus, CPU_CK810, target);
        }
        else if (!strcmp("unicore", argv[4]))
        {
            return but_go(bus, CPU_UNICORE, target);
        }
        usage();
        exit(1);
    }
    if (!strcmp("write", argv[2]))
    {
        value = strtoul(argv[4], 0, 0);
        printf("but write \n" );
        return but_read_write(bus, target, 1, value, 1);

    }
    else if (!strcmp("download", argv[2]))
    {
        printf("but download \n" );
        return but_download(bus, target, argv[4]);

    }

    if(argc < 6)
    {
        usage();
        exit(1);
    }
    if (!strcmp("dump", argv[2]))
    {
        printf("but dump \n" );
        len = strtoul(argv[5], 0, 0);
        return but_dump(bus, target, argv[4], len);
    }

    return 0;
}
