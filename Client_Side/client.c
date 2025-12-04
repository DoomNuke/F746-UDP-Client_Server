#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.h"

int main()
{
    int choice;
    int sockfd;
    char buf[128];
    char recv[128];
    struct sockaddr_in server;
    ssize_t sent;
    ssize_t received;
    socklen_t recv_len = sizeof(server);


    sockfd = (socket(AF_INET, SOCK_DGRAM, 0));
    if(sockfd < 0)
    {
        printf("Error creating the socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, "192.168.1.40",&server.sin_addr);

    printf("Welcome! What would you like to check?\n");


while(1)
{
    printf("1. UART\n");
    printf("2. I2C\n");
    printf("3. SPI\n");
    scanf("%d", &choice);

    switch(choice)
    {
        case UART_TEST:
        {
            strcpy(buf, "UART");
            break;
        }
        case I2C_TEST:
        {
            strcpy(buf, "I2C");
            break;
        }
        case SPI_TEST:
        {
            strcpy(buf, "SPI");
            break;
        }
        default:

        printf("Invalid choice\n");
        continue;
    }

    sent = sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&server, sizeof(server));

    if (sent < 0)
    {
        perror("Error Sending Operation");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Operation Sent for check, waiting for results...\n");

    received = recvfrom(sockfd, recv, sizeof(recv), 0, (struct sockaddr *)&server, &recv_len);

    if (received < 0)
    {
        perror("Error receiving response");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    /*
    * for null termination
    */
    recv[received] = '\0';
    printf("Results of %d are... %s\n", choice, recv);
}

    close(sockfd);
    return 0;
}