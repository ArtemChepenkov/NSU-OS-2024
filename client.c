#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Подключаемся к серверу
    if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Вводим сообщение для отправки
    printf("Введите сообщение: ");
 while(1){   fgets(buffer, BUFFER_SIZE, stdin);

    // Отправляем сообщение серверу
    if (write(sock, buffer, strlen(buffer)) == -1) {
        perror("write");
        close(sock);
        exit(EXIT_FAILURE);
    }
}
    printf("Сообщение отправлено серверу\n");

    close(sock);
    return 0;
}
