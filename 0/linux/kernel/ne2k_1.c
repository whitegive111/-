#define NE_DATAPORT 0x10
#define NE_IOBASE 0xc020
#define NE_IRQ 11

#define NE_PAGE_SIZE 256 //NE2000内存缓存每一页的大小是256B
#define NE_TXBUF_SIZE 4 //发送缓存尺寸(以页page为单位)
#define NE_TX_BUFERS 2 //发送缓存数量

#define NE_CR_STP 0x01 //关闭网卡，不允许收发数据包
#define NE_CR_STA 0x02 //开启网卡，允许收发数据包
#define NE_CR_RD2 0x20 //终止远程DMA
#define NE_P0_CR        0x00      //COMMAND 寄存器端口地址偏移
#define NE_P0_PSTART    0x01      //PSTART 寄存器端口地址偏移
#define NE_P0_PSTOP     0x02      //PSTOP 寄存器端口地址偏移
#define NE_P0_BNRY      0x03      //BNRY 寄存器端口地址偏移
#define NE_CR_PAGE1 0x40    //将PS0位设置为1 
#define NE_P1_CURR 0x07     //CURR 寄存器端口地址偏移

#define NE_P1_PAR0 0x01 //MAC地址第一个字节的端口
#define NE_P1_MAR0 0x08 //多播地址第一个字节的端口
#define NE_P0_RCR 0x0C
#define NE_RCR_AB 0x04 //允许接收广播数据包

#define NE_P0_DCR 0x0E //数据配置寄存器DCR
#define NE_DCR_WTS 0x01 //以字为单位
/*要配合抓包工具观察收到的数据包
  在实际场合中很可能需要发送时将一个字中的两个字节的顺序对调*/
#define NE_DCR_BOS 0x02 //小端字节顺序
#define NE_P0_RBCR0 0x0A //Remote Byte Count（低字节）
#define NE_P0_RBCR1 0x0B //Remote Byte Count（高字节）

//网卡管理数据结构定义
struct macaddr {
    unsigned char bytes[6];
};
struct ne {
unsigned short iobase;
unsigned short irq;
unsigned short membase; //缓存区的首地址
unsigned short memsize; //缓存区的大小
unsigned char rx_page_start; //发送缓冲区的开始页号
unsigned char rx_page_stop; //发送缓冲区的结束页号
unsigned char next_packet; //下一个未读的接收到的数据包
struct macaddr paddr; //网卡的MAC地址，将来收发送数据包时要用到
};

struct ne ne2k; //一个全局数据结构记录NE2000的基本信息

ne2k_init()
{
    int i;
    ne2k.iobase = NE_IOBASE;
    ne2k.irq = NE_IRQ;
    ne2k.membase = 16*1024;//BUFFER RAM (包括接收缓存和发送缓存)的开始位置,0x4000,16KB处 
    ne2k.memsize = 16*1024; //BUFFER RAM 的大小
    ne2k.rx_page_start = ne2k.membase/NE_PAGE_SIZE;
    //接收缓存开始位置
    ne2k.rx_page_stop = ne2k.rx_page_start + ne2k.memsize/NE_PAGE_SIZE- NE_TXBUF_SIZE * NE_TX_BUFERS;
    //接收缓存结束位置
    ne2k.next_packet = ne2k.rx_page_start + 1;
    //下一个未读的数据包在BNDY+1 处
    ne2k.paddr.bytes[0]=0x00;
    ne2k.paddr.bytes[1]=0x0c;
    ne2k.paddr.bytes[2]=0x29;
    ne2k.paddr.bytes[3]=0x07;
    ne2k.paddr.bytes[4]=0xf8;
    ne2k.paddr.bytes[5]=0xbd;

    //现在BUFFER_RAM已经分割好，接下来开始设置寄存器页上的各种寄存器
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //设置寄存器页0上的COMMAND寄存器，选择并准备写寄存器页0
    outb(ne2k.rx_page_start, ne2k.iobase+NE_P0_PSTART);
    //设置PSTART
    outb(ne2k.rx_page_stop, ne2k.iobase+NE_P0_PSTOP);
    //设置PSTOP
    outb(ne2k.rx_page_start, ne2k.iobase+NE_P0_BNRY);
    //设置BNRY
    
    outb(NE_CR_PAGE1|NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //设置寄存器页0上的COMMAND寄存器，选择并准备写寄存器页1
    outb(ne2k.next_packet, ne2k.iobase+NE_P1_CURR);
    //设置CURR寄存器
    for(i=0;i<6;i++)
        outb(ne2k.paddr.bytes[i], ne2k.iobase + NE_P1_PAR0 + i);
        //i=0..5
    for(i=0;i<8;i++)
        outb(0, ne2k.iobase + NE_P1_MAR0 + i);
        //i=0..7
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //设置寄存器页1上的COMMAND寄存器，选择并准备写寄存器页0
    outb(NE_RCR_AB, ne2k.iobase + NE_P0_RCR); 
    //允许广播
    outb(NE_DCR_WTS,ne2k.iobase+NE_P0_DCR);
    //以字为单位收发，按小端序来放置字节
    outb(0,ne2k.iobase+NE_P0_RBCR0);
    outb(0,ne2k.iobase+NE_P0_RBCR1);
    outb(NE_CR_RD2|NE_CR_STA,ne2k.iobase+NE_P0_CR);
}
