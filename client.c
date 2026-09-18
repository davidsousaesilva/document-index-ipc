#include "defs.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Missing arguments.\n");
        return -1;
    }

    pid_t pid = getpid();
    char fifo_name[100];
    sprintf(fifo_name, "response_%d", pid);
    mkfifo(fifo_name, 0666);

    int fd_request = open("request", O_WRONLY);
    if (fd_request == -1)
    {
        perror("CLIENT: Erro ao abrir request\n");
        return -1;
    }

    if (strcmp("-a", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   A
    {
        if (argc < 6)
        {
            printf("Missing arguments. The expected arguments are: title, author, path, year.\n");
            return -1;
        }

        Msg msg;
        msg.type = 'a';
        msg.pid = pid;
        strcpy(msg.title, argv[2]);
        strcpy(msg.author, argv[3]);
        strcpy(msg.path, argv[5]);
        msg.year = atoi(argv[4]);
        msg.key = -1;
        strcpy(msg.keyword, "");
        msg.nr_processes = -1;

        write(fd_request, &msg, sizeof(Msg));
        close(fd_request);

        int fd_response = open(fifo_name, O_RDONLY);
        if (fd_response == -1)
        {
            perror("CLIENT: Erro ao abrir fifo de resposta\n");
            return -1;
        }

        Res_a recebido;
        read(fd_response, &recebido, sizeof(Res_a));
        printf("Document %d indexed\n", recebido.key);

        close(fd_response);
        unlink(fifo_name);
        return 0;
    }

    else if (strcmp("-c", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  C
    {
        if (argc < 3)
        {
            printf("Missing arguments.\n");
            return -1;
        }

        Msg msg;
        msg.type = 'c';
        msg.pid = pid;
        strcpy(msg.title, "");
        strcpy(msg.author, "");
        strcpy(msg.path, "");
        msg.year = -1;
        msg.key = atoi(argv[2]);
        strcpy(msg.keyword, "");
        msg.nr_processes = -1;

        write(fd_request, &msg, sizeof(Msg));
        close(fd_request);

        int fd_response = open(fifo_name, O_RDONLY);
        if (fd_response == -1)
        {
            perror("CLIENT: Erro ao abrir fifo de resposta\n");
            return -1;
        }

        Res_c recebido;
        read(fd_response, &recebido, sizeof(Res_c));
        printf("Title: %s\n", recebido.title);
        printf("Authors: %s\n", recebido.author);
        printf("Year: %d\n", recebido.year);
        printf("Path: %s\n", recebido.path);

        close(fd_response);
        unlink(fifo_name);
        return 0;
    }

    else if (strcmp("-d", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  D
    {
        if (argc < 3)
        {
            printf("Missing arguments.\n");
            return -1;
        }

        Msg msg;
        msg.type = 'd';
        msg.pid = pid;
        strcpy(msg.title, "");
        strcpy(msg.author, "");
        strcpy(msg.path, "");
        msg.year = -1;
        msg.key = atoi(argv[2]);
        strcpy(msg.keyword, "");
        msg.nr_processes = -1;

        write(fd_request, &msg, sizeof(Msg));
        close(fd_request);

        int fd_response = open(fifo_name, O_RDONLY);
        if (fd_response == -1)
        {
            perror("CLIENT: Erro ao abrir fifo de resposta\n");
            return -1;
        }

        Res_d recebido;
        read(fd_response, &recebido, sizeof(Res_d));
        printf("Index entry %d %s\n", recebido.key, recebido.boolean ? "deleted" : "failed to delete");

        close(fd_response);
        unlink(fifo_name);
        return 0;
    }

    else if (strcmp("-l", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  L
    {
        if (argc < 4)
        {
            printf("Missing arguments.\n");
            return -1;
        }

        Msg msg;
        msg.type = 'l';
        msg.pid = pid;
        strcpy(msg.title, "");
        strcpy(msg.author, "");
        strcpy(msg.path, "");
        msg.year = -1;
        msg.key = atoi(argv[2]);
        strcpy(msg.keyword, argv[3]);
        msg.nr_processes = -1;

        write(fd_request, &msg, sizeof(Msg));
        close(fd_request);

        int fd_response = open(fifo_name, O_RDONLY);
        if (fd_response == -1)
        {
            perror("CLIENT: Erro ao abrir fifo de resposta\n");
            return -1;
        }

        Res_l recebido;
        read(fd_response, &recebido, sizeof(Res_l));
        
        if (recebido.count == -1)
            printf("Documento inválido\n");
        else     
            printf("%d\n", recebido.count);

        close(fd_response);
        unlink(fifo_name);
        return 0;
    }

    else if (strcmp("-s", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  S
    {
        if (argc < 3)
        {
            printf("Missing arguments.\n");
            return -1;
        }

        else if (argc == 3)
        {
            Msg msg;
            msg.type = 's';
            msg.pid = pid;
            strcpy(msg.title, "");
            strcpy(msg.author, "");
            strcpy(msg.path, "");
            msg.year = -1;
            msg.key = -1;
            strcpy(msg.keyword, argv[2]);
            msg.nr_processes = 1;

            write(fd_request, &msg, sizeof(Msg));
            close(fd_request);

            int fd_response = open(fifo_name, O_RDONLY);
            if (fd_response == -1)
            {
                perror("CLIENT: Erro ao abrir fifo de resposta\n");
                return -1;
            }

            int total;
            int* keys = NULL;
            // Ler a resposta obtida
            read(fd_response, &total, sizeof(int));

            if (total > 0) 
            {
                keys = malloc(total * sizeof(int));
                read(fd_response, keys, total * sizeof(int));
            }
            close(fd_response);
            unlink(fifo_name);

            if (total == 0)
            {
                printf("[]\n");
                return 0;
            }

            printf("[");
            for (int i = 0; i < total; i++)
            {
                printf("%d", keys[i]);
                if (i < total - 1)
                printf(", ");
            }
            printf("]\n");
            return 0;
        }

        else if (argc == 4)
        {
            Msg msg;
            msg.type = 's';
            msg.pid = pid;
            strcpy(msg.title, "");
            strcpy(msg.author, "");
            strcpy(msg.path, "");
            msg.year = -1;
            msg.key = -1;
            strcpy(msg.keyword, argv[2]);
            msg.nr_processes = atoi(argv[3]);

            write(fd_request, &msg, sizeof(Msg));
            close(fd_request);

            int fd_response = open(fifo_name, O_RDONLY);
            if (fd_response == -1)
            {
                perror("CLIENT: Erro ao abrir fifo de resposta\n");
                return -1;
            }
            int total;
            int* keys = NULL;
            // Ler a resposta obtida
            read(fd_response, &total, sizeof(int));

            if (total > 0) 
            {
                keys = malloc(total * sizeof(int));
                read(fd_response, keys, total * sizeof(int));
            }
            close(fd_response);
            unlink(fifo_name);

            if (total == 0)
            {
                printf("[]\n");
                return 0;
            }

            printf("[");
            for (int i = 0; i < total; i++)
            {
                printf("%d", keys[i]);
                if (i < total - 1)
                printf(", ");
            }
            printf("]\n");
            return 0;
        }
    }
    else if (strcmp("-f", argv[1]) == 0) // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  F
    {
        Msg msg;
        msg.type = 'f';
        msg.pid = -1;
        strcpy(msg.title, "");
        strcpy(msg.author, "");
        strcpy(msg.path, "");
        msg.year = -1;
        msg.key = -1;
        strcpy(msg.keyword, "");
        msg.nr_processes = -1;

        write(fd_request, &msg, sizeof(Msg));
        close(fd_request);

        printf("Server is shutting down\n");
        unlink(fifo_name);
        return 0;
    }
}
