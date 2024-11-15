#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <aio.h>
#include <errno.h>
#include <ctype.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

struct client {
    int fd;
    struct aiocb aio_cb;
    char buffer[BUFFER_SIZE];
};

struct client clients[MAX_CLIENTS];

void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void remove_client(int index) {
    close(clients[index].fd);
    clients[index].fd = -1;
}

void handle_client_read(int signo, siginfo_t* info, void* context) {
    struct aiocb* req = (struct aiocb*)info->si_value.sival_ptr;

    if (aio_error(req) == 0) {
        ssize_t bytes_read = aio_return(req);
printf("%d\n", bytes_read);
        if (bytes_read > 0) {
            // Process the data
            struct client* cli = (struct client*)req->aio_buf;
printf("%s\n", req->aio_buf);
            to_upper(cli->buffer);
printf("%s\n", cli->buffer);
            // Send response to client
            write(cli->fd, cli->buffer, bytes_read);
printf("%s\n",cli->buffer);
	            // Re-issue the read
            aio_read(req);
        }
        else if (bytes_read == 0) {
            // Client disconnected
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (&clients[i].aio_cb == req) {
                    printf("Client disconnected.\n");
                    remove_client(i);
                    break;
                }
            }
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_un server_addr;
    struct sigaction sa;

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
    }

    // Create server socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Bind socket
    unlink(SOCKET_PATH);  // Remove old socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Set up SIGIO signal handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_client_read;
    sigaction(SIGIO, &sa, NULL);

    printf("Server listening on %s\n", SOCKET_PATH);

    while (1) {
        // Accept new clients
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
            }
            continue;
        }

        // Find a free slot for the client
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1) {
                clients[i].fd = client_fd;

                // Set up AIO control block
                memset(&clients[i].aio_cb, 0, sizeof(struct aiocb));
                clients[i].aio_cb.aio_fildes = client_fd;
                clients[i].aio_cb.aio_buf = clients[i].buffer;
                clients[i].aio_cb.aio_nbytes = BUFFER_SIZE;
                clients[i].aio_cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
                clients[i].aio_cb.aio_sigevent.sigev_signo = SIGIO;
                clients[i].aio_cb.aio_sigevent.sigev_value.sival_ptr = &clients[i].aio_cb;

                // Start reading
                if (aio_read(&clients[i].aio_cb) == -1) {
                    perror("aio_read");
                    close(client_fd);
                    clients[i].fd = -1;
                }
                break;
            }
        }

        if (i == MAX_CLIENTS) {
            printf("Max clients reached. Rejecting new connection.\n");
            close(client_fd);
        }
    }

    // Clean up
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
