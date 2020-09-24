#include "ck810.h"
#include "datatype.h"
#include "misc.h"
#include "intc.h"
#include "gpio.h"
#include "timer.h"

#define GPIO_SWPORTA_DR     0x00
#define GPIO_SWPORTA_DDR    0x04
#define GPIO_SWPORTB_DR     0x0c
#define GPIO_SWPORTB_DDR    0x10
#define GPIO_SWPORTC_DR     0x18
#define GPIO_SWPORTC_DDR    0x1c
#define GPIO_SWPORTD_DR     0x24
#define GPIO_SWPORTD_DDR    0x28
#define GPIO_INTEN      0x30
#define GPIO_INTMASK        0x34
#define GPIO_INTTYPE_LEVEL  0x38
#define GPIO_INT_POLARITY   0x3c
#define GPIO_INTSTATUS      0x40
#define GPIO_RAW_INTSTATUS  0x44
#define GPIO_PORTA_DEBOUNCE 0x48
#define GPIO_PORTA_EOI      0x4c
#define GPIO_EXT_PORTA      0x50
#define GPIO_EXT_PORTB      0x54
#define GPIO_EXT_PORTC      0x58
#define GPIO_EXT_PORTD      0x5c
#define GPIO_ID_CODE 0x64
#define GPIO_INT_BOTHEDGE 0x68
#define GPIO_VER_ID_CODE 0x6c
#define GPIO_CONFIG_REG1 0x74
#define GPIO_CONFIG_REG2 0x70

#define GPIO_INTC_ACTIVE_LOW  0
#define GPIO_INTC_ACTIVE_HIGH 1
#define GPIO_INTC_ACTIVE_BOTH 2

static inline unsigned int gpio_reg_read(unsigned int offset)
{
    return read_mreg32(CK_GPIO_ADDR + offset);
}

static inline void gpio_reg_write(unsigned int offset, unsigned int val)
{
    write_mreg32(CK_GPIO_ADDR + offset, val);
}

static inline void gpio_reg_set_bit(unsigned int offset, unsigned int bit) {
    gpio_reg_write(offset, gpio_reg_read(offset) | (0x1 << bit));
}

static inline void gpio_reg_clear_bit(unsigned int offset, unsigned int bit) {
    gpio_reg_write(offset, gpio_reg_read(offset) & (~(0x1 << bit)));
}

static inline unsigned int gpio_reg_read_bit(unsigned int offset, unsigned int bit) {
    return ((gpio_reg_read(offset) >> bit) & 0x1);
}

// gpio4 <--> FPGA_A_PB4 (push button on FPGA board)
static inline BOOL press_pb13() {
    return (CK_Gpio_Input(1) == 0) ;
}

void CK_Gpio_Init() {
    unsigned int config_reg1, config_reg2;

    printf ("[%s:%d], ID=0x%x\n", __FUNCTION__, __LINE__, gpio_reg_read(GPIO_ID_CODE));
    printf ("[%s:%d], VER=0x%x\n", __FUNCTION__, __LINE__, gpio_reg_read(GPIO_VER_ID_CODE));

    config_reg1 = gpio_reg_read(GPIO_CONFIG_REG1);
    printf ("[%s:%d], config1=0x%x\n", __FUNCTION__, __LINE__, config_reg1);
    printf ("[%s:%d], APB_DATA_WIDTH=%d\n", __FUNCTION__, __LINE__, 0x8 << (config_reg1 & 0x3));
    printf ("[%s:%d], NUM_PORTS=%d\n", __FUNCTION__, __LINE__, ((config_reg1 >> 2) & 0x3) + 1);

    config_reg2 = gpio_reg_read(GPIO_CONFIG_REG2);
    printf ("[%s:%d], config2=0x%x\n", __FUNCTION__, __LINE__, config_reg2);
    printf ("[%s:%d], PWIDTH_A=%d\n", __FUNCTION__, __LINE__, config_reg2 & 0x1f);
    printf ("[%s:%d], PWIDTH_B=%d\n", __FUNCTION__, __LINE__, (config_reg2 >> 5) & 0x1f);
    printf ("[%s:%d], PWIDTH_C=%d\n", __FUNCTION__, __LINE__, (config_reg2 >> 10) & 0x1f);
    printf ("[%s:%d], PWIDTH_D=%d\n", __FUNCTION__, __LINE__, (config_reg2 >> 15) & 0x1f);
}

void CK_Gpio_Output(unsigned int pin, unsigned char val) {
    // set pin for output
    gpio_reg_set_bit(GPIO_SWPORTA_DDR, pin);

    // write pin value
    if (val == 0) {
        gpio_reg_clear_bit(GPIO_SWPORTA_DR, pin);
    } else {
        gpio_reg_set_bit(GPIO_SWPORTA_DR, pin);
    }
}

void dw_Gpio_Output(unsigned int port, unsigned int pin, unsigned char val) {
    // set pin for output
    gpio_reg_set_bit(GPIO_SWPORTA_DDR + (port * 0xc), pin);

    // write pin value
    if (val == 0) {
        gpio_reg_clear_bit(GPIO_SWPORTA_DR + (port * 0xc), pin);
    } else {
        gpio_reg_set_bit(GPIO_SWPORTA_DR + (port * 0xc), pin);
    }
}

