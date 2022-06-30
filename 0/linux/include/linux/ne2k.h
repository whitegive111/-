#ifndef _NE2K_H
#define _NE2K_H

#define NE_DATAPORT 0x10
#define NE_IOBASE 0xc020
#define NE_IRQ 11

/** indicates this packet's data should be immediately passed to the application */
#define PBUF_FLAG_PUSH      0x01U //立即发送
/** indicates this is a custom pbuf: pbuf_free calls pbuf_custom->custom_free_function()
    when the last reference is released (plus custom PBUF_RAM cannot be trimmed) */
#define PBUF_FLAG_IS_CUSTOM 0x02U
/** indicates this pbuf is UDP multicast to be looped back */
#define PBUF_FLAG_MCASTLOOP 0x04U //udp多播返回
/** indicates this pbuf was received as link-level broadcast */
#define PBUF_FLAG_LLBCAST   0x08U //链路层的广播
/** indicates this pbuf was received as link-level multicast */
#define PBUF_FLAG_LLMCAST   0x10U //链路层的多播
/** indicates this pbuf includes a TCP FIN flag */
#define PBUF_FLAG_TCP_FIN   0x20U //tcp挥手标志

#define NE_PAGE_SIZE 256 //NE2000内存缓存每一页的大小是256B
#define NE_TXBUF_SIZE 4 //发送缓存尺寸(以页page为单位)
#define NE_TX_BUFERS 2 //发送缓存数量

#define NE_CR_STP 0x01 //关闭网卡，不允许收发数据包
#define NE_CR_STA 0x02 //开启网卡，允许收发数据包
#define NE_CR_TXP 0x04 //开始发送数据包
#define NE_CR_RD0 0x8 //远程DMA读
#define NE_CR_RD1 0x10 //远程DMA写
#define NE_CR_RD2 0x20 //终止远程DMA

#define NE_P0_CR        0x00      //COMMAND 寄存器端口地址偏移
#define NE_P0_PSTART    0x01      //PSTART 寄存器端口地址偏移
#define NE_P0_PSTOP     0x02      //PSTOP 寄存器端口地址偏移
#define NE_P0_BNRY      0x03      //BNRY 寄存器端口地址偏移
#define NE_P0_TPSR 0x04           //发送数据包开始页
#define NE_P0_TBCR0 0x05          //发送字节数(低8位)
#define NE_P0_TBCR1 0x06          //发送字节数(高8位)
#define NE_P0_ISR 0x07            //中断状态寄存器
#define NE_P0_RSAR0 0x08          //远程DMA的开始地址（低八位）
#define NE_P0_RSAR1 0x09          //远程DMA的开始地址（高八位）
#define NE_P0_RBCR0 0x0A          //远程DMA的大小（低八位）
#define NE_P0_RBCR1 0x0B           //远程DMA的大小（高八位）
#define NE_P0_RCR 0x0C
#define NE_P0_DCR 0x0E            //数据配置寄存器DCR
#define NE_P0_IMR 0x0F

#define NE_P1_CURR      0x07      //CURR 寄存器端口地址偏移
#define NE_P1_PAR0 0x01 //MAC地址第一个字节的端口
#define NE_P1_MAR0 0x08 //多播地址第一个字节的端口

#define NE_CR_PAGE0 0x00    //选择寄存器页0
#define NE_CR_PAGE1 0x40    //选择寄存器页1 

#define NE_RCR_AB 0x04 //允许接收广播数据包

#define NE_DCR_WTS 0x01 //以字为单位
/*要配合抓包工具观察收到的数据包
  在实际场合中很可能需要发送时将一个字中的两个字节的顺序对调*/
#define NE_DCR_BOS 0x02 //小端字节顺序

#define ETHER_ADDR_LEN 6 //硬件地址长度
#define LOGIC_ADDR_LEN 4 //IP地址长度

#define NE_IMR_PRXE 0x01 //打开数据包接收中断
#define NE_ISR_PRX 0x01 //接收到一个数据包
#define NE_ISR_RDC 0x40 //远程DMA结束以后会设置这个位

extern void ne2k_init(void);
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

//ARP数据包和以太网物理帧包头的数据结构定义
typedef struct arphdr {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char sha[ETHER_ADDR_LEN]; //发送端硬件地址
    unsigned char spa[LOGIC_ADDR_LEN]; //发送端IP地址
    unsigned char tha[ETHER_ADDR_LEN]; //目标端硬件地址
    unsigned char tpa[LOGIC_ADDR_LEN]; //目标端IP地址
}arphdr_t;

typedef struct etherhdr {
    unsigned char dhost[ETHER_ADDR_LEN];//以太网目标地址
    unsigned char shost[ETHER_ADDR_LEN];//以太网源地址
    unsigned short ethertype; //协议类型
}etherhdr_t;


/*
next是一个pbuf类型的指针，指向下一个pbuf，因为网络中的数据包可能很大，而pbuf能管理的数据包大小有限，就会采用链表的形式将所有的pbuf包连接起来，这样子才能完整描述一个数据包，这些连接起来的pbuf包会组成一个链表，称之为pbuf链表。
payload是一个指向数据区域的指针，指向该pbuf管理的数据区域起始地址，这里的数据区域可以是紧跟在pbuf结构体地址后面的RAM空间，也可以是ROM中的某个地址上，取决于pbuf的类型。
tot_len中记录的是当前pbuf及其后续pbuf所有数据的长度，例如如果当前pbuf是pbuf链表上第一个数据结构，那么tot_len就记录着整个pbuf链表中所有pbuf中数据的长度；如果当前pbuf是链表上最后一个数据结构，那就记录着当前pbuf的长度。
len表示当前pbuf中有效的数据长度。
type表示pbuf的类型，LwIP中有4种pbuf的类型，并且使用了一个枚举类型的数据结构定义他们。
flags不同协议层表示内容不同。
ref表示该pbuf被引用的次数，引用表示有其他指针指向当前pbuf，这里的指针可以是pbuf的next指针，也可以是其他任意形式的指针，初始化一个pbuf的时候，ref会被设置为1，因为该pbuf的地址一定会被返回一个指针变量，当有其他指针指向pbuf的时候，就必须调用相关函数将ref字段加1。
*/
struct pbuf {
  struct pbuf *next;	//指向下一个pbuf

  void *payload;  //指向buff中的真实数据

  unsigned short tot_len;  //该len与其后所有pbuf的len
  
  unsigned short len;  //payload中数据长度，不包括首部

  unsigned char type;

  unsigned char flags;

  unsigned short ref;  //buffer被引用次数，包括next

};

struct recv_ring_desc{
	unsigned char recv_status;//接收状态
	unsigned char next_packet;//指向下一个数据包的指针
	unsigned short recv_count;//接收到的数据包的长度（包括这4个字节）
};

struct ne ne2k; //一个全局数据结构记录NE2000的基本信息

#endif