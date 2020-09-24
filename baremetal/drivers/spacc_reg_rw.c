
#include "ck810.h"
#include "spacc.h"
#include "misc.h"



void CK_SPACC_Reg_RW_Test (void)
{
    int tmp;
    int case_fail;

    case_fail = 0;
    
    printf("--- Start SPAcc Register Read/Write test ---\n");

///////////////////////////////////////////////////////////////
    printf("\t# Rd default #\n");

    tmp = reg_readbk32(IRQ_EN, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    tmp = reg_readbk32(IRQ_STAT, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    tmp = reg_readbk32(FIFO_STAT, 0x80000000);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    tmp = reg_readbk32(STATUS, 0x0);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }


    printf("\t# RW reg #\n");

    write_mreg32(SRC_PTR, 0x12345670);
    tmp = reg_readbk32(SRC_PTR, 0x12345670);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(DST_PTR,0x89abcdef);
    tmp = reg_readbk32(DST_PTR, 0x89abcde8);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(PRE_AAD_LEN,0x6);
    tmp = reg_readbk32(PRE_AAD_LEN, 0x6);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(POST_AAD_LEN,0x3);
    tmp = reg_readbk32(POST_AAD_LEN, 0x3);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(ICV_OFFSET,0x9);
    tmp = reg_readbk32(ICV_OFFSET, 0x9);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    printf("\t# CIP context rw #\n");

    write_mreg32(CIPH_CTX,0x12345678);
    tmp = reg_readbk32(CIPH_CTX, 0x12345678);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(CIPH_CTX+0x200,0xedcba987);
    tmp = reg_readbk32(CIPH_CTX+0x200, 0xedcba987);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(CIPH_CTX+0x3fc,0x5a3c69a5);
    tmp = reg_readbk32(CIPH_CTX+0x3fc, 0x5a3c69a5);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    printf("\t# Hash context rw #\n");

    write_mreg32(HAS_CTX,0x12345678);
    tmp = reg_readbk32(HAS_CTX, 0x12345678);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(HAS_CTX+0x80,0x965a3ca5);
    tmp = reg_readbk32(HAS_CTX+0x80, 0x965a3ca5);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }

    write_mreg32(HAS_CTX+0xfc,0xedcba987);
    tmp = reg_readbk32(HAS_CTX+0xfc, 0xedcba987);
    if(tmp){

     } 
    else {
           case_fail = 1;
    }



////////////////////////////////////////////////////////
    if (case_fail)
        printf("\t\tcase is failed! \n");
    else
        printf("\t\tcase is passed! \n");

}