unsigned int dw_Gpio_Input(unsigned int port, unsigned int pin) {
    // set pin for input
    gpio_reg_clear_bit(GPIO_SWPORTA_DDR + (port * 0xc), pin);

    // get pin value
    return gpio_reg_read_bit(GPIO_EXT_PORTA + (port >> 2), pin);
}

// gpioA26 - A31 <--> d9 - D14 (led on EVB board)
void CK_Gpio_Test_Output() {
    int failed = 0;

    printf ("\n[%s:%d], -----GPIO output test begin-----\n", __FUNCTION__, __LINE__);
    printf ("[%s:%d], ***please check d9&d10 led status***", __FUNCTION__, __LINE__);

    CK_Gpio_Output(26, 1);
    printf ("\n\t Is d9 led on? [y/n] ");
    if (CK_WaitForReply() != 1) {
        failed = 1;
    }

    CK_Gpio_Output(26, 0);
    printf ("\n\t Is d9 led off? [y/n] ");
    if (CK_WaitForReply() != 1) {
        failed = 1;
    }

    CK_Gpio_Output(27, 1);
    printf ("\n\t Is d10 led on? [y/n] ");
    if (CK_WaitForReply() != 1) {
        failed = 1;
    }

    CK_Gpio_Output(27, 0);
    printf ("\n\t Is d10 led off? [y/n] ");
    if (CK_WaitForReply() != 1) {
        failed = 1;
    }

    if (failed == 1) {
        printf ("\n\t - - -FAILURE\n");
    } else {
        printf ("\n\t - - -PASS\n");
    }
    printf ("[%s:%d], -----GPIO output test end-----\n", __FUNCTION__, __LINE__);
}

unsigned int CK_Gpio_Input(unsigned int pin) {
    // set pin for input
    gpio_reg_clear_bit(GPIO_SWPORTA_DDR, pin);

    // get pin value
    return gpio_reg_read_bit(GPIO_EXT_PORTA, pin);
}

// gpioA16 <--> J13 PIN1 : input
// gpioA17 <--> J13 PIN3 : output

// gpioA18 <--> J13 PIN5 : input
// gpioA19 <--> J13 PIN7 : output
void CK_Gpio_Test_Input() {
    unsigned int gpio0, gpio1;
    unsigned int gpio0_old, gpio1_old;
    int passed = 0;

    printf ("\n[%s:%d], -----GPIO input test begin-----\n", __FUNCTION__, __LINE__);
    printf("For this test to run correctly, connect\n"
            " EVB board J13 PIN1 - EVB Board J13 PIN3,"
            " EVB board J13 PIN5 - EVB Board J13 PIN7!\n");
    printf("connection done, continue? - - - [y/n] ");
    while (CK_WaitForReply() != 1) {
        printf ("\n\t Start test? [y/n] ");
    }
    printf ("[%s:%d], ***crtl + c to exit test***\n", __FUNCTION__, __LINE__);
    CK_Gpio_Output(17, 1);
    CK_Gpio_Output(19, 1);

    gpio0 = CK_Gpio_Input(16);
    gpio1 = CK_Gpio_Input(18);
    printf ("[%s:%d], gpio0=%d\n", __FUNCTION__, __LINE__, gpio0);
    printf ("[%s:%d], gpio1=%d\n", __FUNCTION__, __LINE__, gpio1);
    gpio0_old = gpio0;
    gpio1_old = gpio1;

    CK_Gpio_Output(17, 0);
    CK_Gpio_Output(19, 0);
    while (1) {
        gpio0 = CK_Gpio_Input(16);
        gpio1 = CK_Gpio_Input(18);
        if (gpio0 != gpio0_old) {
            passed = 1;
            printf ("[%s:%d], gpio0=%d\n", __FUNCTION__, __LINE__, gpio0);
            gpio0_old = gpio0;
        }
        if (gpio1 != gpio1_old) {
            passed = 1;
            printf ("[%s:%d], gpio1=%d\n", __FUNCTION__, __LINE__, gpio1);
            gpio1_old = gpio1;
        }
        if (0x03 ==  getchar())
        //if (press_pb13())
        {
            if (passed == 0) {
                printf ("\t - - -FAILURE\n");
            } else {
                printf ("\t - - -PASS\n");
            }
            printf ("[%s:%d], -----GPIO input test end-----\n", __FUNCTION__, __LINE__);
            break;
        }
    }
}

static volatile unsigned int gpio_intc_count;

