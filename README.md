# 扩展实验二
实现简单的网络功能（这实验巨多而且很坑）

1.先检验linux是否有tun/tap功能，如无，可尝试修改/etc/network/interfaces文件来配置bochs的IP地址（但不知道通过这种方法设置的IP地址能不能用），或者在网上照流程安装tuntap，可借鉴https://blog.csdn.net/lishuhuakai/article/details/70305543（但本人安装后版本不兼容用不了）。

2.首先修改conf/0.11.bxrc文件，如下
pci: enabled=1, chipset=i440fx,slot2=ne2k
ne2k:ioaddr=0x300,irq=10,mac=00:0c:29:07:f8:bd,ethmod=tuntap,ethdev=/dev/net/tun,script=/path/tunconfig
ne2k即是要实现的网卡功能，而ne2k对应的信息要根据个人电脑信息进行配置，具体可查阅《操作系统原理、实现与实践》-李治军 这本书。

3.进行网卡的初始化，代码主要在0/linux/kernel/中的ne2k.c中，并对相应的Makefile进行调整。
（后续由于没时间跑路了，去做鼠标驱动了，本项目代码仅留作参考，网卡初始化没有问题）
