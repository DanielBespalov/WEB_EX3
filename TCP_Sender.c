#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define EXIT_MESSAGE "EXIT"

// Randomly drop packets based on the given loss probability
int should_drop_packet(double loss_probability) {
    return (rand() / (double)RAND_MAX) < loss_probability;
}

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

void set_congestion_control(int sockfd, const char *algo) {
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) != 0) {
        error_handling("Error setting TCP congestion control");
    }
}

void send_file(int sockfd, FILE *fp,double loss_probability) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    srand(time(NULL)); // Initialize random seed

    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) {
        if (should_drop_packet(loss_probability)) {
            printf("Simulating packet loss...\n");
            continue; // Skip sending this packet
        }

        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            error_handling("File send failed");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Usage: %s -ip <IP_ADDRESS> -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[2];
    int port = atoi(argv[4]);
    char *algo = argv[6];

    int sockfd;
    struct sockaddr_in server_addr;

    char filename[256]; 

    double loss_probability = 0.02;

    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error_handling("Socket creation failed");
    }

    set_congestion_control(sockfd, algo);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("Connection failed");
    }

        printf("Enter the name of the file to open: ");
        if (fgets(filename, sizeof(filename), stdin) == NULL) {
            error_handling("Error reading filename");
        }

        filename[strcspn(filename, "\n")] = 0;  // Remove newline character

        FILE *fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror("File open failed");
        }
    
    while (1) {
        // Send the file
        send_file(sockfd, fp);
        

        printf("File sent. Send again? (y/n): ");
        char choice = getchar();
        getchar();  // To consume the newline character

        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    // Send exit message
    if (send(sockfd, EXIT_MESSAGE, strlen(EXIT_MESSAGE), 0) == -1) {
        error_handling("Exit message send failed");
    }
    
    fclose(fp);
    close(sockfd);

    return 0;
}