void CK_Gpio_Intc_Handler() {
    printf ("[%s:%d], intstatus=%d, raw_intstatus=%d\n", __FUNCTION__, __LINE__,
        gpio_reg_read(GPIO_INTSTATUS), gpio_reg_read(GPIO_RAW_INTSTATUS));
    printf ("[%s:%d], clear interrupt\n", __FUNCTION__, __LINE__);
    gpio_reg_write(GPIO_PORTA_EOI, 0xffffffff);
    printf ("[%s:%d], intstatus=%d, raw_intstatus=%d\n", __FUNCTION__, __LINE__,
        gpio_reg_read(GPIO_INTSTATUS), gpio_reg_read(GPIO_RAW_INTSTATUS));
    gpio_intc_count++;
}

static CKStruct_IRQHandler gpio_irq_info = {
    .devname = "GPIO",
    .irqid = CK_INTC_GPIO,
    .priority = CK_INTC_GPIO,
    .handler = CK_Gpio_Intc_Handler,
    .bfast = FALSE,
    .next = NULL
};

static void CK_Gpio_Intc_Set(unsigned int pin, BOOL level, int active) {
    printf ("\n[%s:%d], -----GPIO interrupt test begin-----\n", __FUNCTION__, __LINE__);

    // interrupt disable
    gpio_reg_clear_bit(GPIO_INTEN, pin);

    if (level) {
        gpio_reg_clear_bit(GPIO_INTTYPE_LEVEL, pin);
        printf ("[%s:%d], set level-sensitive\n", __FUNCTION__, __LINE__);
    } else {
        gpio_reg_set_bit(GPIO_INTTYPE_LEVEL, pin);
        printf ("[%s:%d], set edge-sensitive\n", __FUNCTION__, __LINE__);
    }

    if (!level && (active == GPIO_INTC_ACTIVE_BOTH)) {
        // enable bothedge
        printf ("[%s:%d], set both edge\n", __FUNCTION__, __LINE__);
        gpio_reg_set_bit(GPIO_INT_BOTHEDGE, pin);
    } else {
        if (active == GPIO_INTC_ACTIVE_LOW) {
            printf ("[%s:%d], set active-low\n", __FUNCTION__, __LINE__);
            gpio_reg_clear_bit(GPIO_INT_POLARITY, pin);
        } else if (active == GPIO_INTC_ACTIVE_HIGH) {
            printf ("[%s:%d], set active-high\n", __FUNCTION__, __LINE__);
            gpio_reg_set_bit(GPIO_INT_POLARITY, pin);
        }
    }

    // enable debounce
    gpio_reg_set_bit(GPIO_PORTA_DEBOUNCE, pin);
    // interrupt enable
    gpio_reg_set_bit(GPIO_INTEN, pin);
    // unmasked
    gpio_reg_clear_bit(GPIO_INTMASK, pin);

    // Clear out spurious interrupts
    gpio_reg_write(GPIO_PORTA_EOI, 0xffffffff);

    printf ("\t Start test? [y/n] ");
    while (CK_WaitForReply() != 1) {
        printf ("\n\t Start test? [y/n] ");
    }
    printf ("\n");

    // register irq handler
    CK_INTC_RequestIrq(&gpio_irq_info, AUTO_MODE);
    gpio_intc_count = 0;
}

static void CK_Gpio_Intc_Check(unsigned int pin, BOOL even) {
    int passed = 0;
    while (1) {
	CK_Gpio_Output(17, 1);
	timer_udelay(1000);
	CK_Gpio_Output(17, 0);
	timer_udelay(1000);
        if (gpio_intc_count > 3) {
            if (gpio_intc_count > 0) {
                if (even) {
                    if ((gpio_intc_count % 2) == 0) {
                        passed = 1;
                    }
                } else {
                    passed = 1;
                }
            }

            if (passed == 0) {
                printf ("\t - - -FAILURE\n");
            } else {
                printf ("\t - - -PASS\n");
            }
            printf ("[%s:%d], -----GPIO interrupt test end-----\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    // unregister irq handler
    CK_INTC_FreeIrq(&gpio_irq_info, AUTO_MODE);
}

// gpioA16 <--> J13 PIN1 : input
// gpioA17 <--> J13 PIN3 : output
void CK_Gpio_Test_Intc() {
#if 0
    // level, low
    CK_Gpio_Output(17, 1);
    CK_Gpio_Intc_Set(16, true, GPIO_INTC_ACTIVE_LOW);
    CK_Gpio_Intc_Check(16, false);

    // level, high
    CK_Gpio_Intc_Set(16, true, GPIO_INTC_ACTIVE_HIGH);
    CK_Gpio_Intc_Check(16, false);
#endif
    // edge, falling
    CK_Gpio_Intc_Set(16, false, GPIO_INTC_ACTIVE_LOW);
    CK_Gpio_Intc_Check(16, false);

    // edge, raising
    CK_Gpio_Intc_Set(16, false, GPIO_INTC_ACTIVE_HIGH);
    CK_Gpio_Intc_Check(16, false);

    // edge, both
    CK_Gpio_Intc_Set(16, false, GPIO_INTC_ACTIVE_BOTH);
    CK_Gpio_Intc_Check(16, true);
}

void CK_Gpio_Test() {
    CK_Gpio_Init();

    CK_Gpio_Test_Output();
    CK_Gpio_Test_Input();
    CK_Gpio_Test_Intc();
}

