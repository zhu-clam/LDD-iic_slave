/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 *
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */

#include "uart.h"
#include "datatype.h"
#include <string.h>

#define MAX_STRING_ITEM_NUM     4
#define MAX_STRING_ITEM_LEN     12
#define MAX_CMD_STRING_SIZE     64
extern void putc(char c);
extern char getc(void);
extern int puts(const char *ptr);
extern int load_serial_bin_and_check(u32 offset);
extern int printf(const char *format,...);

static void getcmd(char *string)
{
    char *string2 = string;
    char c;

    do {
        c = getc();
        if (c == 0x0D) { // enter
            //getc();//for next char 0x0A
            break;
        } else if ((c == 0x08) ||
                   (c == 0x1B) ||
                   (c == 0x7F)) { // backspace esc del
            if ((int)string2 < (int)string) {
                putc('\b');
                putc(' ');
                putc('\b');
                string--;
            }
        } else {
            if ((string - string2) < (MAX_CMD_STRING_SIZE - 1)) {
                *string = c;
                string++;
                putc(c);
            }
        }
    } while (1);

    *string = '\0';
    puts("\n");
}

static int cmd_string_parse(char *cmd_string,
                            char cmd_string_item[][MAX_STRING_ITEM_LEN])
{
    int i_num;
    int s_pos = 0, i_pos = 0;
    char c;

    for (i_num = 0; i_num < MAX_STRING_ITEM_NUM;) {
        c = cmd_string[s_pos++];
        if (c == ' ' || c == '\t') { //parameters are gapped with space and tab
            if (i_pos == 0)
                continue;

            cmd_string_item[i_num][i_pos] = '\0';
            i_pos = 0;
            i_num++;
        } else if (c == '\0') { // the string end
            cmd_string_item[i_num][i_pos] = '\0';
            break;
        } else {
            cmd_string_item[i_num][i_pos] = c;
            i_pos++;
        }
    }
    return (i_pos == 0) ? (i_num) : (i_num + 1);
}

static int char_to_int(unsigned char hex_data)
{
    int ret=0;

    if (hex_data >= '0' && hex_data <= '9')
        ret = hex_data - '0';
    else if (hex_data >= 'A' && hex_data <= 'F')
        ret = hex_data - 'A' + 10;
    else if (hex_data >= 'a' && hex_data <= 'f')
        ret = hex_data - 'a' + 10;

    return ret;
}

static unsigned int cmd_str2hex(char *str)
{
    int hex_mode = 0;
    int pos = 0;
    unsigned int data_ret = 0;
    unsigned char c_data;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        hex_mode = 1;
        pos = 2;
    }

    while (1) {
        if (str[pos] == '\0')
            return data_ret;

        c_data = char_to_int(str[pos++]);
        data_ret = data_ret * (hex_mode ? 16 : 10) + c_data;
    }
}

static void uart_console_help(void)
{
    printf("Command arguments: addr should be hex or decimal\n");
    printf(" read  addr\n");
    printf(" write addr val\n");
    printf(" dump addr len\n");
    printf(" download addr \n");
    printf(" go addr\n");
    printf(" ? or help\n");
    // debug(" exit\n");
}

static int cmd_string_execute(char cmd_string_item[][MAX_STRING_ITEM_LEN],
                              int cmd_string_item_num)
{
    LOAD_ENTRY enter_jump_func;
    u32 addr;
    if( strcmp( cmd_string_item[0], "read" ) == 0)
    {
        u32 reg_data;
        u32 reg_addr;
        if(cmd_string_item_num == 2)
        {
            reg_addr = cmd_str2hex( cmd_string_item[1] );
            reg_data = read_mreg32(reg_addr);
            printf("    0x%x \n", reg_data );
        }else
        {
            uart_console_help();
        }
    }
    else if( strcmp( cmd_string_item[0], "write" ) == 0 )
    {
        u32 reg_data;
        u32 reg_addr;
        if(cmd_string_item_num == 3)
        {
            reg_addr = cmd_str2hex( cmd_string_item[1] );
            reg_data = cmd_str2hex( cmd_string_item[2] );
            write_mreg32(reg_addr,reg_data);
        }else
        {
            uart_console_help();
        }
    }
    else if( strcmp( cmd_string_item[0], "dump" ) == 0 )///// dump
    {
        if(cmd_string_item_num == 3)
        {
            u32 mem_addr = cmd_str2hex( cmd_string_item[1] );
            u32 mem_num  = cmd_str2hex( cmd_string_item[2] );
            u32 mem_data;
            int i;
            printf("    ");
            for( i=0; i<mem_num; i++ )
            {
                printf("0x%02x ",read_mreg8(mem_addr+i));
                if((i+1)%16 == 0)
                    printf("\n    ");
            }
            printf("\n");
        }
        else
        {
            uart_console_help();
        }
    }
    else if (strcmp(cmd_string_item[0], "download") == 0) {
        if (cmd_string_item_num == 2) {
            addr = cmd_str2hex(cmd_string_item[1]);
            load_serial_bin_and_check(addr);
        } else {
            uart_console_help();
        }
    } else if (strcmp( cmd_string_item[0], "go") == 0) {
        if(cmd_string_item_num == 2) {
            addr = cmd_str2hex(cmd_string_item[1]);
            enter_jump_func = (LOAD_ENTRY)addr;
            enter_jump_func();
        } else {
            uart_console_help();
        }
    } else if (strcmp(cmd_string_item[0], "?") == 0 ||
               strcmp(cmd_string_item[0], "help") == 0) {
        uart_console_help();
    } else {
        //uart_puts( "Command error.\n" );
        uart_console_help();
    }

    return 0;
}
void uart_console(void)
{
    char cmd_string[MAX_CMD_STRING_SIZE];
    char cmd_string_item[MAX_STRING_ITEM_NUM][MAX_STRING_ITEM_LEN];
    int cmd_string_item_num;
    int ret;

    puts("Welcome to uart console ...\n>");

    while (1) {
        memset(&cmd_string, 0x00, sizeof(cmd_string));
        memset(&cmd_string_item, 0x00, sizeof(cmd_string_item));

        getcmd(cmd_string);
        cmd_string_item_num = cmd_string_parse(cmd_string, cmd_string_item);
        ret = cmd_string_execute(cmd_string_item, cmd_string_item_num);
        if (ret) {
            puts("Exit from uart console.\n>");
            break;
        } else {
            putc('>');
        }
    }
}
u32 uart_get_boot_mode(void)
{
    int boot_type = 0;
    char c;
    while(1){

        printf("\nplease input boot module ID:\n");
        printf("0 -- spi nor flash\n");
        printf("1 -- spi nand flash page size is:2K+64\n");
        printf("2 -- spi nand flash page size is:2K+128\n");
        //printf("3 -- spi nand flash page size is:4k+218\n");
        printf("4 -- spi nand flash page size is:4K+256\n");
        printf("5 -- sdio\n");
        printf("6 -- nand flash page size is:2K+64\n");
        //printf("7 -- nand flash page size is:2K+128\n");
        printf("8 -- nand flash page size is:4k+218\n");
        //printf("9 -- nand flash page size is:4K+256\n");
        printf("a -- nand flash page size is:8K+448\n");
        printf("b -- uart\n");
        printf("c -- pcie\n");
        printf(">");

        c = getc();
        putc(c);
        if (c >= '0' && c <= '9') {
            boot_type = c - '0';
            break;
        }
        if (c >= 'a' && c <= 'c') {
            boot_type = c - 'a' + 10;
            break;
        }

    }

    return boot_type;
}
