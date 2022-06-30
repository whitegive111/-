set_trap_gate(0x2b, &ne2k_interrupt); //断口设置
outb_p(inb_p(0x21) & 0xfb, 0x21);
outb_p(inb_p(0xa1) & 0x7f, 0xa1); //发送ocw

call ne2k_handler
movb $0x20, % al
outb %al,$0xa0
outb %al,$0x20

#define NE_ISR_PRX 0x01
ne2k_handler()
{
	outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_PO_CR);
	while ((status = inp(ne2k.iobase + NE_PO_ISR)) != 0) {
		outb(status, ne2k.iobase + NE_PO_ISR);
		if (status & NE_ISR_PRX) {
			ne2k_receive();
		}
	}
}
struct recv_ring.desc{
	unsigned char recv_status;
	unsigned char next_packet;
	unsigned short recv_count;
};
#define NE_P1_CURR 0x07
#define NE_CR_PAGE0 0x00
#define NE_CR_PAGE1 0x40
#define NE_P0_BNRY	0x03

void ne2k_receive()
{
	struct recv_ring_desc packet_hdr;
	unsigned short packet_ptr;
	unsigned short len;
	unsigned char bndry;
	outb(NR_CR_PAGE1 | NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
	//因为NE_P1_CURR在寄存器页1上
	while (ne2k.next_pkt != inb(ne2k.iobase + NE_P1_CURR))
	{
		packet_ptr = ne2k.next_pkt * NE_PAGE_SIZE;
		packetlen = packet_hdr.recv_count - sizeof(struct recv_ring_desc);
		pbuf = malloc(packetlen);
		packet_ptr += sizeof(struct recv_ring_desc);
		ne2k.next_pkt = packet_hdr.next_packet;
		outb(NE_CR_RD2 | NE_CR_STA, ne2k.iobase + NE_P0_CR);
		bndry = ne2k.next_pkt - 1;
		if (bndry < ne2k.rx_page_start)bndry = ne2k.rx_page_stop - 1;
		outb(bndry, ne2k.iobase + NE_P0_BNRY);
	}
}
//ARP数据包解析与ARP缓存表
struct arphdr_t* arp = (struct arphdr_t*)q->payload;
if (arp->oper == 2)
{
	macadder = arp->sha;
	ipaddr = arp->spa;
}
struct arpcache
{
	ipaddr;
	macaddr;
};

//计算Checksum代码实现
int sum = 0;
while (count > 1) {
	sum += *(unsigned short)addr++;
	count -= 2;
}
if (count > 0)sun += *(unsigned char*)addr;
while (sum >> 16)sum = (sum & 0xffff) + (sum >> 16);
checksum = sum;

//ping 命令代码实现
static int timestart;
static int nreceived = 0;
static struct task_struct* icmp_wait = NULL;
main(int argc, char* argv[])
{
	for (i = 0; i < 5; i++)
	{
		timestart = jiffies;
		send_echorequest(ipaddr, i);
		sleep_on(&icmp_wait);
		if (icmp_wake_type==TIMEOUT)
	}
}
send_echorequest(ipaddr, i);
{
	ident = getpid() & 0xffff;
	struct icmphdr* icp = malloc(struct icmphdr);
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_seq = ntransmitted++;
	icp->icmp_id = ident;
	icp->icmp_cksum = cksum((unsigned short*)icp);
	sendto(icp);
}
//网卡中断处理程序的部分代码
void ne2k_receive()
[
	if (icmp->icmp_type == ICMP_ECHOREPLY)
	{
		if (icmp->icmp_id != ident)return;
		++nreceived;
		icmp_seq = icp->icmp_seq;
		ttl = ip->ttl;
		timeend = jiffies;
		printf("64 bytes from xx:icmp_seq=%d ttl=%d time=%f", );
		wake_up(&icmp_wait);
	}
]