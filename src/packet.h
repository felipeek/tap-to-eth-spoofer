#pragma once

#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/ip.h>

struct ether_header* packet_ethernet_get_header(const unsigned char* packet);
uint16_t packet_ethernet_get_type(const struct ether_header* ether_header);
const uint8_t* packet_ethernet_get_src_mac_addr(const struct ether_header* ether_header);
void packet_ethernet_set_src_mac_addr(struct ether_header* ether_header, uint8_t new_src_mac_addr[6]);
const uint8_t* packet_ethernet_get_dst_mac_addr(const struct ether_header* ether_header);
void packet_ethernet_set_dst_mac_addr(struct ether_header* ether_header, uint8_t new_dst_mac_addr[6]);
void packet_ethernet_print(const struct ether_header* ether_header);
struct iphdr* packet_ip_get_header(const unsigned char* packet);
uint32_t packet_ip_get_src_ip(const struct iphdr* ip_header);
void packet_ip_set_src_ip(struct iphdr* ip_header, uint32_t new_src_ip);
uint32_t packet_ip_get_dst_ip(const struct iphdr* ip_header);
void packet_ip_print(const struct iphdr* ip_header);
struct arphdr* packet_arp_get_header(const unsigned char* packet);
uint16_t packet_arp_get_hardware_type(const struct arphdr* arp_header);
uint16_t packet_arp_get_protocol_type(const struct arphdr* arp_header);
uint16_t packet_arp_get_opcode(const struct arphdr* arp_header);
const unsigned char* packet_arp_get_sender_hw_address(const struct arphdr* arp_header);
void packet_arp_set_sender_hw_address(struct arphdr* arp_header, char new_sender_hw_addr[6]);
uint32_t packet_arp_get_sender_protocol_address(const struct arphdr* arp_header);
void packet_arp_set_sender_protocol_address(const struct arphdr* arp_header, uint32_t new_sender_protocol_addr);
const unsigned char* packet_arp_get_target_hw_address(const struct arphdr* arp_header);
void packet_arp_set_target_hw_address(struct arphdr* arp_header, char new_target_hw_addr[6]);
uint32_t packet_arp_get_target_protocol_address(const struct arphdr* arp_header);
void packet_arp_set_target_protocol_address(const struct arphdr* arp_header, uint32_t new_target_protocol_addr);
void packet_arp_print(const struct arphdr* arp_header);
void packet_print(const unsigned char* packet);