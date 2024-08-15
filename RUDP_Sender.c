#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "RUDP_API.h"

#define BUFFER_SIZE 1024
#define EXIT_MESSAGE "EXIT"

void send_file(int sockfd, struct sockaddr_in *server_addr, socklen_t server_addr_size, FILE *fp) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE - RUDP_HEADER_SIZE, fp)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the data
        rudp_send_packet(sockfd, server_addr, server_addr_size, buffer, FLAG_DATA);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s -ip <IP_ADDRESS> -p <PORT>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[2];
    int port = atoi(argv[4]);

    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_size = sizeof(server_addr);

    char filename[256]; 

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        error_handling("Socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    printf("Enter the name of the file to open: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        error_handling("Error reading filename");
    }

    filename[strcspn(filename, "\n")] = 0;  // Remove newline character

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File open failed");
        exit(1);
    }

    while (1) {
        // Send the file
        send_file(sockfd, &server_addr, server_addr_size, fp);

        rudp_send_packet(sockfd, &server_addr, server_addr_size, EXIT_MESSAGE, FLAG_FIN);        

        printf("File sent. Send again? (y/n): ");
        char choice = getchar();
        getchar();  // To consume the newline character

        if (choice == 'n' || choice == 'N') {
            break;
        }
    }
    
    
    fclose(fp);
    close(sockfd);

    return 0;
}
