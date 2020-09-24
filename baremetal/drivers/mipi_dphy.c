#include "datatype.h"
#include "ck810.h"
#include "mipi_subsys.h"
#include "OV_5640.h"

#define R_CSI2_DPHY_SHUTDOWNZ	MIPI_HOST_PHY_SHUTDOWNZ
#define R_CSI2_DPHY_RSTZ 		MIPI_HOST_DPHY_RSTZ
#define R_CSI2_DPHY_RX 		    MIPI_HOST_PHY_RX
#define	R_CSI2_DPHY_STOPSTATE 	MIPI_HOST_PHY_STOPSTATE
#define R_CSI2_DPHY_TST_CTRL0 	MIPI_HOST_PHY_TEST_CTRL0
#define R_CSI2_DPHY_TST_CTRL1 	MIPI_HOST_PHY_TEST_CTRL1
#define R_CSI2_DPHY2_TST_CTRL0 	MIPI_HOST_PHY2_TEST_CTRL0
#define R_CSI2_DPHY2_TST_CTRL1	MIPI_HOST_PHY2_TEST_CTRL1

#define ADDR_MIPI_PHY_CFGCLKFREQRANGE           (MIPI_IPI2AXI_BASE + 0x28)
#define ADDR_MIPI_PHY_HSFREQRANGE               (MIPI_IPI2AXI_BASE + 0x2C)


int hsfreqranges[63] = {
0x00,/*7'b0000000,*/ 
0x10,/*7'b0010000,*/
0x20,/*7'b0100000,*/
0x30,/*7'b0110000,*/
0x01,/*7'b0000001,*/
0x11,/*7'b0010001,*/
0x21,/*7'b0100001,*/
0x31,/*7'b0110001,*/
0x02,/*7'b0000010,*/
0x12,/*7'b0010010,*/
0x22,/*7'b0100010,*/
0x32,/*7'b0110010,*/
0x03,/*7'b0000011,*/
0x13,/*7'b0010011,*/
0x23,/*7'b0100011,*/
0x33,/*7'b0110011,*/
0x04,/*7'b0000100,*/
0x14,/*7'b0010100,*/
0x25,/*7'b0100101,*/
0x35,/*7'b0110101,*/
0x05,/*7'b0000101,*/
0x16,/*7'b0010110,*/
0x26,/*7'b0100110,*/
0x37,/*7'b0110111,*/
0x07,/*7'b0000111,*/
0x18,/*7'b0011000,*/
0x28,/*7'b0101000,*/
0x39,/*7'b0111001,*/
0x09,/*7'b0001001,*/
0x19,/*7'b0011001,*/
0x29,/*7'b0101001,*/
0x3a,/*7'b0111010,*/
0x0a,/*7'b0001010,*/
0x1a,/*7'b0011010,*/
0x2a,/*7'b0101010,*/
0x3b,/*7'b0111011,*/
0x0b,/*7'b0001011,*/
0x1b,/*7'b0011011,*/
0x2b,/*7'b0101011,*/
0x3c,/*7'b0111100,*/
0x0c,/*7'b0001100,*/
0x1c,/*7'b0011100,*/
0x2c,/*7'b0101100,*/
0x3d,/*7'b0111101,*/
0x0d,/*7'b0001101,*/
0x1d,/*7'b0011101,*/
0x2e,/*7'b0101110,*/
0x3e,/*7'b0111110,*/
0x0e,/*7'b0001110,*/
0x1e,/*7'b0011110,*/
0x2f,/*7'b0101111,*/
0x3f,/*7'b0111111,*/
0x0f,/*7'b0001111,*/
0x40,/*7'b1000000,*/
0x41,/*7'b1000001,*/
0x42,/*7'b1000010,*/
0x43,/*7'b1000011,*/
0x44,/*7'b1000100,*/
0x45,/*7'b1000101,*/
0x46,/*7'b1000110,*/
0x47,/*7'b1000111,*/
0x48,/*7'b1001000,*/
0x49 /*7'b1001001,*/
};


