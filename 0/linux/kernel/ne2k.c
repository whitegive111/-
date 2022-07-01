#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fdreg.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <signal.h>
#include <linux/ne2k.h>

extern void ne2k_interrupt(void);
//������ʼ��
void ne2k_init(void)
{
    int i;
    ne2k.iobase = NE_IOBASE;
    ne2k.irq = NE_IRQ;
    ne2k.membase = 16*1024;//BUFFER RAM (�������ջ���ͷ��ͻ���)�Ŀ�ʼλ��,0x4000,16KB�� 
    ne2k.memsize = 16*1024; //BUFFER RAM �Ĵ�С
    ne2k.rx_page_start = ne2k.membase/NE_PAGE_SIZE;
    //���ջ��濪ʼλ��
    ne2k.rx_page_stop = ne2k.rx_page_start + ne2k.memsize/NE_PAGE_SIZE- NE_TXBUF_SIZE * NE_TX_BUFERS;
    //���ջ������λ��
    ne2k.next_packet = ne2k.rx_page_start + 1;
    //��һ��δ�������ݰ���BNDY+1 ��
    
    for(i=0;i<6;i++){
        ne2k.paddr.bytes[i]=inb(NE_DATAPORT+ne2k.iobase);
        printk("%x\n",ne2k.paddr.bytes[i]);
    }
    //����BUFFER_RAM�Ѿ��ָ�ã���������ʼ���üĴ���ҳ�ϵĸ��ּĴ���
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //���üĴ���ҳ0�ϵ�COMMAND�Ĵ�����ѡ��׼��д�Ĵ���ҳ0
    outb(ne2k.rx_page_start, ne2k.iobase+NE_P0_PSTART);
    //����PSTART
    outb(ne2k.rx_page_stop, ne2k.iobase+NE_P0_PSTOP);
    //����PSTOP
    outb(ne2k.rx_page_start, ne2k.iobase+NE_P0_BNRY);
    //����BNRY

    outb(NE_CR_PAGE1|NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //���üĴ���ҳ0�ϵ�COMMAND�Ĵ�����ѡ��׼��д�Ĵ���ҳ1
    outb(ne2k.next_packet, ne2k.iobase+NE_P1_CURR);
    //����CURR�Ĵ���
    for(i=0;i<6;i++)
        outb(ne2k.paddr.bytes[i], ne2k.iobase + NE_P1_PAR0 + i);
        //i=0..5����MAC��ַ�����ڼĴ���ҳ1��Ӧ�Ķ˿�
    for(i=0;i<8;i++)
        outb(0, ne2k.iobase + NE_P1_MAR0 + i);
        //i=0..7�����öಥ��ַ�����ڱ�ʵ����Ӱ�죬������0
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //���üĴ���ҳ1�ϵ�COMMAND�Ĵ�����ѡ��׼��д�Ĵ���ҳ0
    outb(NE_RCR_AB, ne2k.iobase + NE_P0_RCR); 
    //����㲥
    outb(NE_DCR_WTS,ne2k.iobase+NE_P0_DCR);
    //����Ϊ��λ�շ�����С�����������ֽ�

    //��Զ��DMA�ļ���ֵ��ʼ��Ϊ0�������������ڲ���Ҫ�������棬���������շ����ݰ�
    outb(0,ne2k.iobase+NE_P0_RBCR0);
    outb(0,ne2k.iobase+NE_P0_RBCR1);
    outb(NE_CR_RD2|NE_CR_STA,ne2k.iobase+NE_P0_CR);

    set_trap_gate(0x2b, &ne2k_interrupt); //�ж��������
    outb_p(inb_p(0x21) & 0xFB, 0x21);
    outb_p(inb_p(0xa1) & 0x7F, 0xA1); //����OCW�����δ�

    outb(NE_IMR_PRXE, ne2k.iobase + NE_P0_IMR);//�������ݰ������жϵĴ���

}
//ARP��װ
void arphdr_alloc(arphdr_t *arp){
    int i;
    arp->htype=1;//��ʾӲ������Ϊ��̫������֡
    arp->ptype=0x0800;//��ʾ����һ��IPЭ��
    arp->hlen=6;//Ӳ����ַΪ6B
    arp->plen=4;//IP��ַλ4B
    arp->oper=1;//��ʾ����һ��ARP�������ݰ�
    for(i=0;i<ETHER_ADDR_LEN;i++)
        arp->sha[i]=ne2k.paddr.bytes[i]; //���Ͷ�Ӳ����ַ
    arp->spa[0]=192;
    arp->spa[1]=168;
    arp->spa[2]=1;
    arp->spa[3]=11;
    for(i=0;i<ETHER_ADDR_LEN;i++)
        arp->tha[i]=0xFF; //Ŀ���Ӳ����ַ
    arp->tpa[0]=192;
    arp->tpa[1]=168;
    arp->tpa[2]=140;
    arp->tpa[3]=128;
}

void etherhdr_alloc(etherhdr_t *eth){
    int i;
    for(i=0;i<ETHER_ADDR_LEN;i++)
        eth->dhost[i]=0xFF;//��ʾ����һ����̫���㲥֡
    for(i=0;i<ETHER_ADDR_LEN;i++)
        eth->shost[i]=ne2k.paddr.bytes[i]; //��̫��Դ��ַ
    eth->ethertype=0x0806;//��ʾ����֡��װ��һ��ARP���ݰ�
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
//������������֡�Ĵ���ʵ�֣���һ����������֡���������ķ��ͻ�������
int ne2k_transmit(struct pbuf *p)
{
    unsigned short packetlen = p->len;
    //ARP���ݰ�����̫������֡����Ϊ54���ֽ�
    unsigned char status;
    int i;
    outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
    //������ֹԶ��DMA
    outb((unsigned char) packetlen, ne2k.iobase + NE_P0_RBCR0);
    outb((unsigned char) (packetlen >> 8), ne2k.iobase +NE_P0_RBCR1);
    //��¼ҪԶ��д�����ݳ���
    unsigned short dst = ne2k.rx_page_stop;//���ͻ������ڽ��ջ�������
    //���ͻ������Ŀ�ʼ��ַ
    outb((unsigned char) (dst * NE_PAGE_SIZE), ne2k.iobase + NE_P0_RSAR0);
    outb((unsigned char) ((dst * NE_PAGE_SIZE) >> 8), ne2k.iobase + NE_P0_RSAR1);
    //��¼ҪԶ��д��Ŀ���ַ
    outb(NE_CR_RD1 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
    //����Զ��д
    unsigned short *payload=(char *)(p->payload);
    for(i=0;i<packetlen/2;i++) outw(*(payload+i), ne2k.iobase + NE_DATAPORT);
    //�����ݶ˿�д���ÿ���ֻ��Զ����õ�BUFFER RAM����dst��ʼ�ҳ���Ϊpacketlen��һ�δ�������
    while((status = inb(ne2k.iobase + NE_P0_ISR) & 0x40) == 0);

    //������������֡�Ĵ���ʵ�֣���S�������������������ݴ�������
    outb((unsigned char) dst, ne2k.iobase + NE_P0_TPSR);
    if (packetlen > 54) {
        outb(packetlen, ne2k.iobase + NE_P0_TBCR0);
        outb(packetlen >> 8,ne2k.iobase + NE_P0_TBCR1);
    }
    else {
        outb(54, ne2k.iobase + NE_P0_TBCR0);
        outb(0, ne2k.iobase + NE_P0_TBCR1);
    }
    outb(NE_CR_RD2 | NE_CR_TXP | NE_CR_STA, ne2k.iobase + NE_P0_CR);//�������ݰ�
}

//�������ݰ�
void ne2k_handler(void)
{
    unsigned char status;
	outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);//ѡ�Ĵ���ҳ0
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
	//��ΪNE_P1_CURR�ڼĴ���ҳ1��
	while (ne2k.next_packet != inb(ne2k.iobase + NE_P1_CURR))
	{
		packet_ptr = ne2k.next_packet * NE_PAGE_SIZE;
        //�������ݶ˿ڴ�packet_ptr��ַ������4���ֽڵ�packet_hdr��
		len = packet_hdr.recv_count - sizeof(struct recv_ring_desc);
		pbuf = malloc(len);
		packet_ptr += sizeof(struct recv_ring_desc);
        //���������ݰ������������ݶ˿ڴ�packet_ptr��ַ������packetlen/2���ֵ�pbuf�У�
		ne2k.next_packet = packet_hdr.next_packet;
		outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
            //�Ļ�ҳ0����ΪNE_PO_BNRY��page 0��
		bndry = ne2k.next_packet - 1;
		if (bndry < ne2k.rx_page_start) bndry = ne2k.rx_page_stop - 1;
		outb(bndry, ne2k.iobase + NE_P0_BNRY);
        
        outb(NE_CR_PAGE1 | NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
        //�ٸĻص�ҳ1����ΪNE_P1_CURR��ҳ1��
	}
}