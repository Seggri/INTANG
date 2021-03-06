/*
 * Strategy implementation
 * .setup function:     Set up triggers, which listen to specific 
 *                      incoming or outgoing packets, and bind 
 *                      triggers to these events. 
 * .teardown function:  Unbind triggers.
 *
 */

#include "rst_wrong_ack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "globals.h"
#include "socket.h"
#include "protocol.h"
#include "logging.h"


void send_RSTACK_with_wrong_ack(
        const char *src_ip, unsigned short src_port, 
        const char *dst_ip, unsigned short dst_port,
        unsigned int seq_num, unsigned int ack_num)
{
    char pkt[MAX_PACKET_SIZE];

    struct send_tcp_vars vars;
    //log_debug("size of vars: %ld", sizeof vars);
    memset(&vars, 0, sizeof vars);
    strncpy(vars.src_ip, src_ip, 16);
    strncpy(vars.dst_ip, dst_ip, 16);
    vars.src_port = src_port;
    vars.dst_port = dst_port;
    vars.flags = TCP_RST | TCP_ACK;
    vars.seq_num = seq_num;
    vars.ack_num = htonl(ntohl(ack_num) + 100);

    //dump_send_tcp_vars(&vars);

    send_tcp(&vars);
}

int x2_setup()
{
    char cmd[256];
    sprintf(cmd, "iptables -A INPUT -p tcp -m multiport --sport 53,80 --tcp-flags SYN,ACK SYN,ACK -j NFQUEUE --queue-num %d", NF_QUEUE_NUM);
    system(cmd);

    return 0;
}

int x2_teardown()
{
    char cmd[256];
    sprintf(cmd, "iptables -D INPUT -p tcp -m multiport --sport 53,80 --tcp-flags SYN,ACK SYN,ACK -j NFQUEUE --queue-num %d", NF_QUEUE_NUM);
    system(cmd);

    return 0;
}

int x2_process_synack(struct mypacket *packet)
{
    char sip[16], dip[16];
    unsigned short sport, dport;

    struct in_addr s_in_addr = {packet->iphdr->saddr};
    struct in_addr d_in_addr = {packet->iphdr->daddr};
    strncpy(sip, inet_ntoa(s_in_addr), 16);
    strncpy(dip, inet_ntoa(d_in_addr), 16);
    sport = ntohs(packet->tcphdr->th_sport);
    dport = ntohs(packet->tcphdr->th_dport);

    send_RSTACK_with_wrong_ack(dip, dport, sip, sport, packet->tcphdr->th_ack, htonl(ntohl(packet->tcphdr->th_seq)+1));
    send_RSTACK_with_wrong_ack(dip, dport, sip, sport, packet->tcphdr->th_ack, htonl(ntohl(packet->tcphdr->th_seq)+1));
    send_RSTACK_with_wrong_ack(dip, dport, sip, sport, packet->tcphdr->th_ack, htonl(ntohl(packet->tcphdr->th_seq)+1));

    return 1;
}

int x2_process_request(struct mypacket *packet)
{
    return 0;
}




