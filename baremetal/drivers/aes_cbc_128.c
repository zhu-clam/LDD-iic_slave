//vector0

//#include "clib.h"
#include "ck810.h"
#include "spacc.h"
#include "misc.h"




int i_key[] = {
      0xae781dad,
      0x18efd7f1,
      0x4acc012a,
      0x75dbeb3a,
};

int i_iv[] = {
      0x6fac1192,
      0xeb4540ff,
      0xcb5afd0f,
      0xb408d5c5,
};

int plain[] = {
      0x5d1d5a36,
      0xa86380db,
      0x7905a5b2,
      0x6afaccd3,
      0x44e77625,
      0xa4a61e36,
      0x4fcb9ebc,
      0x84448cff,
      0xdc0131fc,
      0x8a248223,
      0x87211ff7,
      0x11b71f4b,
      0x73c7cfba,
      0xc0831484,
      0x6a6a2775,
      0x07ec6258,
      0x805177f1,
      0x7d30dd83,
      0x6597bb2a,
      0x58171693,
      0xd9e01310,
      0x4df4a881,
      0xff3449cf,
      0x56b96f68,
      0xf0b03e14,
      0x6b00166d,
      0x4cabd2e0,
      0xdb503a9a,
      0xf107c7c1,
      0x8ba69d3d,
      0xb3c625f3,
      0x6f451d59,
      0x77387ec3,
      0x4574d32b,
      0x57eccc74,
      0xa4dc34da,
      0x1f851b64,
      0xe6756fbf,
      0x03aae328,
      0x1f3a5d0c,
      0x8f4b5ba3,
      0xd22becb0,
      0x6373f25e,
      0xb780a155,
      0xbf4612be,
      0x37e306d4,
      0x9d3fce52,
      0xe2d6d119,
      0xaa6e38c7,
      0xd95d787a,
      0x45a93b9d,
      0x73349bbb,
      0x02f97736,
      0xfbd177e7,
      0x31ef0eaa,
      0x39d5ac2a,
      0x5a76772e,
      0x80d66393,
      0xde4898b8,
      0xe97f737c,
      0x1b933b90,
      0x2aea509b,
      0xcf5d0da2,
      0x11c947f4,
      0x6ee88a43,
      0x5d7f66e0,
      0x45f500fe,
      0x957d1230,
      0xc03f20dd,
      0x5013ae59,
      0xc469f3e0,
      0x544fab22,
      0xb30727eb,
      0xf8d6b1fe,
      0xc3accdd8,
      0xfd35d9c1,
      0x99725373,
      0xf400954e,
      0x6893975b,
      0xdf2f2519,
      0x8b8c647b,
      0xe385139b,
      0xb5c2df66,
      0x27cc13ab,
      0x34fa2d24,
      0xd8c75c58,
      0xb01907b7,
      0xdf0068b7,
      0x76c657d6,
      0x97d32c6f,
      0x6c8b8272,
      0x6104cc5d,
      0x47fbf646,
      0x0be89f56,
      0xc9d61437,
      0x2c02b81d,
      0x19febe78,
      0x71b15ae4,
      0x2a4835df,
      0x40fe6c36,
      0x94656a60,
      0xf6d434c4,
      0x605c862d,
      0x19d3affc,
      0x3554118f,
      0xedd0fcfa,
      0xf1c3aa94,
      0x3d1c7135,
      0x4a8a568a,
      0xf0c4f0a6,
      0x32c114c8,
      0x091cfb95,
};

int f_key[] = {
      0xcd91db6e,
      0x12d95753,
      0x1abdb3df,
      0x4f711e0c,
};

int f_iv[] = {
      0xa445879a,
      0xb30f2817,
      0x2ed0deed,
      0xaf0c73c2,
};


