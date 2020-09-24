#########################################################################
# File Name: start.sh
# Author: ma6174
# mail: ma6174@163.com
# Created Time: Wed 21 Nov 2018 11:05:46 PM PST
#########################################################################
#!/bin/bash
echo "bring up linux via csky jtag"
#/home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-gdb -x gdb.init
#/home/zhuxf/tool-chain/bin/csky-abiv2-linux-gdb -x gdb.init
/home/zhuxf/tool-chain/bin/csky-linux-gnuabiv2-gdb -x gdb.init
