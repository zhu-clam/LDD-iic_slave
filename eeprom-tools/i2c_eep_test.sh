#!/bin/sh
t=2
max_len=32

for i in `seq 1 $max_len`
do
    test_e2p -A -i 1 -d 0x50 -m 0 -l $i -p 32 -t $t
done
