#ifndef RUDP_API_H
#define RUDP_API_H

#include <stdint.h>
#include <arpa/inet.h> // For sockaddr_in
#include <sys/types.h> // For ssize_t

#define BUFFER_SIZE 1024
#define RUDP_HEADER_SIZE 6
#define FLAG_SYN 0x01
#define FLAG_ACK 0x02
#define FLAG_FIN 0x04
#define FLAG_DATA 0x08

struct rudp_header {
    uint16_t length;
    uint16_t checksum;
    uint16_t flags;
};

void error_handling(char *message);
uint16_t calculate_checksum(void *data, int length);
ssize_t rudp_receive_packet(int sockfd, char *buffer, struct sockaddr_in *addr, socklen_t *addr_size, double loss_percentage);
void rudp_send_packet(int sockfd, struct sockaddr_in *addr, socklen_t addr_size, char *data, uint16_t flags, double loss_percentage);


#endif // RUDP_API_H
