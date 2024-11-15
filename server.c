#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024

void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        to_upper(buffer);
        printf("Processed message: %s\n", buffer);
        write(client_fd, buffer, bytes_read);  // Send response back to client
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    }
    else if (bytes_read < 0) {
        perror("Error reading from client");
    }

    close(client_fd);
    exit(EXIT_SUCCESS);  // End child process
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;

    // Create a UNIX domain socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the socket address structure
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Bind the socket to the address
    unlink(SOCKET_PATH);  // Remove any existing socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // Main server loop
    while (1) {
        if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
            perror("accept");
            continue;
        }
        printf("Client connected.\n");

        // Create a new process to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: handle the client
            close(server_fd);  // Child doesn't need the listening socket
            handle_client(client_fd);
        }
        else if (pid > 0) {
            // Parent process: continue accepting clients
            close(client_fd);  // Parent doesn't need the client socket
        }
        else {
            perror("fork");
            close(client_fd);
        }
    }

    // Clean up
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
