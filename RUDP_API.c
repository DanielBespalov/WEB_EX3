#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

void error_handling(char *message) {
    perror(message);
    exit(1);
}

uint16_t calculate_checksum(void *data, int length) {
    uint16_t *buf = (uint16_t *)data;
    unsigned long sum = 0;

    for (; length > 1; length -= 2) {
        sum += *buf++;
    }

    if (length == 1) {
        sum += *(uint8_t *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return (uint16_t)(~sum);
}

void rudp_send_packet(int sockfd, struct sockaddr_in *addr, socklen_t addr_size, char *data, uint16_t flags) {
    struct rudp_header header;
    char packet[BUFFER_SIZE];

    int data_len = strlen(data);
    if (data_len > BUFFER_SIZE - RUDP_HEADER_SIZE) {
        data_len = BUFFER_SIZE - RUDP_HEADER_SIZE;
    }

    header.length = htons(data_len);
    header.flags = htons(flags);
    header.checksum = 0;

    memcpy(packet, &header, RUDP_HEADER_SIZE);
    memcpy(packet + RUDP_HEADER_SIZE, data, data_len);

    header.checksum = calculate_checksum(packet, RUDP_HEADER_SIZE + data_len);
    memcpy(packet, &header, RUDP_HEADER_SIZE);

    if (sendto(sockfd, packet, RUDP_HEADER_SIZE + data_len, 0, (struct sockaddr *)addr, addr_size) == -1) {
        error_handling("Failed to send packet");
    }
}

ssize_t rudp_receive_packet(int sockfd, char *buffer, struct sockaddr_in *addr, socklen_t *addr_size) {
    char packet[BUFFER_SIZE];
    ssize_t bytes_received = recvfrom(sockfd, packet, BUFFER_SIZE, 0, (struct sockaddr *)addr, addr_size);

    if (bytes_received > 0) {
        struct rudp_header *header = (struct rudp_header *)packet;
        uint16_t received_checksum = header->checksum;
        header->checksum = 0;
        uint16_t calculated_checksum = calculate_checksum(packet, bytes_received);

        if (received_checksum != calculated_checksum) {
            printf("Checksum error, packet discarded\n");
            return -1;
        }

        int data_length = ntohs(header->length);
        if (data_length > bytes_received - RUDP_HEADER_SIZE) {
            printf("Packet length error, packet discarded\n");
            return -1;
        }

        memcpy(buffer, packet + RUDP_HEADER_SIZE, data_length);
        return data_length;
    }

    return bytes_received;
}
