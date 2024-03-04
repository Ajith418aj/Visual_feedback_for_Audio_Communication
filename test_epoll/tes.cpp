#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

int main() {
    const char *interface_name = "eth0";
    const char *source_port = "2152";
    const char *queue_id = "10";

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

    struct ethtool_rxnfc filter;
    memset(&filter, 0, sizeof(struct ethtool_rxnfc));
    filter.cmd = ETHTOOL_SRXFH;
    filter.flow_type = RX_CLS_FLOW_DISC;
    filter.data = (unsigned long)&ifr;

    struct ethtool_rxnfc_flow_spec flow_spec;
    memset(&flow_spec, 0, sizeof(struct ethtool_rxnfc_flow_spec));
    flow_spec.flow_type = ETHER_FLOW;
    flow_spec.h_u.ether_spec.h_proto = htons(ETH_P_IP);
    flow_spec.h_u.ether_spec.h_proto_mask = htons(0xFFFF);
    flow_spec.h_u.ether_spec.h_source[0] = htons(atoi(source_port));
    flow_spec.location = atoi(queue_id);

    filter.rule_cnt = 1;
    filter.data = (unsigned long)&flow_spec;

    if (ioctl(sockfd, SIOCETHTOOL, &filter) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    printf("Filter configuration successful.\n");

    close(sockfd);

    return 0;
}
