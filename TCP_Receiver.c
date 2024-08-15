#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define EXIT_MESSAGE "EXIT"
#define MIN_FILE_SIZE_MB 2  // maximum file size in megabytes

void error_handling(char *message) {
    perror(message);
    exit(1);
}

void set_congestion_control(int sockfd, const char *algo) {
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) != 0) {
        error_handling("Error setting TCP congestion control");
    }
}

double calculate_bandwidth(size_t file_size, double time_taken) {
    if (time_taken <= 0) return 0;
    return (file_size / (1024.0 * 1024.0)) / time_taken;  // MB/s
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[2]);
    char *algo = argv[4];

    int sockfd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error_handling("Socket creation failed");
    }

    set_congestion_control(sockfd, algo);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("Bind failed");
    }

    if (listen(sockfd, 5) == -1) {
        error_handling("Listen failed");
    }

    int run_number = 1;  // Track the number of runs

    while (1) {
        new_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (new_sock == -1) {
            error_handling("Accept failed");
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;
        double total_bandwidth = 0;  // Accumulates total bytes across all files
        int file_count = 0;
        double total_time = 0.0;

        while (1) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            size_t file_bytes_received = 0;  // Track bytes received for this file

            while ((bytes_received = recv(new_sock, buffer, BUFFER_SIZE, 0)) > 0) {
                
                if (strncmp(buffer, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
                    printf("Received EXIT message\n");  // Debug print
                    break;
                }

                file_bytes_received += bytes_received;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

            if (bytes_received == -1) {
                error_handling("Receive failed");
            } else if (bytes_received == 0) {
                printf("Connection closed by sender\n");  // Debug print
                break;
            }

            file_count++;
            double bandwidth = calculate_bandwidth(file_bytes_received, time_taken);
            printf("Run #%d Data: Time = %.2f ms; Speed = %.2f MB/s\n", run_number, time_taken * 1000, bandwidth);

            total_time += time_taken;
            total_bandwidth += file_bytes_received;  // Accumulate total bytes received

            if (strncmp(buffer, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
                break;
            }
        }

        double average_time = (file_count > 0) ? total_time / file_count : 0.0;
        double total_bandwidth_mb = (total_time > 0) ? calculate_bandwidth(total_bandwidth, total_time) : 0;

        // Ensure at least 2MB file size
        if (total_bandwidth < MIN_FILE_SIZE_MB * 1024 * 1024) {
            printf("Warning: Received file size is less than the minimum required %dMB\n", MIN_FILE_SIZE_MB);
        }

        printf("\nStatistics:\n");
        printf("- Average time = %.2f ms\n", average_time * 1000);
        printf("- Average bandwidth = %.2f MB/s\n", total_bandwidth_mb);

        close(new_sock);  // Close the connection with the current sender and go back to accepting new connections
        printf("Sender disconnected. Waiting for new connection...\n");
        run_number++;
    }

    close(sockfd);
    

    return 0;
}