int hsfreqranges_ulim[63] = { 
82.5,92.5,102.5,112.5,
122.5,132.5,142.5,152.5,
162.5,172.5,182.5,192.5,
207.5,222.5,237.5,262.5,
287.5,312.5,337.5,375,
425,475,525,575,625,
675,725,775,825,875,
925,975,1025,1075,
1125,1175,1225,1275,
1325,1375,1425,1475,
1525,1575,1625,1675,
1725,1775,1825,1875,
1925,1975,2025,2075,
2125,2175,2225,2275,2325,
2375,2425,2475,2525
};

int tb_osc_freq_target[63] = {
16,18,19,21,23,25,27,29,31,33,34,36,
39,42,45,48,52,57,61,66,75,84,94,103,
112,121,130,139,148,156,165,174,183,
192,200,209,218,226,235,243,252,260,
269,277,285,294,302,310,318,327,335,
343,351,359,367,375,383,391,399,407,
415,422,430
};





static void snps_dphy_te_write(unsigned int code,unsigned char dat)
{

	unsigned char codeH = (code >> 8) &0x0F;
	unsigned char codeL = code & 0xFF ;
    write_mreg32(R_CSI2_DPHY_TST_CTRL0,0);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,0);
   udelay(100 * 1000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,(1L << 16)|0x00000000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL1,0x00000000|codeH);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,(1L << 16)|codeL);
	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0x00000000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,dat);
	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

}

static void snps_dphy_te_read(unsigned int code,unsigned char *dat)
{

	unsigned char codeH = (code >> 8) &0x0F;
	unsigned char codeL = code & 0xFF ;
    write_mreg32(R_CSI2_DPHY_TST_CTRL0,0);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,0);
    udelay(100 * 1000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,(1L << 16)|0x00000000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL1,0x00000000);

    write_mreg32(R_CSI2_DPHY_TST_CTRL1,codeH);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,(1L << 16)|codeL);
	write_mreg32(R_CSI2_DPHY_TST_CTRL0,(1L << 1)|0x00000000);

	write_mreg32(R_CSI2_DPHY_TST_CTRL0,0x00000000);
	write_mreg32(R_CSI2_DPHY_TST_CTRL1,0x00000000);

	*dat = read_mreg32(R_CSI2_DPHY_TST_CTRL1)>>8;

}


static void start_rst_dphy(void)
{

	write_mreg32(R_CSI2_DPHY_SHUTDOWNZ , 0x00000000);
    write_mreg32(R_CSI2_DPHY_RSTZ     , 0x00000000);
    write_mreg32(R_CSI2_DPHY_TST_CTRL0, 0x00000000);
    write_mreg32(R_CSI2_DPHY_TST_CTRL0, 0x00000001);
    write_mreg32(R_CSI2_DPHY_TST_CTRL0, 0x00000000);

}

