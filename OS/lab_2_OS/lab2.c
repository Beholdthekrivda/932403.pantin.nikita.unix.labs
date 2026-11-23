#include <stdio.h>       // printf, perror, fprintf
#include <stdlib.h>      // EXIT_SUCCESS, -1
#include <string.h>      // memset
#include <unistd.h>      // close
#include <errno.h>       // errno, EINTR
#include <sys/types.h>   // ssize_t, системные типы
#include <sys/socket.h>  // socket, bind, listen, accept, recv
#include <netinet/in.h>  // sockaddr_in, htonl, htons, INADDR_ANY
#include <signal.h>      // sigaction, sigemptyset, sigaddset, sigprocmask, sig_atomic_t, SIGHUP, SIG_BLOCK
#include <sys/select.h>  // pselect, fd_set, FD_ZERO, FD_SET

volatile sig_atomic_t was_sighup = 0;

static void sighup_handler(int signal_number)
{
    was_sighup = 1;
}

int create_socket_and_bind(int port)
{
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        return -1;
    }

    int opt = 1;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 16) == -1)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int reg_and_block_signal(sigset_t *original_mask)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = sighup_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    
    if (sigaction(SIGHUP, &action, NULL) == -1)
    {
        perror("sigaction");
        return -1;
    }

    sigset_t blocked_mask;
    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &blocked_mask, original_mask) == -1)
    {
        perror("sigprocmask");
        return -1;
    }

    return 0;
}

int create_connection(int server_fd, sigset_t *original_mask)
{
    int client_fd = -1, new_fd = -1;
    fd_set read_fds;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        if (client_fd != -1) FD_SET(client_fd, &read_fds);

        int max_fd = (server_fd > client_fd) ? server_fd : client_fd;

        if (pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, original_mask) == -1)
        {
            if (errno == EINTR)
            {
                if (was_sighup)
                {
                    printf("Получен SIGHUP\n");
                    was_sighup = 0;
                }
                continue;
            }
            perror("pselect");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            new_fd = accept(server_fd, NULL, NULL);

            if (new_fd == -1) return -1;
            else
            {
                printf("Получен новый fd клиента: %d\n", new_fd);
                if (client_fd == -1) client_fd = new_fd;
                else close(new_fd);
            }
        }

        if (client_fd != -1 && FD_ISSET(client_fd, &read_fds))
        {
            char buf[1024];
            ssize_t received = recv(client_fd, buf, sizeof(buf), 0);

            if (received > 0)
            {
                printf("Полученно байт: %zd\n", received);
            }
            else if (received == 0)
            {
                printf("Клиент закрыл соединение\n");
                close(client_fd);
                client_fd = -1;
            }
            else
            {
                perror("recv");
                close(client_fd);
                client_fd = -1;
            }
        }
    }

    if (client_fd != -1) close(client_fd);
    return -1;
}

int main()
{
    int port = 8080, server_fd = create_socket_and_bind(port);
    if (server_fd == -1) return -1;

    sigset_t original_mask;
    if (reg_and_block_signal(&original_mask) == -1)
    {
        close(server_fd);
        return -1;
    }

    int rc = create_connection(server_fd, &original_mask);

    close(server_fd);

    if (rc == -1) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}