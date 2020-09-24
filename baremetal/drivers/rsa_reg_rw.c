
#include "ck810.h"
#include "rsa.h"
#include "misc.h"



int CK_RSA_Reg_RW_Test (void)
{
    int result;
    int tmp;
    int k;
    int i;
    int case_fail;

    case_fail = 0;
    
    printf("--- Start RSA Register Read/Write test ---\n");

///////////////////////////////////////////////////////////////
    printf("\t# Rd default #\n");

    tmp = reg_readbk32(CTRL, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    tmp = reg_readbk32(STAT, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    tmp = reg_readbk32(IRQ_EN, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }


    printf("# Wr A0 #\n");

    for(i=0;i<128;i++){
        write_mreg32(PKA_REGION_A+0x4*i, i);
    }
    
    for(i=0;i<128;i++){
        tmp = read_mreg32(PKA_REGION_A+0x4*i);
        if (tmp != i) {
            case_fail = 1;
            printf("# A0 Address 0x%x value 0x%x != 0x%x\n", PKA_REGION_A+0x4*i,
                tmp, i);
        }            
    }

    printf("# Wr B0 #\n");

    for(i=0;i<128;i++){
        write_mreg32(PKA_REGION_B+0x4*i, i);
    }
    
    for(i=0;i<128;i++){
        tmp = read_mreg32(PKA_REGION_B+0x4*i);
        if (tmp != i) {
            case_fail = 1;
            printf("# B0 Address 0x%x value 0x%x != 0x%x\n", PKA_REGION_B+0x4*i,
                tmp, i);
        }            
    }

    printf("# Wr C0 #\n");

    for(i=0;i<128;i++){
        write_mreg32(PKA_REGION_C+0x4*i, i);
    }
    
    for(i=0;i<128;i++){
        tmp = read_mreg32(PKA_REGION_C+0x4*i);
        if (tmp != i) {
            case_fail = 1;
            printf("# C0 Address 0x%x value 0x%x != 0x%x\n", PKA_REGION_C+0x4*i,
                tmp, i);
        }            
    }
    
    printf("# Wr D0 #\n");

    for(i=0;i<128;i++){
        write_mreg32(PKA_REGION_D+0x4*i, i);
    }
    
    for(i=0;i<128;i++){
        tmp = read_mreg32(PKA_REGION_D+0x4*i);
        if (tmp != i) {
            case_fail = 1;
            printf("# D0 Address 0x%x value 0x%x != 0x%x\n", PKA_REGION_D+0x4*i,
                tmp, i);
        }            
    }
    
    printf("# Wr FW #\n");

    for(i=0;i<1024;i++){
        write_mreg32(PKA_FW+0x4*i, i);
    }
    
    for(i=0;i<1024;i++){
        tmp = read_mreg32(PKA_FW+0x4*i);
        if (tmp != i) {
            case_fail = 1;
            printf("# FW Address 0x%x value 0x%x != 0x%x\n", PKA_FW+0x4*i,
                tmp, i);
        }            
    }
////////////////////////////////////////////////////////
    if (case_fail)
        printf("\t\tcase is failed! \n");
    else
        printf("\t\tcase is passed! \n");

}
