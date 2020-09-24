/*
 * Filename: ttimer.c
 * Description: To test the timer driver program
 * Copyright:
 * Author:
 * date:
 */

#include "ck810.h"
#include "misc.h"
#include "timer.h"

extern void CK_Timer_User_Define_Test(void);
extern void CK_Timer_Free_Running_Test(void);

#define CK_TIMER_TEST_TIME          100000  /* 100, 2s */
#define CK_TIMER_TEST_CYCLES        3
#define CK_MAX_TEST_CYCLES          1
volatile CK_UINT32 test_count[TIMERID_MAX];


/*
 * Callback function for timer interrupt, set timer_flag.
 */
void CK_Timer_Handler(CK_UINT32 irq)
{
    CK_UINT32 i;

    printf("[%s:%d]\n", __FUNCTION__, __LINE__);
    for(i = 0; i < TIMERID_MAX; i++) {
        if (CK_Timer_ClearIrqFlag(i)) {
            test_count[i]++;
            printf("Clear interrupt, Timer #%d, cycle %d, current 0x%x\n",
                i, test_count[i], CK_Timer_CurrentValue(i));

            if (test_count[i] == CK_TIMER_TEST_CYCLES) {
                CK_Timer_Stop(i);
                CK_Timer_Close(i);
            }
        }
    }
}

/*
 * main function of timer test program.
 */
void CK_Timer_Test() {
    CK_Timer_User_Define_Test();
    CK_Timer_Free_Running_Test();
}

void CK_Timer_User_Define_Test()
{
    CK_UINT32 now;
    CK_UINT32 last;
    CK_UINT32 i;
    BOOL is_fast;

    printf("\n\tStart Timer User Define mode test. . . \n");
    CK_Timer_Init();
    for(i = 0; i < TIMERID_MAX; i++) {
        printf("\n    Testing Timer #%d\n", i);

        test_count[i] = 0;
        is_fast = TRUE;
        /* Open timer0 & timer1 as normal interrupt,
         * timer2 & timer3 as fast interrupt
         */
        if (i < 2)
            is_fast = FALSE;

        CK_Timer_Open(i, CK_Timer_Handler, CK_INTC_TIM0 + i, is_fast);
        CK_Timer_Start(i, CK_TIMER_TEST_TIME);

        last = CK_Timer_CurrentValue(i);
        delay(1);
        now = CK_Timer_CurrentValue(i);
        if (now == last) {
            printf("Timer #%d - - - is not working.\n", i);
            CK_Timer_Stop(i);
            CK_Timer_Close(i);
        } else {

            while(test_count[i] < CK_TIMER_TEST_CYCLES)
                delay(1);

            if (test_count[i] == CK_TIMER_TEST_CYCLES)
                printf("    Timer #%d - - - PASS\n", i);
            else
                printf("    Timer #%d - - - FAILURE\n", i);
        }
    }
}

void CK_Timer_Free_Running_Test()
{
    CK_UINT32 now;
    CK_UINT32 last;
    CK_UINT32 i;

    printf("\n\tStart Timer #0 Free-running mode test (this may take 4 minutes). . . \n");
    CK_Timer_Init();

    printf("\n    Testing Timer #0\n");

    test_count[0] = 0;

    CK_Timer_Open(0, CK_Timer_Handler, CK_INTC_TIM0, FALSE);
    CK_Timer_Start_Free_Running(0);

    last = CK_Timer_CurrentValue(0);
    delay(1);
    now = CK_Timer_CurrentValue(0);
    if (now == last) {
        printf("Timer #0 - - - is not working.\n");
        CK_Timer_Stop(0);
        CK_Timer_Close(0);
        return;
    }

    i = 0;
    while(test_count[0] < 2) {
        i++;
        
        //if ((i % 500) == 0)
        //    printf("CurrentCount=0x%x\n", CK_Timer_CurrentValue(0));
    }

    if (test_count[0] == 2)
        printf("    Timer #0 - - - PASS\n");
    else
        printf("    Timer #0 - - - FAILURE\n");
}
