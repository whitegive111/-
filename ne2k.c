#define NE_DATAPORT 0x10
#define NE_IOBASE 0xc020
#define NE_IRQ 11

#define NE_PAGE_SIZE 256 //NE2000�ڴ滺��ÿһҳ�Ĵ�С��256B
#define NE_TXBUF_SIZE 4 //���ͻ���ߴ�(��ҳpageΪ��λ)
#define NE_TX_BUFERS 2 //���ͻ�������

#define NE_CR_STP 0x01 //�ر��������������շ����ݰ�
#define NE_CR_STA 0x02 //���������������շ����ݰ�
#define NE_CR_RD2 0x20 //��ֹԶ��DMA
#define NE_P0_CR        0x00      //COMMAND �Ĵ����˿ڵ�ַƫ��
#define NE_P0_PSTART    0x01      //PSTART �Ĵ����˿ڵ�ַƫ��
#define NE_P0_PSTOP     0x02      //PSTOP �Ĵ����˿ڵ�ַƫ��
#define NE_P0_BNRY      0x03      //BNRY �Ĵ����˿ڵ�ַƫ��
#define NE_CR_PAGE1 0x40    //��PS0λ����Ϊ1 
#define NE_P1_CURR 0x07     //CURR �Ĵ����˿ڵ�ַƫ��

#define NE_P1_PAR0 0x01 //MAC��ַ��һ���ֽڵĶ˿�
#define NE_P1_MAR0 0x08 //�ಥ��ַ��һ���ֽڵĶ˿�
#define NE_P0_RCR 0x0C
#define NE_RCR_AB 0x04 //������չ㲥���ݰ�

#define NE_P0_DCR 0x0E //�������üĴ���DCR
#define NE_DCR_WTS 0x01 //����Ϊ��λ
/*Ҫ���ץ�����߹۲��յ������ݰ�
  ��ʵ�ʳ����кܿ�����Ҫ����ʱ��һ�����е������ֽڵ�˳��Ե�*/
#define NE_DCR_BOS 0x02 //С���ֽ�˳��
#define NE_P0_RBCR0 0x0A //Remote Byte Count�����ֽڣ�
#define NE_P0_RBCR1 0x0B //Remote Byte Count�����ֽڣ�

//�����������ݽṹ����
struct macaddr {
    unsigned char bytes[6];
};
struct ne {
unsigned short iobase;
unsigned short irq;
unsigned short membase; //���������׵�ַ
unsigned short memsize; //�������Ĵ�С
unsigned char rx_page_start; //���ͻ������Ŀ�ʼҳ��
unsigned char rx_page_stop; //���ͻ������Ľ���ҳ��
unsigned char next_packet; //��һ��δ���Ľ��յ������ݰ�
struct macaddr paddr; //������MAC��ַ�������շ������ݰ�ʱҪ�õ�
};

struct ne ne2k; //һ��ȫ�����ݽṹ��¼NE2000�Ļ�����Ϣ

ne2k_init()
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
    ne2k.paddr.bytes[0]=0x00;
    ne2k.paddr.bytes[1]=0x0c;
    ne2k.paddr.bytes[2]=0x29;
    ne2k.paddr.bytes[3]=0x07;
    ne2k.paddr.bytes[4]=0xf8;
    ne2k.paddr.bytes[5]=0xbd;

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
        //i=0..5
    for(i=0;i<8;i++)
        outb(0, ne2k.iobase + NE_P1_MAR0 + i);
        //i=0..7
    outb(NE_CR_RD2|NE_CR_STP, ne2k.iobase+NE_P0_CR);
    //���üĴ���ҳ1�ϵ�COMMAND�Ĵ�����ѡ��׼��д�Ĵ���ҳ0
    outb(NE_RCR_AB, ne2k.iobase + NE_P0_RCR); 
    //����㲥
    outb(NE_DCR_WTS,ne2k.iobase+NE_P0_DCR);
    //����Ϊ��λ�շ�����С�����������ֽ�
    outb(0,ne2k.iobase+NE_P0_RBCR0);
    outb(0,ne2k.iobase+NE_P0_RBCR1);
    outb(NE_CR_RD2|NE_CR_STA,ne2k.iobase+NE_P0_CR);
}