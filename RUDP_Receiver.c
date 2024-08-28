// RUDP_Receiver.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include "RUDP_API.h"  // Ensure this header is correct and includes necessary RUDP definitions

#define BUFFER_SIZE 1024
#define EXIT_MESSAGE "EXIT"
#define MIN_FILE_SIZE_MB 2  // minimum file size in megabytes

double calculate_bandwidth(size_t file_size, double time_taken) {
    if (time_taken <= 0) return 0;
    return (file_size / (1024.0 * 1024.0)) / time_taken;  // MB/s
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        printf("Usage: %s -p <PORT>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[2]);
    if (port <= 0) {
        printf("Invalid port number.\n");
        exit(1);
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(1);
    }

    printf("Socket created and bound successfully.\n");

    int run_number = 1;  // Track the number of runs
    double total_bandwidth_mb;

    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;
        double total_bandwidth = 0;  // Accumulates total bytes across all files
        int file_count = 0;
        double total_time = 0.0;
        double loss_probability = 0.10; 
        
        while (1) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            size_t file_bytes_received = 0;  // Track bytes received for this file

            while ((bytes_received = rudp_receive_packet(sockfd, buffer, &client_addr, &client_addr_size, loss_probability)) > 0) {
                if (strncmp(buffer, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
                    break;
                }
                file_bytes_received += bytes_received;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

            if (bytes_received == -1) {
                perror("Receive failed");
                exit(1);
            } else if (bytes_received == 0) {
                break;
            }

            file_count++;
            total_time += time_taken;
            total_bandwidth += file_bytes_received;  // Accumulate total bytes received
            // Print statistics after connection closed
            double average_time = (file_count > 0) ? total_time / file_count : 0.0;
            total_bandwidth_mb = (total_time > 0) ? calculate_bandwidth(total_bandwidth, total_time) : 0;
            printf("\nStatistics:\n");
            printf("- Average time = %.2f ms\n", average_time * 1000);
            printf("- Average bandwidth = %.2f MB/s\n", total_bandwidth_mb);

            run_number++;
    }
}

        

        // Ensure at least 2MB file size
        if (total_bandwidth_mb < MIN_FILE_SIZE_MB * 1024 * 1024) {
            printf("Warning: Received file size is less than the minimum required %dMB\n", MIN_FILE_SIZE_MB);
        }

        

    close(sockfd);

    return 0;
}
