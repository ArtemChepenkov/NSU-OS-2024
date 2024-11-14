#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <aio.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024

sigjmp_buf to_exit;
struct sigaction sig_io_handler_action;

void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}


void sig_io_handler(int signo, siginfo_t* info, void* context) {
    struct aiocb* req;
    if (signo != SIGIO || info->si_signo != SIGIO) {
        return;
    }

    req = (struct aiocb*)info->si_value.sival_ptr;

    if (aio_error(req) == 0) {
        size_t size;
        size = aio_return(req);
        if (size == 0) {
            siglongjmp(to_exit, 1);
        }

        write(1, req->aio_buf, size);
        
        aio_read(req);
    }
}

int main() {
    static struct aiocb readrq;
    static const struct aiocb* readrqv[2] = { &readrq, NULL };


    memset(&sig_io_handler_action, 0, sizeof(sig_io_handler_action));
    sig_io_handler_action.sa_sigaction = sig_io_handler;
    sig_io_handler_action.sa_flags = SA_SIGINFO;
//    sigiohandleraction.sa_mask = set;
    sigaction(SIGIO, &sig_io_handler_action, NULL);

    readrq.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    readrq.aio_sigevent.sigev_signo = SIGIO;
    readrq.aio_sigevent.sigev_value.sival_ptr = &readrq;

    if (aio_read(&readrq)) {
        perror("aio_read");
        exit(1);
    }

    if (!sigsetjmp(to_exit, 1)) {
        while (1) sigpause(SIGIO);
    }

    return 0;
}
