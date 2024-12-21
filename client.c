#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed. Error Code : %d\n", WSAGetLastError());
        closesocket(sock);
        return 1;
    }
    printf("Connected to server.\n");

    while (1) {
        printf("Enter command: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (send(sock, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            printf("Send failed. Error Code : %d\n", WSAGetLastError());
            break;
        }

        int valread = recv(sock, buffer, BUFFER_SIZE, 0);
        if (valread == SOCKET_ERROR) {
            printf("Recv failed. Error Code : %d\n", WSAGetLastError());
            break;
        }
        buffer[valread] = '\0';

        printf("%s\n", buffer);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
