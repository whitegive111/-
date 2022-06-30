#ifndef _NE2K_H
#define _NE2K_H

#define NE_DATAPORT 0x10
#define NE_IOBASE 0xc020
#define NE_IRQ 11

/** indicates this packet's data should be immediately passed to the application */
#define PBUF_FLAG_PUSH      0x01U //��������
/** indicates this is a custom pbuf: pbuf_free calls pbuf_custom->custom_free_function()
    when the last reference is released (plus custom PBUF_RAM cannot be trimmed) */
#define PBUF_FLAG_IS_CUSTOM 0x02U
/** indicates this pbuf is UDP multicast to be looped back */
#define PBUF_FLAG_MCASTLOOP 0x04U //udp�ಥ����
/** indicates this pbuf was received as link-level broadcast */
#define PBUF_FLAG_LLBCAST   0x08U //��·��Ĺ㲥
/** indicates this pbuf was received as link-level multicast */
#define PBUF_FLAG_LLMCAST   0x10U //��·��Ķಥ
/** indicates this pbuf includes a TCP FIN flag */
#define PBUF_FLAG_TCP_FIN   0x20U //tcp���ֱ�־

#define NE_PAGE_SIZE 256 //NE2000�ڴ滺��ÿһҳ�Ĵ�С��256B
#define NE_TXBUF_SIZE 4 //���ͻ���ߴ�(��ҳpageΪ��λ)
#define NE_TX_BUFERS 2 //���ͻ�������

#define NE_CR_STP 0x01 //�ر��������������շ����ݰ�
#define NE_CR_STA 0x02 //���������������շ����ݰ�
#define NE_CR_TXP 0x04 //��ʼ�������ݰ�
#define NE_CR_RD0 0x8 //Զ��DMA��
#define NE_CR_RD1 0x10 //Զ��DMAд
#define NE_CR_RD2 0x20 //��ֹԶ��DMA

#define NE_P0_CR        0x00      //COMMAND �Ĵ����˿ڵ�ַƫ��
#define NE_P0_PSTART    0x01      //PSTART �Ĵ����˿ڵ�ַƫ��
#define NE_P0_PSTOP     0x02      //PSTOP �Ĵ����˿ڵ�ַƫ��
#define NE_P0_BNRY      0x03      //BNRY �Ĵ����˿ڵ�ַƫ��
#define NE_P0_TPSR 0x04           //�������ݰ���ʼҳ
#define NE_P0_TBCR0 0x05          //�����ֽ���(��8λ)
#define NE_P0_TBCR1 0x06          //�����ֽ���(��8λ)
#define NE_P0_ISR 0x07            //�ж�״̬�Ĵ���
#define NE_P0_RSAR0 0x08          //Զ��DMA�Ŀ�ʼ��ַ���Ͱ�λ��
#define NE_P0_RSAR1 0x09          //Զ��DMA�Ŀ�ʼ��ַ���߰�λ��
#define NE_P0_RBCR0 0x0A          //Զ��DMA�Ĵ�С���Ͱ�λ��
#define NE_P0_RBCR1 0x0B           //Զ��DMA�Ĵ�С���߰�λ��
#define NE_P0_RCR 0x0C
#define NE_P0_DCR 0x0E            //�������üĴ���DCR
#define NE_P0_IMR 0x0F

#define NE_P1_CURR      0x07      //CURR �Ĵ����˿ڵ�ַƫ��
#define NE_P1_PAR0 0x01 //MAC��ַ��һ���ֽڵĶ˿�
#define NE_P1_MAR0 0x08 //�ಥ��ַ��һ���ֽڵĶ˿�

#define NE_CR_PAGE0 0x00    //ѡ��Ĵ���ҳ0
#define NE_CR_PAGE1 0x40    //ѡ��Ĵ���ҳ1 

#define NE_RCR_AB 0x04 //������չ㲥���ݰ�

#define NE_DCR_WTS 0x01 //����Ϊ��λ
/*Ҫ���ץ�����߹۲��յ������ݰ�
  ��ʵ�ʳ����кܿ�����Ҫ����ʱ��һ�����е������ֽڵ�˳��Ե�*/
