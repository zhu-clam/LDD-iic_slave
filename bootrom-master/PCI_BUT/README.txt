Run BUT tool, you can get the help:
Usage: 
            ./but  bus_number  read  [addr] 
            ./but  bus_number  write  [addr] [val]
            ./but  bus_number  download  [addr]  [file]
            ./but  bus_number  dump  [addr]  [file]  [len]
            ./but  bus_number  go  [addr]  [cpu:ck810/ck860/unicore]
for example:
sudo ./but 1 download 0xf0000000 u-boot-spl.bin
sudo ./but 1 download 0x17a00000 u-boot.bin
sudo ./but 1 download 0x10000000 uImage
sudo ./but 1 download 0xf000000 boya_ck860-min.dtb
sudo ./but 1 go 0xf0000180 ck860

