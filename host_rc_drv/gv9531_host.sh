#!/bin/bash
GV9531=`lspci|grep -E '001a|abcd'|grep -v grep|awk '{print $1}'`
GV9531_NUM=`echo $GV9531|wc -w`
let GV9531_NUM=$GV9531_NUM-1
if [ "$1" = "init" ];then
  Module=`lsmod | grep "pci_gv9531_host"`
  if [ -z "$Module" ];then
    ###make###
    make clean && make
    ###insmod###
    sudo insmod pci_gv9531_host.ko
    ###rescan###
    NUM=0
    sudo rm .rescan*
    for i in $GV9531
    do
      echo "#!/bin/sh" > .rescan_$NUM.sh
      echo "sudo sh -c 'echo 1 > /sys/bus/pci/devices/0000:$i/remove'" >> .rescan_$NUM.sh
      echo "sudo sh -c 'echo 1 > /sys/bus/pci/rescan'" >> .rescan_$NUM.sh
      echo "sudo chmod 666 /dev/pci-endpoint-test.$NUM" >> .rescan_$NUM.sh
      chmod u+x .rescan_$NUM.sh
      sudo sh .rescan_$NUM.sh
      let NUM=$NUM+1
    done
  
  
  #kill all
  sudo killall tun4gv9531_x
  ###tun###
  for id in `seq 0 $GV9531_NUM`
  do
    sleep 1
    sudo ./tun4gv9531_x 0 $id &
  done
  fi
elif [ "$1" = "update_linux" ];then
  if [ -z "$2" ];then
    echo "please input linux file"
    exit
  else
    LINUX_FILE=$2
    for id in `seq 0 $GV9531_NUM`
    do
      let ip=$[100+id]
      let hostip=$[id]
      sleep 1
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "ifconfig tun2host_0 mtu 32768"
      scp -o "StrictHostKeyChecking no" ${LINUX_FILE} root@10.254.10.$ip:/root/
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 511 > /sys/class/gpio/export && echo out > /sys/class/gpio/gpio511/direction && echo 0 > /sys/class/gpio/gpio511/value"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 0 > /sys/class/gpio/gpio511/value"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "flashcp -v ${LINUX_FILE} /dev/mtd0"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 1 > /sys/class/gpio/gpio511/value"
    done 
  fi
elif [ "$1" = "update_pci" ];then
  if [ -z "$2" ];then
    echo "please input linux file"
    exit
  else
    BOYA_FILE=$2
    for id in `seq 0 $GV9531_NUM`
    do
      let ip=$[100+id]
      let hostip=$[id]
      sleep 1
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "ifconfig tun2host_0 mtu 32768"
      scp -o "StrictHostKeyChecking no" ${BOYA_FILE} root@10.254.10.$ip:/root/
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 511 > /sys/class/gpio/export && echo out > /sys/class/gpio/gpio511/direction && echo 0 > /sys/class/gpio/gpio511/value"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 0 > /sys/class/gpio/gpio511/value"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "flashcp -v ${BOYA_FILE} /dev/mtd1"
      ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "echo 1 > /sys/class/gpio/gpio511/value"
	  
    done 
  fi  
elif [ "$1" = "reboot" ];then
	CHIP_NUM=0
	GV9531_WDT_EN=`ls .WDT_EN_* | wc -w`
	echo "there is $GV9531_WDT_EN number chip"
    	if [ $GV9531_WDT_EN = '0' ];then 
	#	echo reboot
		echo "reboot directly"
		sudo reboot
	else
	WDT_FILE=$(($GV9531_WDT_EN-1))
	for CHIP_NUM in `seq 0 $WDT_FILE`
	do
		`echo  1 > .WDT_EN_$CHIP_NUM`
	done
		echo reboot
		sleep 1
		sudo reboot
	fi
#reset all of chip	
elif [ "$1" = "resetall" ];then
	CHIP_NUM=0
	GV9531_WDT_EN=`ls .WDT_EN_* | wc -w`
	echo "there is $GV9531_WDT_EN number chip"

	WDT_FILE=$(($GV9531_WDT_EN-1))
	for CHIP_NUM in `seq 0 $WDT_FILE`
	do
		#`echo  1 > .WDT_EN_$CHIP_NUM`
		./gv-shell gv$CHIP_NUM sh /bin/reboot.sh
	done	
#end reset all of chip	
elif [ "$1" = "reset" ];then
     CHIP_ID=$2
     //PID_NUM=$(cat .pid_$CHIP_ID.sh)
	 PID_NUM=$(cat /tmp/pid_$CHIP_ID.sh)
     #"cat .pid_$CHIP_ID.sh > PID_NUM"
     echo The CHIP_ID $CHIP_ID PID_NUM is $PID_NUM
#     sudo kill -9 $PID_NUM   #kill special tun server app
#reset special chip_id`s chip
	`echo 1 > .WDT_EN_$CHIP_ID`
	sleep 1
	`echo 0 > .WDT_EN_$CHIP_ID`
	 sudo kill -9 $PID_NUM   #kill special tun server app
#	./gv-shell gv$CHIP_ID sh /root/boya_own/reboot.sh
#rescan ,execute special .rescan_id.sh 
elif [ "$1" = "rescan" ];then
     RESCAN_NUM=$2
     sudo sh .rescan_$RESCAN_NUM.sh

#restart ,launch specail tun server 
elif [ "$1" = "restart" ];then
     RESTART_NUM=$2
    sudo ./tun4gv9531_x 0 $RESTART_NUM & 

elif [ "$1" = "runapp" ];then
  for id in `seq 0 $GV9531_NUM`
  do
    {
     let ip=$[100+id]
     let hostip=$[id]
     sleep 1
     sudo mkdir /nfs_share/$hostip/
     sleep 1
     sudo cp -r /home/byavs/Pcie_Test/cvApp2pcie/ /nfs_share/$hostip/
     sleep 1
     ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "umount /mnt"
     ssh -o "StrictHostKeyChecking no" root@10.254.10.$ip "mount -t nfs -o nolock 10.254.10.$hostip:/nfs_share/$hostip/ /mnt"
    } &
  done
   wait
fi
