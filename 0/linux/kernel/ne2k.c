#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fdreg.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <signal.h>
#include <linux/ne2k.h>

extern void ne2k_interrupt(void);
//网卡初始化
void ne2k_init(void)
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
    
    for(i=0;i<6;i++){
        ne2k.paddr.bytes[i]=inb(NE_DATAPORT+ne2k.iobase);
        printk("%x\n",ne2k.paddr.bytes[i]);
    }
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
        //i=0..5，将MAC地址保存在寄存器页1对应的端口
    for(i=0;i<8;i++)
        outb(0, ne2k.iobase + NE_P1_MAR0 + i);
        //i=0..7，设置多播地址，对于本实验无影响，于是置0
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //设置寄存器页1上的COMMAND寄存器，选择并准备写寄存器页0
    outb(NE_RCR_AB, ne2k.iobase + NE_P0_RCR); 
    //允许广播
    outb(NE_DCR_WTS,ne2k.iobase+NE_P0_DCR);
    //以字为单位收发，按小端序来放置字节

    //将远程DMA的计数值初始化为0，告诉网卡现在不需要访问主存，启动网卡收发数据包
    outb(0,ne2k.iobase+NE_P0_RBCR0);
    outb(0,ne2k.iobase+NE_P0_RBCR1);
    outb(NE_CR_RD2|NE_CR_STA,ne2k.iobase+NE_P0_CR);

    set_trap_gate(0x2b, &ne2k_interrupt); //中断入口设置
    outb_p(inb_p(0x21) & 0xFB, 0x21);
    outb_p(inb_p(0xa1) & 0x7F, 0xA1); //发送OCW将屏蔽打开

    outb(NE_IMR_PRXE, ne2k.iobase + NE_P0_IMR);//允许数据包接收中断的触发

}
//ARP封装
void arphdr_alloc(arphdr_t *arp){
    int i;
    arp->htype=1;//表示硬件网络为以太网物理帧
    arp->ptype=0x0800;//表示这是一个IP协议
    arp->hlen=6;//硬件地址为6B
    arp->plen=4;//IP地址位4B
    arp->oper=1;//表示这是一个ARP请求数据包
    for(i=0;i<ETHER_ADDR_LEN;i++)
        arp->sha[i]=ne2k.paddr.bytes[i]; //发送端硬件地址
    arp->spa[0]=192;
    arp->spa[1]=168;
    arp->spa[2]=1;
    arp->spa[3]=11;
    for(i=0;i<ETHER_ADDR_LEN;i++)
        arp->tha[i]=0xFF; //目标端硬件地址
    arp->tpa[0]=192;
    arp->tpa[1]=168;
    arp->tpa[2]=140;
    arp->tpa[3]=128;
}

