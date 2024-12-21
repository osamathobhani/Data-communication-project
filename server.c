#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int id;
    char name[50];
    char department[50];
} Employee;

typedef struct {
    int id;
    SOCKET socket;
} Client;


Employee employees[100];
int employee_count = 0;


Client clients[100];
int client_count = 0;

int client_id_counter = 1;

DWORD WINAPI handle_client(void *data) {
    Client *client_data = (Client *)data;
    SOCKET sock = client_data->socket;
    int client_id = client_data->id;

    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE];

    int valread;
    while ((valread = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (strncmp(buffer, "ADD", 3) == 0) {
           
            char name[50], department[50];
            int id;
            sscanf(buffer + 4, "%d %s %s", &id, name, department);

            
            int found = 0;
            for (int i = 0; i < employee_count; i++) {
                if (employees[i].id == id) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                sprintf(response, "Employee with ID %d already exists! Cannot add again.\n", id);
                printf("Client %d attempted to add employee with duplicate ID %d\n", client_id, id);
            } else {
                employees[employee_count].id = id;
                strcpy(employees[employee_count].name, name);
                strcpy(employees[employee_count].department, department);
                employee_count++;
                sprintf(response, "Employee %s added successfully!\n", name);
                printf("Client %d added employee: ID=%d, Name=%s, Department=%s\n",
                       client_id, id, name, department);
            }
        } else if (strncmp(buffer, "GET", 3) == 0) {
           
            int id;
            if (sscanf(buffer + 4, "%d", &id) != 1) {
                sprintf(response, "Invalid GET command. Please specify a valid Employee ID.\n");
            } else {
                int found = 0;
                for (int i = 0; i < employee_count; i++) {
                    if (employees[i].id == id) {
                        sprintf(response, "ID: %d, Name: %s, Department: %s\n",
                                employees[i].id, employees[i].name, employees[i].department);
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    sprintf(response, "Employee with ID %d not found!\n", id);
                }
            }
            printf("Client %d searched for employee ID=%d\n", client_id, id);
        } else if (strncmp(buffer, "LIST", 4) == 0) {
           
            if (employee_count == 0) {
                sprintf(response, "No employees in the system.\n");
            } else {
                sprintf(response, "List of all employees:\n");
                for (int i = 0; i < employee_count; i++) {
                    char employee_info[200];
                    sprintf(employee_info, "ID: %d, Name: %s, Department: %s\n",
                            employees[i].id, employees[i].name, employees[i].department);
                    strcat(response, employee_info);
                }
            }
            printf("Client %d requested the list of all employees.\n", client_id);
        } else if (strncmp(buffer, "UPDATE", 6) == 0) {
          
            int id;
            char name[50], department[50];
            if (sscanf(buffer + 7, "%d %s %s", &id, name, department) != 3) {
                sprintf(response, "Invalid UPDATE command. Please provide Employee ID, Name, and Department.\n");
            } else {
                int found = 0;
                for (int i = 0; i < employee_count; i++) {
                    if (employees[i].id == id) {
                        strcpy(employees[i].name, name);
                        strcpy(employees[i].department, department);
                        sprintf(response, "Employee ID %d updated successfully!\n", id);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    sprintf(response, "Employee with ID %d not found! Cannot update.\n", id);
                }
            }
            printf("Client %d attempted to update employee ID=%d\n", client_id, id);
        } else if (strncmp(buffer, "DELETE", 6) == 0) {
            
            int id;
            if (sscanf(buffer + 7, "%d", &id) != 1) {
                sprintf(response, "Invalid DELETE command. Please provide a valid Employee ID.\n");
            } else {
                int found = 0;
                for (int i = 0; i < employee_count; i++) {
                    if (employees[i].id == id) {
                        for (int j = i; j < employee_count - 1; j++) {
                            employees[j] = employees[j + 1]; 
                        }
                        employee_count--;
                        sprintf(response, "Employee ID %d deleted successfully!\n", id);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    sprintf(response, "Employee with ID %d not found! Cannot delete.\n", id);
                }
            }
            printf("Client %d attempted to delete employee ID=%d\n", client_id, id);
        } else {
            sprintf(response, "Invalid command! Please use ADD, GET <ID>, LIST, UPDATE <ID> <Name> <Department>, or DELETE <ID>.\n");
            printf("Client %d sent an invalid command: %s\n", client_id, buffer);
        }

        send(sock, response, strlen(response), 0);
        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("Client %d disconnected.\n", client_id);
    closesocket(sock);
    free(client_data);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_fd);
        return 1;
    }
    printf("Bind done.\n");

    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_fd);
        return 1;
    }
    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            printf("Accept failed. Error Code : %d\n", WSAGetLastError());
            continue;
        }

        int client_id = client_id_counter++;
        printf("New client connected: Client ID=%d\n", client_id);

        Client *client_data = (Client *)malloc(sizeof(Client));
        client_data->id = client_id;
        client_data->socket = client_socket;

        clients[client_count++] = *client_data;

        HANDLE thread = CreateThread(NULL, 0, handle_client, client_data, 0, NULL);
        if (thread == NULL) {
            printf("Thread creation failed. Error Code : %d\n", GetLastError());
            closesocket(client_socket);
            free(client_data);
        } else {
            CloseHandle(thread);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