void changefreqrange( int freq )
{
    int hsfreqrange;
    int osc_freq_target;
    int i;

    for ( i=0; i<63; i=i+1) {
        if ( freq <= hsfreqranges_ulim[i])
            break;
    }

    hsfreqrange = hsfreqranges[i];

    write_mreg32(ADDR_MIPI_PHY_CFGCLKFREQRANGE,0x0000001c);
    write_mreg32(ADDR_MIPI_PHY_HSFREQRANGE    ,hsfreqrange);

    switch(hsfreqrange) {
    case 0x00 /*7'b0000000*/  :  osc_freq_target =  tb_osc_freq_target [ 0]; break ;
    case 0x10 /*7'b0010000*/  :  osc_freq_target =  tb_osc_freq_target [ 1]; break ;
    case 0x20 /*7'b0100000*/  :  osc_freq_target =  tb_osc_freq_target [ 2]; break ;
    case 0x30 /*7'b0110000*/  :  osc_freq_target =  tb_osc_freq_target [ 3]; break ;
    case 0x01 /*7'b0000001*/  :  osc_freq_target =  tb_osc_freq_target [ 4]; break ;
    case 0x11 /*7'b0010001*/  :  osc_freq_target =  tb_osc_freq_target [ 5]; break ;
    case 0x21 /*7'b0100001*/  :  osc_freq_target =  tb_osc_freq_target [ 6]; break ;
    case 0x31 /*7'b0110001*/  :  osc_freq_target =  tb_osc_freq_target [ 7]; break ;
    case 0x02 /*7'b0000010*/  :  osc_freq_target =  tb_osc_freq_target [ 8]; break ;
    case 0x12 /*7'b0010010*/  :  osc_freq_target =  tb_osc_freq_target [ 9]; break ;
    case 0x22 /*7'b0100010*/  :  osc_freq_target =  tb_osc_freq_target [10]; break ;
    case 0x32 /*7'b0110010*/  :  osc_freq_target =  tb_osc_freq_target [11]; break ;
    case 0x03 /*7'b0000011*/  :  osc_freq_target =  tb_osc_freq_target [12]; break ;
    case 0x13 /*7'b0010011*/  :  osc_freq_target =  tb_osc_freq_target [13]; break ;
    case 0x23 /*7'b0100011*/  :  osc_freq_target =  tb_osc_freq_target [14]; break ;
    case 0x33 /*7'b0110011*/  :  osc_freq_target =  tb_osc_freq_target [15]; break ;
    case 0x04 /*7'b0000100*/  :  osc_freq_target =  tb_osc_freq_target [16]; break ;
    case 0x14 /*7'b0010100*/  :  osc_freq_target =  tb_osc_freq_target [17]; break ;
    case 0x25 /*7'b0100101*/  :  osc_freq_target =  tb_osc_freq_target [18]; break ;
    case 0x35 /*7'b0110101*/  :  osc_freq_target =  tb_osc_freq_target [19]; break ;
    case 0x05 /*7'b0000101*/  :  osc_freq_target =  tb_osc_freq_target [20]; break ;
    case 0x16 /*7'b0010110*/  :  osc_freq_target =  tb_osc_freq_target [21]; break ;
    case 0x26 /*7'b0100110*/  :  osc_freq_target =  tb_osc_freq_target [22]; break ;
    case 0x37 /*7'b0110111*/  :  osc_freq_target =  tb_osc_freq_target [23]; break ;
    case 0x07 /*7'b0000111*/  :  osc_freq_target =  tb_osc_freq_target [24]; break ;
    case 0x18 /*7'b0011000*/  :  osc_freq_target =  tb_osc_freq_target [25]; break ;
    case 0x28 /*7'b0101000*/  :  osc_freq_target =  tb_osc_freq_target [26]; break ;
    case 0x39 /*7'b0111001*/  :  osc_freq_target =  tb_osc_freq_target [27]; break ;
    case 0x09 /*7'b0001001*/  :  osc_freq_target =  tb_osc_freq_target [28]; break ;
    case 0x19 /*7'b0011001*/  :  osc_freq_target =  tb_osc_freq_target [29]; break ;
    case 0x29 /*7'b0101001*/  :  osc_freq_target =  tb_osc_freq_target [30]; break ;
    case 0x3a /*7'b0111010*/  :  osc_freq_target =  tb_osc_freq_target [31]; break ;
    case 0x0a /*7'b0001010*/  :  osc_freq_target =  tb_osc_freq_target [32]; break ;
    case 0x1a /*7'b0011010*/  :  osc_freq_target =  tb_osc_freq_target [33]; break ;
    case 0x2a /*7'b0101010*/  :  osc_freq_target =  tb_osc_freq_target [34]; break ;
    case 0x3b /*7'b0111011*/  :  osc_freq_target =  tb_osc_freq_target [35]; break ;
    case 0x0b /*7'b0001011*/  :  osc_freq_target =  tb_osc_freq_target [36]; break ;
    case 0x1b /*7'b0011011*/  :  osc_freq_target =  tb_osc_freq_target [37]; break ;
    case 0x2b /*7'b0101011*/  :  osc_freq_target =  tb_osc_freq_target [38]; break ;
    case 0x3c /*7'b0111100*/  :  osc_freq_target =  tb_osc_freq_target [39]; break ;
    case 0x0c /*7'b0001100*/  :  osc_freq_target =  tb_osc_freq_target [40]; break ;
    case 0x1c /*7'b0011100*/  :  osc_freq_target =  tb_osc_freq_target [41]; break ;
    case 0x2c /*7'b0101100*/  :  osc_freq_target =  tb_osc_freq_target [42]; break ;
    case 0x3d /*7'b0111101*/  :  osc_freq_target =  tb_osc_freq_target [43]; break ;
    case 0x0d /*7'b0001101*/  :  osc_freq_target =  tb_osc_freq_target [44]; break ;
    case 0x1d /*7'b0011101*/  :  osc_freq_target =  tb_osc_freq_target [45]; break ;
    case 0x2e /*7'b0101110*/  :  osc_freq_target =  tb_osc_freq_target [46]; break ;
    case 0x3e /*7'b0111110*/  :  osc_freq_target =  tb_osc_freq_target [47]; break ;
    case 0x0e /*7'b0001110*/  :  osc_freq_target =  tb_osc_freq_target [48]; break ;
    case 0x1e /*7'b0011110*/  :  osc_freq_target =  tb_osc_freq_target [49]; break ;
    case 0x2f /*7'b0101111*/  :  osc_freq_target =  tb_osc_freq_target [50]; break ;
    case 0x3f /*7'b0111111*/  :  osc_freq_target =  tb_osc_freq_target [51]; break ;
    case 0x0f /*7'b0001111*/  :  osc_freq_target =  tb_osc_freq_target [52]; break ;
    case 0x40 /*7'b1000000*/  :  osc_freq_target =  tb_osc_freq_target [53]; break ;
    case 0x41 /*7'b1000001*/  :  osc_freq_target =  tb_osc_freq_target [54]; break ;
    case 0x42 /*7'b1000010*/  :  osc_freq_target =  tb_osc_freq_target [55]; break ;
    case 0x43 /*7'b1000011*/  :  osc_freq_target =  tb_osc_freq_target [56]; break ;
    case 0x44 /*7'b1000100*/  :  osc_freq_target =  tb_osc_freq_target [57]; break ;
    case 0x45 /*7'b1000101*/  :  osc_freq_target =  tb_osc_freq_target [58]; break ;
    case 0x46 /*7'b1000110*/  :  osc_freq_target =  tb_osc_freq_target [59]; break ;
    case 0x47 /*7'b1000111*/  :  osc_freq_target =  tb_osc_freq_target [60]; break ;
    case 0x48 /*7'b1001000*/  :  osc_freq_target =  tb_osc_freq_target [61]; break ;
    case 0x49 /*7'b1001001*/  :  osc_freq_target =  tb_osc_freq_target [62]; break ;
    }

	snps_dphy_te_write(0xe2,(unsigned char)(osc_freq_target&0xFF));
    snps_dphy_te_write(0xe3,(unsigned char)(osc_freq_target >> 8));
	///snps_dphy_te_write(0xe4,0x01);
	snps_dphy_te_write(0xe4,0x11);
	snps_dphy_te_write(0xe5,0x01);
   

  
}