#define NE_DCR_BOS 0x02 //С���ֽ�˳��

#define ETHER_ADDR_LEN 6 //Ӳ����ַ����
#define LOGIC_ADDR_LEN 4 //IP��ַ����

#define NE_IMR_PRXE 0x01 //�����ݰ������ж�
#define NE_ISR_PRX 0x01 //���յ�һ�����ݰ�
#define NE_ISR_RDC 0x40 //Զ��DMA�����Ժ���������λ

extern void ne2k_init(void);
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

//ARP���ݰ�����̫������֡��ͷ�����ݽṹ����
typedef struct arphdr {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char sha[ETHER_ADDR_LEN]; //���Ͷ�Ӳ����ַ
    unsigned char spa[LOGIC_ADDR_LEN]; //���Ͷ�IP��ַ
    unsigned char tha[ETHER_ADDR_LEN]; //Ŀ���Ӳ����ַ
    unsigned char tpa[LOGIC_ADDR_LEN]; //Ŀ���IP��ַ
}arphdr_t;

typedef struct etherhdr {
    unsigned char dhost[ETHER_ADDR_LEN];//��̫��Ŀ���ַ
    unsigned char shost[ETHER_ADDR_LEN];//��̫��Դ��ַ
    unsigned short ethertype; //Э������
}etherhdr_t;


/*
next��һ��pbuf���͵�ָ�룬ָ����һ��pbuf����Ϊ�����е����ݰ����ܴܺ󣬶�pbuf�ܹ�������ݰ���С���ޣ��ͻ�����������ʽ�����е�pbuf�����������������Ӳ�����������һ�����ݰ�����Щ����������pbuf�������һ��������֮Ϊpbuf����
payload��һ��ָ�����������ָ�룬ָ���pbuf���������������ʼ��ַ�������������������ǽ�����pbuf�ṹ���ַ�����RAM�ռ䣬Ҳ������ROM�е�ĳ����ַ�ϣ�ȡ����pbuf�����͡�
tot_len�м�¼���ǵ�ǰpbuf�������pbuf�������ݵĳ��ȣ����������ǰpbuf��pbuf�����ϵ�һ�����ݽṹ����ôtot_len�ͼ�¼������pbuf����������pbuf�����ݵĳ��ȣ������ǰpbuf�����������һ�����ݽṹ���Ǿͼ�¼�ŵ�ǰpbuf�ĳ��ȡ�
len��ʾ��ǰpbuf����Ч�����ݳ��ȡ�
type��ʾpbuf�����ͣ�LwIP����4��pbuf�����ͣ�����ʹ����һ��ö�����͵����ݽṹ�������ǡ�
flags��ͬЭ����ʾ���ݲ�ͬ��
ref��ʾ��pbuf�����õĴ��������ñ�ʾ������ָ��ָ��ǰpbuf�������ָ�������pbuf��nextָ�룬Ҳ����������������ʽ��ָ�룬��ʼ��һ��pbuf��ʱ��ref�ᱻ����Ϊ1����Ϊ��pbuf�ĵ�ַһ���ᱻ����һ��ָ���������������ָ��ָ��pbuf��ʱ�򣬾ͱ��������غ�����ref�ֶμ�1��
*/
struct pbuf {
  struct pbuf *next;	//ָ����һ��pbuf

  void *payload;  //ָ��buff�е���ʵ����

  unsigned short tot_len;  //��len���������pbuf��len
  
  unsigned short len;  //payload�����ݳ��ȣ��������ײ�

  unsigned char type;

  unsigned char flags;

  unsigned short ref;  //buffer�����ô���������next

};

struct recv_ring_desc{
	unsigned char recv_status;//����״̬
	unsigned char next_packet;//ָ����һ�����ݰ���ָ��
	unsigned short recv_count;//���յ������ݰ��ĳ��ȣ�������4���ֽڣ�
};

struct ne ne2k; //һ��ȫ�����ݽṹ��¼NE2000�Ļ�����Ϣ

#endif