void etherhdr_alloc(etherhdr_t *eth){
    int i;
    for(i=0;i<ETHER_ADDR_LEN;i++)
        eth->dhost[i]=0xFF;//表示这是一个以太网广播帧
    for(i=0;i<ETHER_ADDR_LEN;i++)
        eth->shost[i]=ne2k.paddr.bytes[i]; //以太网源地址
    eth->ethertype=0x0806;//表示物理帧封装了一个ARP数据包
}
struct pbuf* pbuf_alloc(void){
    char *c=(char *)malloc(sizeof(struct pbuf)+sizeof(etherhdr_t)+sizeof(arphdr_t));
    struct pbuf *p=(struct pbuf*)c;
    p->len=sizeof(etherhdr_t)+sizeof(arphdr_t);
    p->tot_len=p->len;
    p->next=NULL;
    etherhdr_t *eth=(etherhdr_t*)(c+sizeof(struct pbuf));
    etherhdr_alloc(eth);
    arphdr_t *arp=(arphdr_t*)(c+sizeof(struct pbuf)+sizeof(etherhdr_t));
    arphdr_alloc(arp);
    p->payload=(void*)eth;
    return p;
}
//网卡发送物理帧的代码实现（第一步：将物理帧放入网卡的发送缓冲区）
int ne2k_transmit(struct pbuf *p)
{
    unsigned short packetlen = p->len;
    //ARP数据包的以太网物理帧长度为54个字节
    unsigned char status;
    int i;
    outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
    //首先终止远程DMA
    outb((unsigned char) packetlen, ne2k.iobase + NE_P0_RBCR0);
    outb((unsigned char) (packetlen >> 8), ne2k.iobase +NE_P0_RBCR1);
    //记录要远程写的内容长度
    unsigned short dst = ne2k.rx_page_stop;//发送缓冲区在接收缓冲区后
    //发送缓存区的开始地址
    outb((unsigned char) (dst * NE_PAGE_SIZE), ne2k.iobase + NE_P0_RSAR0);
    outb((unsigned char) ((dst * NE_PAGE_SIZE) >> 8), ne2k.iobase + NE_P0_RSAR1);
    //记录要远程写的目标地址
    outb(NE_CR_RD1 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
    //进行远程写
    unsigned short *payload=(char *)(p->payload);
    for(i=0;i<packetlen/2;i++) outw(*(payload+i), ne2k.iobase + NE_DATAPORT);
    //向数据端口写入的每个字会自动放置到BUFFER RAM中以dst开始且长度为packetlen的一段储存区域
    while((status = inb(ne2k.iobase + NE_P0_ISR) & 0x40) == 0);

    //网卡发送物理帧的代码实现（第S二步：给网卡发送数据传输命令
    outb((unsigned char) dst, ne2k.iobase + NE_P0_TPSR);
    if (packetlen > 54) {
        outb(packetlen, ne2k.iobase + NE_P0_TBCR0);
        outb(packetlen >> 8,ne2k.iobase + NE_P0_TBCR1);
    }
    else {
        outb(54, ne2k.iobase + NE_P0_TBCR0);
        outb(0, ne2k.iobase + NE_P0_TBCR1);
    }
    outb(NE_CR_RD2 | NE_CR_TXP | NE_CR_STA, ne2k.iobase + NE_P0_CR);//发送数据包
}

//接收数据包
void ne2k_handler(void)
{
    unsigned char status;
	outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);//选寄存器页0
	while ((status = inb(ne2k.iobase + NE_P0_ISR)) != 0) {
		outb(status, ne2k.iobase + NE_P0_ISR);
		if (status & NE_ISR_PRX) {
			ne2k_receive();
		}
	}
}

void ne2k_receive(void)
{
	struct recv_ring_desc packet_hdr;
	unsigned short packet_ptr;
	unsigned short len;
	unsigned char bndry;
    unsigned char *pbuf;
	outb(NE_CR_PAGE1 | NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
	//因为NE_P1_CURR在寄存器页1上
	while (ne2k.next_packet != inb(ne2k.iobase + NE_P1_CURR))
	{
		packet_ptr = ne2k.next_packet * NE_PAGE_SIZE;
        //利用数据端口从packet_ptr地址处读出4个字节到packet_hdr中
		len = packet_hdr.recv_count - sizeof(struct recv_ring_desc);
		pbuf = malloc(len);
		packet_ptr += sizeof(struct recv_ring_desc);
        //真正的数据包内容利用数据端口从packet_ptr地址处读出packetlen/2个字到pbuf中；
		ne2k.next_packet = packet_hdr.next_packet;
		outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
            //改回页0，因为NE_PO_BNRY在page 0上
		bndry = ne2k.next_packet - 1;
		if (bndry < ne2k.rx_page_start) bndry = ne2k.rx_page_stop - 1;
		outb(bndry, ne2k.iobase + NE_P0_BNRY);
        
        outb(NE_CR_PAGE1 | NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
        //再改回到页1，因为NE_P1_CURR在页1上
	}
}