static void dphy_init(int freq)
{
	unsigned int val;
	printf("dphy_init  ----1 \r\n");	
	changefreqrange(freq);
	printf("dphy_init  ----2 \r\n");		
	snps_dphy_te_write(0x08,0x38);
	snps_dphy_te_read(0x08,&val);
	printf("dphy_init  0x08= 0x%x \r\n",val&0xFF);		
    write_mreg32(R_CSI2_DPHY_SHUTDOWNZ ,0x00000001);
    write_mreg32(R_CSI2_DPHY_RSTZ,0x00000001);
#if 0	
	do{
		val = read_mreg32(R_CSI2_DPHY_STOPSTATE);
		printf("val = 0x%x \r\n",val);
	}while(( val & 0x0001000f ) != 0x0001000f ); 
#else
     udelay(100*1000);
#endif	
	printf("dphy_init  ----4 \r\n");		
}

static void dphy_cal_check(int freq)
{
    int res_error;
    int nerrors;
    char testdout;
    char ddl_test_lanes;
    char offset_test_lanes;
    char skew_test_lanes;

    printf("[INFO] Preforming calibration check...\n"); 
    res_error =0;
    nerrors =0;

    // Resistor calibration

	snps_dphy_te_read(0x222,&testdout);
    if ( ( testdout & 0x01 ) == 0x01 ) {
        res_error = 1;
    }
    else {
        res_error = 0;
    }
    snps_dphy_te_read(0x221,&testdout);
    if ( (testdout & 0x80) == 0x00 ) {
        nerrors = nerrors + 1;
        printf("[ERROR] Resistor Calibration Failed\n");
    }
    else {
        if ( res_error == 0 ) {
            printf("[INFO] Resistor Calibration PASS\n");
        }
        else {
            printf("[ERROR] Resistor Calibration Failed\n");
        }
    }

    // Offset calibration
    offset_test_lanes = 0x0;
	snps_dphy_te_read(0x039d,&testdout);

    if ( ( ( testdout & 0x01 ) == 0x01 ) & ( ( testdout >>1 ) != 0x08 )) {
        offset_test_lanes = offset_test_lanes | 0x01;
    }
    else {
        offset_test_lanes = offset_test_lanes & 0xfe;
    }


	snps_dphy_te_read(0x059f,&testdout);

    //lane0 configured as RX
    if ( ( ( testdout & 0x02) == 0x02 ) & ( ( testdout & 0x04 ) == 0x00 ) ) {
        offset_test_lanes = offset_test_lanes | 0x02;
    }
    else {
        offset_test_lanes = offset_test_lanes & 0xfd;
    }


	snps_dphy_te_read(0x079f,&testdout);

    if ( ( ( testdout & 0x02) == 0x02 ) & ( ( testdout & 0x04 ) == 0x00 ) ) {
        offset_test_lanes = offset_test_lanes | 0x04;
    }
    else {
        offset_test_lanes = offset_test_lanes & 0xfb;
    }


	snps_dphy_te_read(0x099f,&testdout);

    if ( ( ( testdout & 0x02) == 0x02 ) & ( ( testdout & 0x04 ) == 0x00 ) ) {
        offset_test_lanes = offset_test_lanes | 0x08;
    }
    else {
        offset_test_lanes = offset_test_lanes & 0xf7;
    }

 
	snps_dphy_te_read(0x0b9f,&testdout);

    if ( ( ( testdout & 0x02) == 0x02 ) & ( ( testdout & 0x04 ) == 0x00 ) ) {
        offset_test_lanes = offset_test_lanes | 0x10;
    }
    else {
        offset_test_lanes = offset_test_lanes & 0xef;
    }


    if ( offset_test_lanes == 0x00 ) {
        nerrors = nerrors +1;
        printf("[ERROR] Offset calibration machine FAILED\n");
    }
    else {
        printf("[INFO] Offset calibration machine PASS\n");
    }

    //DDL calibration
  
	snps_dphy_te_read(0x05e0,&testdout);

    ddl_test_lanes =0;
    

    if ( ( testdout & 0x04 ) == 0x04 ) { //done flag asserted
        ddl_test_lanes = ddl_test_lanes | 0x01;
    }
    else {
        ddl_test_lanes = ddl_test_lanes & 0xfe;
    }


   snps_dphy_te_read(0x07e0,&testdout);

    if ( ( testdout & 0x04 ) == 0x04 ) { //done flag asserted
        ddl_test_lanes = ddl_test_lanes | 0x02;
    }
    else {
        ddl_test_lanes = ddl_test_lanes & 0xfd;
    }



	snps_dphy_te_read(0x09e0,&testdout);

    if ( ( testdout & 0x04 ) == 0x04 ) { //done flag asserted
        ddl_test_lanes = ddl_test_lanes | 0x04;
    }
    else {
        ddl_test_lanes = ddl_test_lanes & 0xfb;
    }

 

	snps_dphy_te_read(0x0be0,&testdout);

     if ( (testdout & 0x04) == 0x04 ) { //done flag asserted
        ddl_test_lanes = ddl_test_lanes | 0x08;
    }
    else {
        ddl_test_lanes = ddl_test_lanes & 0xf7;
    }

    if ( ddl_test_lanes ==0 ) {
        nerrors = nerrors + 1;
        printf("[ERROR] DDL calibration machine FAILED\n");
    }
    else {
        printf("[INFO] DDL calibration machine PASS\n");
    }
   
    //SKEW calibration
    if( freq <= 1500 ) {

		snps_dphy_te_read(0x052f,&testdout);
        skew_test_lanes = 0;

        if ( ( (testdout & 0x08) == 0x08 ) &  ( ( testdout & 0x10 ) == 0x00 )) { //done flag asserted + not failed
            skew_test_lanes = skew_test_lanes | 0x01;
        }
        else {
            skew_test_lanes = skew_test_lanes & 0xfe;
        }

		snps_dphy_te_read(0x072f,&testdout);

        if ( ( (testdout & 0x08) == 0x08 ) &  ( ( testdout & 0x10 ) == 0x00 )) { //done flag asserted + not failed
            skew_test_lanes = skew_test_lanes | 0x02;
        }
        else {
            skew_test_lanes = skew_test_lanes & 0xfd;
        }


  
		snps_dphy_te_read(0x092f,&testdout);

        if ( ( (testdout & 0x08) == 0x08 ) &  ( ( testdout & 0x10 ) == 0x00 )) { //done flag asserted + not failed
            skew_test_lanes = skew_test_lanes | 0x04;
        }
        else {
            skew_test_lanes = skew_test_lanes & 0xfb;
        }



		snps_dphy_te_read(0x0b2f,&testdout);

        if ( ( (testdout & 0x08) == 0x08 ) &  ( ( testdout & 0x10 ) == 0x00 )) { //done flag asserted + not failed
            skew_test_lanes = skew_test_lanes | 0x08;
        }
        else {
            skew_test_lanes = skew_test_lanes & 0xf7;
        }

        if ( skew_test_lanes ==0) {
            nerrors = nerrors + 1;
            printf("[ERR0R] SKEW calibration machine FAILED\n");
        }
        else {
            printf("[INFO] SKEW calibration machine PASS\n");
        }

    }
    else {

            printf("[INFO] SKEW calibration machine PASS : bypassed for datarates higher than 1500Mbps\n");
    }
    printf("[INFO] calibration check completed\n");

}

void mipi_dphy_initial(int freq)
{
	unsigned char val;
	printf(" mipi_dphy_initial  freq = %d \r\n ",freq);	
	start_rst_dphy();
	#if defined( __OV5640_800MBPS__)&& defined(__ISP_TEST__)
	snps_dphy_te_write(0xf1,0x90);
	#endif
	
	#if defined( __OV5640_800MBPS__)&& defined(__MIPI_TEST__)
	snps_dphy_te_write(0xf1,0x88);
	#endif
	
	
		
	dphy_init(freq);
	
	dphy_cal_check(freq);
	
	
	snps_dphy_te_read(0x7a,&val);
	
	
	printf("   0x7a  = 0x%x \r\n",val);
}