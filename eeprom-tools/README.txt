This tool is used to test I2C access to EEPROM. And I2C depends on boya_ck860.dtb.
Test steps:
    1.  test_e2p -r -i 1 -d 0x50 -m 0 -l 8          //read 8 bytes data from I2C1 slave address 0x50
    2.  test_e2p -r -i 1 -d 0x50 -m 0 -l 6 -s 11 22 33 44 55 66 //write 8 bytes data to I2C1 slave address 0x50
    3.  test_e2p -r -i 1 -d 0x50 -m 0 -l 8          //read 8 bytes data from I2C1 slave address 0x50 and compare the results
And you can use scripts for coverage testing:
    ./i2c_eep_test.sh       //test with max_len=32 and repeat times = 2
If you want to know how to  use test_e2p, you could execute:
    test_e2p    //print usage of test_e2p