int CK_SPACC_Test (void)
{
    int result;
    int tmp;
    int k;
    int i;
    int case_fail;

    //CTRL Register -- [31]: end   [30]: begin   [29]: key_exp
    int vl_en_ctrl = 0xe000e012;   //CTRL Register   (begin,end=1,1)
    int vl_de_ctrl = 0xe0006012;
    int vl_src_len = 0x000001c0;   //Source packet length
    int vl_dst_len = 0x000001c0;   //Destination packet length
    int vl_icv_len = 0x00000000;   //ICV Length
    int vl_icv_offset = 0x00000000;   //ICV Offset
    int vl_offset = 0x00000000;   //{dst_offset[15:0], src_offset[15:0]}
    int vl_pre_aad_len = 0x00000000;   //PRE_AAD_LEN
    int vl_proc_len = 0x000001c0;   //PROC_LEN
    int vl_post_aad_len = 0x00000000;   //POST_AAD_LEN
    int vl_iv_offset = 0x0000020;   //IV_OFFSET
    int vl_aux = 0x00000000;   //AUX Register

    //Key size information.
    int vl_cipher_key_sz = 0x80000010;   //Cipher
    int vl_hash_key_sz = 0x00000000;   //Hash

    case_fail = 0;
///////////////////////////////////////////////////////////////
    printf("# Wr Plain to SRAM #\n");

    for(i=0;i<112;i=i+1){
       write_mreg32(PLAIN_SRAM_BASE+i*0x4,plain[i]);  
    }

//Encrypt Flow
    printf("# Encrypt Flow #\n");

    //printf("# Wr KEY IV #\n");
    for(i=0;i<4;i=i+1){
       write_mreg32(CIPH_CTX+i*0x4,i_key[i]);  
    }
    for(i=0;i<4;i=i+1){
       write_mreg32(CIPH_CTX+0x20+i*0x4,i_iv[i]);  
    }


    //printf("# Conf SRC DDT #\n");
    write_mreg32(SRC_PTR_ADDR+0x0,PLAIN_SRAM_BASE);
    write_mreg32(SRC_PTR_ADDR+0x4,vl_src_len);
    write_mreg32(SRC_PTR_ADDR+0x8,0x0);
    write_mreg32(SRC_PTR_ADDR+0xc,0x0);

    //printf("# Conf DST DDT #\n");
    write_mreg32(DST_PTR_ADDR+0x0,CIPH_SRAM_BASE);
    write_mreg32(DST_PTR_ADDR+0x4,vl_dst_len);
    write_mreg32(DST_PTR_ADDR+0x8,0x0);
    write_mreg32(DST_PTR_ADDR+0xc,0x0);


    //printf("# Conf reg #\n");
    write_mreg32(SRC_PTR,SRC_PTR_ADDR);
    write_mreg32(DST_PTR,DST_PTR_ADDR);
    write_mreg32(OFFSET,vl_offset);
    write_mreg32(PRE_AAD_LEN,vl_pre_aad_len);
    write_mreg32(POST_AAD_LEN,vl_post_aad_len);
    write_mreg32(PROC_LEN,vl_proc_len);
    write_mreg32(ICV_LEN,vl_icv_len);
    write_mreg32(ICV_OFFSET,vl_icv_offset);
    write_mreg32(IV_OFFSET,vl_iv_offset);
    write_mreg32(AUX_INFO,vl_aux);
    write_mreg32(KEY_SZ,vl_cipher_key_sz);
    write_mreg32(KEY_SZ,vl_hash_key_sz);

    //write CTRL reg
    printf("# Wait for Wr Ctrl Reg #\n");
    result = read_mreg32(FIFO_STAT);
    while ((result&0x8000) != 0x0){
      result = read_mreg32(FIFO_STAT);
    }
    write_mreg32(CTRL,vl_en_ctrl);

    //poll package process complete
    printf("# Wait Encrypt End #\n");
    result = read_mreg32(FIFO_STAT);
    while ((result&0x80000000) != 0x0){
      result = read_mreg32(FIFO_STAT);
    }

    write_mreg32(STAT_POP,0x1);
    printf("# Rd Stat reg #\n");
    result = read_mreg32(STATUS);
    if((result&0x07000000) == 0x0){
        printf("# Encrypt end! #\n");

     } 
    else {
           printf("Encrypt FAIL! \n");
           case_fail = 1;
    }


//Decrypt Flow
    printf("# Decrypt Flow #\n");

    //printf("# Wr KEY IV #\n");
    for(i=0;i<4;i=i+1){
       write_mreg32(CIPH_CTX+i*0x4,i_key[i]);  
    }
    for(i=0;i<4;i=i+1){
       write_mreg32(CIPH_CTX+0x20+i*0x4,i_iv[i]);  
    }


    //printf("# Conf SRC DDT #\n");
    write_mreg32(SRC_PTR_ADDR+0x0,CIPH_SRAM_BASE);
    write_mreg32(SRC_PTR_ADDR+0x4,vl_src_len);
    write_mreg32(SRC_PTR_ADDR+0x8,0x0);
    write_mreg32(SRC_PTR_ADDR+0xc,0x0);

    //printf("# Conf DST DDT #\n");
    write_mreg32(DST_PTR_ADDR+0x0,DECYP_SRAM_BASE);
    write_mreg32(DST_PTR_ADDR+0x4,vl_dst_len);
    write_mreg32(DST_PTR_ADDR+0x8,0x0);
    write_mreg32(DST_PTR_ADDR+0xc,0x0);


    //printf("# Conf reg #\n");
    write_mreg32(SRC_PTR,SRC_PTR_ADDR);
    write_mreg32(DST_PTR,DST_PTR_ADDR);
    write_mreg32(OFFSET,vl_offset);
    write_mreg32(PRE_AAD_LEN,vl_pre_aad_len);
    write_mreg32(POST_AAD_LEN,vl_post_aad_len);
    write_mreg32(PROC_LEN,vl_proc_len);
    write_mreg32(ICV_LEN,vl_icv_len);
    write_mreg32(ICV_OFFSET,vl_icv_offset);
    write_mreg32(IV_OFFSET,vl_iv_offset);
    write_mreg32(AUX_INFO,vl_aux);
    write_mreg32(KEY_SZ,vl_cipher_key_sz);
    write_mreg32(KEY_SZ,vl_hash_key_sz);

    //write CTRL reg
    printf("# Wait for Wr Ctrl Reg #\n");
    result = read_mreg32(FIFO_STAT);
    while ((result&0x8000) != 0x0){
      result = read_mreg32(FIFO_STAT);
    }
    write_mreg32(CTRL,vl_de_ctrl);

    //poll package process complete
    printf("# Wait Decrypt End #\n");
    result = read_mreg32(FIFO_STAT);
    while ((result&0x80000000) != 0x0){
      result = read_mreg32(FIFO_STAT);
    }


    write_mreg32(STAT_POP,0x1);
    //printf("# Rd Stat reg #\n");
    result = read_mreg32(STATUS);
    if((result&0x07000000) == 0x0){
        printf("# Decrypt end! #\n");

     } 
    else {
           printf("Decrypt FAIL! \n");
           case_fail = 1;
    }



/////////////////////////////////////////////////////////////
////check the data 
    printf("# check data #\n");

    for(i=0;i<112;i=i+1){

    tmp = read_mreg32(DECYP_SRAM_BASE+i*0x4);

    if(tmp==plain[i]){

     } 
    else {
           printf("data compare is FAIL! \n");
           case_fail = 1;
    }
    }

    for(i=0;i<4;i=i+1){

    tmp = read_mreg32(CIPH_CTX+i*0x4);

    if(tmp==f_key[i]){

     } 
    else {
           printf("key compare is FAIL! \n");
           case_fail = 1;
    }
    }

    for(i=0;i<4;i=i+1){

    tmp = read_mreg32(CIPH_CTX+0x20+i*0x4);

    if(tmp==f_iv[i]){

     } 
    else {
           printf("IV compare is FAIL! \n");
           case_fail = 1;
    }
    }


////////////////////////////////////////////////////////
    if (case_fail)
        printf("case is failed! \n");
    else
        printf("case is passed! \n");

//case_pass;

}
