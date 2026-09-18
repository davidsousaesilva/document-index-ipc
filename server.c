#include "defs.h"

// retorna -1(erro) N(sucesso)
int readIndexCount()
{
    int fd = open("indexCount.dat", O_RDONLY, O_CREAT, 0666);
    int valor = 0;

    if (fd == -1)
        return 0;
    read(fd, &valor, sizeof(int));
    close(fd);

    return valor;
}

// retorna -1(erro) descritor(sucesso)
int openIndexDat()
{
    int fd = open("index.dat", O_RDWR | O_CREAT, 0666);

    if (fd == -1)
        return -1;

    return fd;
}

// retorna -1(erro) id(sucesso)
int writeDoc(int fd, Doc *doc, int *N)
{
    (*N)++;
    if (fd < 0 || doc == NULL)
        return -1;

    // calcula tamanho atual do ficheiro
    off_t pos = lseek(fd, 0, SEEK_END);
    if (pos == -1)
        return -1;

    // escreve a estrutura no final
    ssize_t escrito = write(fd, doc, sizeof(Doc));
    if (escrito != sizeof(Doc))
        return -1;

    // retorna o índice onde foi escrita (1-based)
    return (pos / sizeof(Doc)) + 1;
}

// retorna -1(erro) 0(nao existe) 1(sucesso)
int removeDoc(int fd, int key, int N)
{
    if (key > N)
        return 0;
    if (fd < 0 || key <= 0)
        return -1;

    // Calcula a posição do documento que será removido (key-1) * sizeof(Doc)
    off_t pos = lseek(fd, (key - 1) * sizeof(Doc), SEEK_SET);
    if (pos == -1)
        return -1;

    Doc temp;
    ssize_t lido = read(fd, &temp, sizeof(Doc));
    if (lido != sizeof(Doc))
        return -1;

    // Marca o documento como inválido
    temp.valid = 0;

    // Volta à posição original e escreve o documento modificado
    pos = lseek(fd, (key - 1) * sizeof(Doc), SEEK_SET);
    if (pos == -1)
        return -1; // erro ao reposicionar no ficheiro

    ssize_t escrito = write(fd, &temp, sizeof(Doc));
    if (escrito != sizeof(Doc))
        return -1; // erro na escrita

    return 1; // sucesso
}

// retorna -1(erro) 0(nao existe) 1(sucesso)
int readDoc(int fd, int key, int N, Res_c *res)
{
    // printf("N dentro do readDoc = %d\n", N);
    if (key > N)
        return 0;

    if (fd < 0 || res == NULL || key <= 0)
        return -1;

    off_t pos = lseek(fd, (key - 1) * sizeof(Doc), SEEK_SET);
    if (pos == -1)
        return -1;

    Doc temp;
    ssize_t lido = read(fd, &temp, sizeof(Doc));
    if (lido != sizeof(Doc))
        return -1;

    if (temp.valid != 1)
        return -1;

    // Copiar os dados para res
    strcpy(res->title, temp.title);
    strcpy(res->author, temp.author);
    strcpy(res->path, temp.path);
    res->year = temp.year;

    return 1; // sucesso
}

// retorna -1(erro) 1(sucesso)
int cleanDoc(int fd, int *N)
{
    if (fd < 0 || N == NULL)
        return -1;

    // Criar um novo ficheiro temporário
    int temp_fd = open("temp_index.dat", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (temp_fd == -1)
        return -1;

    off_t pos = lseek(fd, 0, SEEK_SET);
    if (pos == -1)
    {
        close(temp_fd);
        return -1;
    }

    Doc temp;
    int valid_count = 0; // contador para entradas válidas

    // Ler os documentos do ficheiro original
    while (read(fd, &temp, sizeof(Doc)) == sizeof(Doc))
    {
        if (temp.valid == 1)
        {
            // Escrever no ficheiro temporário apenas as entradas válidas
            if (write(temp_fd, &temp, sizeof(Doc)) != sizeof(Doc))
            {
                close(temp_fd);
                return -1; // erro na escrita
            }
            valid_count++; // Incrementar contador de documentos válidos
        }
    }

    // Fechar o ficheiro temporário
    close(temp_fd);

    // Decrementar o contador N para refletir as entradas válidas
    *N = valid_count;

    // Apagar o ficheiro original e renomear o temporário para o nome do original
    close(fd);
    if (remove("index.dat") == -1)
        return -1;

    if (rename("temp_index.dat", "index.dat") == -1)
        return -1;

    return 1; // sucesso
}

// retorna -1(erro) 1(sucesso)
int writeN(int N)
{
    // Abrir o ficheiro com permissões de escrita, truncando o conteúdo anterior
    int fd = open("indexCount.dat", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
        return -1;

    // Escrever o valor de N no ficheiro
    ssize_t escrito = write(fd, &N, sizeof(int));
    if (escrito != sizeof(int))
    {
        close(fd);
        return -1; // erro na escrita
    }

    // Fechar o ficheiro
    close(fd);

    return 1; // sucesso
}

// retorna -1(erro) 0(nao existe) 1(sucesso)
int getPathDoc(int fd, int key, char *path, int N)
{
    if (key > N)
        return 0;
    if (fd < 0 || key <= 0 || path == NULL)
        return -1;

    off_t pos = lseek(fd, (key - 1) * sizeof(Doc), SEEK_SET);
    if (pos == -1)
        return -1;

    Doc temp;
    ssize_t lido = read(fd, &temp, sizeof(Doc));
    if (lido != sizeof(Doc) || temp.valid != 1)
        return -1;

    strcpy(path, temp.path);
    path[PATH - 1] = '\0'; // garantir null-termination

    return 1;
}

int openClientFifo(int pid) 
{
    char fifo_name[100];
    sprintf(fifo_name, "response_%d", pid);
    int fd = open(fifo_name, O_WRONLY);
    if (fd == -1) {
        perror("SERVER: Erro ao abrir fifo do cliente");
        return -1;
    }
    return fd;
}


int main(int argc, char *argv[])
{
    int N = readIndexCount(); // número de entradas no ficheiro "index.dat"
    int fd = openIndexDat();  // abre o ficheiro "index.dat"

    mkfifo("request", 0666);

    int fd_request = open("request", O_RDONLY);
    if (fd_request == -1)
    {
        perror("SERVER: Erro ao abrir request");
        return -1;
    }

    int dummy = open("request", O_WRONLY);
    if (dummy == -1)
    {
        perror("SERVER: Erro ao abrir dummy");
        return -1;
    }

    Msg mensagem;
    int active_children = 0; // variavel para controlar numero de forks ativos

    while (read(fd_request, &mensagem, sizeof(Msg)) > 0)
    {
        if (mensagem.type == 'f')
        {
            close(dummy);
            continue;
        }

        else if (mensagem.type == 'a') // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   A
        {
            if (active_children >= MAX_CHILDREN)
            {
                wait(NULL);
                active_children--;
            }

            /////////////////////////////////////////////////////////////////// processa no PAI
            Doc doc;
            doc.valid = 1;
            strcpy(doc.title, mensagem.title);
            strcpy(doc.author, mensagem.author);
            strcpy(doc.path, mensagem.path);
            doc.year = mensagem.year;
            int id = writeDoc(fd, &doc, &N);
            /////////////////////////////////////////////////////////////////// processa no PAI

            pid_t child = fork();
            if (child == 0) // PROCESSO FILHO
            {
                char fifo_name[100];
                sprintf(fifo_name, "response_%d", mensagem.pid);

                int fd_response = open(fifo_name, O_WRONLY);
                if (fd_response == -1)
                {
                    perror("SERVER: Erro ao abrir fifo do cliente");
                    _exit(1);
                }

                Res_a res_a;
                res_a.key = id;
                write(fd_response, &res_a, sizeof(Res_a));
                close(fd_response);

                printf("SERVER: Respondido para PID %d\n", mensagem.pid);
                _exit(0);
            }
            else if (child > 0) // VOLTA AO PROCESSO PAI
            {
                active_children++;
            }
        }
        else if (mensagem.type == 'c') // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   C
        {
            if (active_children >= MAX_CHILDREN)
            {
                wait(NULL);
                active_children--;
            }

            pid_t child = fork();
            if (child == 0) // PROCESSO FILHO
            {
                /////////////////////////////////////////////////////////////////// processa no FILHO
                // Abrir o ficheiro novamente dentro do filho
                int fd_local = openIndexDat();
                if (fd_local == -1)
                {
                    perror("SERVER: Erro ao abrir index.dat no filho");
                    _exit(1);
                }

                Res_c res_c;
                strcpy(res_c.title, "");
                strcpy(res_c.author, "");
                strcpy(res_c.path, "");
                res_c.year = -1;

                readDoc(fd_local, mensagem.key, N, &res_c);
                close(fd_local);
                /////////////////////////////////////////////////////////////////// processa no FILHO
                int fd_response = openClientFifo(mensagem.pid);

                write(fd_response, &res_c, sizeof(Res_c));
                close(fd_response);

                printf("SERVER: Respondido para PID %d\n", mensagem.pid);
                _exit(0);
            }
            else if (child > 0) // VOLTA AO PROCESSO PAI
            {
                active_children++;
            }
        }

        else if (mensagem.type == 'd') // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   D
        {
            if (active_children >= MAX_CHILDREN)
            {
                wait(NULL);
                active_children--;
            }

            /////////////////////////////////////////////////////////////////// processa no PAI
            int res = removeDoc(fd, mensagem.key, N);
            Res_d res_d;
            res_d.key = mensagem.key;
            if (res == 1)
                res_d.boolean = 1;
            else
                res_d.boolean = 0;
            /////////////////////////////////////////////////////////////////// processa no PAI

            pid_t child = fork();
            if (child == 0) // PROCESSO FILHO
            {
                int fd_response = openClientFifo(mensagem.pid);

                write(fd_response, &res_d, sizeof(Res_d));
                close(fd_response);

                printf("SERVER: Respondido para PID %d\n", mensagem.pid);
                _exit(0);
            }
            else if (child > 0) // VOLTA AO PROCESSO PAI
            {
                active_children++;
            }
        }
        else if (mensagem.type == 'l')
        {
            if (active_children >= MAX_CHILDREN)
            {
                wait(NULL);
                active_children--;
            }

            pid_t child = fork();
            if (child == 0)
            {
                int fd_local = openIndexDat();
                if (fd_local == -1)
                {
                    perror("SERVER: Erro ao abrir index.dat no filho");
                    _exit(1);
                }

                Doc doc;
                off_t pos = lseek(fd_local, (mensagem.key - 1) * sizeof(Doc), SEEK_SET);
                if (pos == -1)
                {
                    perror("SERVER: Erro ao reposicionar no ficheiro");
                    _exit(1);
                }

                ssize_t lido = read(fd_local, &doc, sizeof(Doc));
                if (doc.valid != 1 || lido != sizeof(Doc))
                {
                    printf("SERVER: Documento inválido para PID %d\n", mensagem.pid);
                    Res_l res_l;
                    res_l.count = -1;

                    char fifo_name[100];
                    sprintf(fifo_name, "response_%d", mensagem.pid);
                    int fd_response = open(fifo_name, O_WRONLY);
                    if (fd_response != -1) {
                        write(fd_response, &res_l, sizeof(Res_l));
                        close(fd_response);
                    }
                    _exit(0);
                }

                close(fd_local);

                int pipe_fd[2];
                if (pipe(pipe_fd) == -1)
                {
                    perror("SERVER: Erro ao criar pipe");
                    _exit(1);
                }

                pid_t pid_grep = fork();
                if (pid_grep == -1)
                {
                    perror("SERVER: Erro no fork do grep");
                    _exit(1);
                }

                if (pid_grep == 0)
                {
                    // Processo grep
                    close(pipe_fd[0]); // Fecha leitura
                    dup2(pipe_fd[1], 1); // stdout -> pipe
                    close(pipe_fd[1]);

                    char full_path[512];
                    sprintf(full_path, "%s/%s", argv[1], doc.path); // <- base_path enviado no Msg

                    execlp("grep", "grep", mensagem.keyword, full_path, (char *)NULL);
                    perror("SERVER: Erro ao executar grep");
                    _exit(1);
                }

                // Criar pipe para leitura do resultado do wc
                int wc_pipe[2];
                if (pipe(wc_pipe) == -1)
                {
                    perror("SERVER: Erro ao criar pipe para wc");
                    _exit(1);
                }

                pid_t pid_wc = fork();
                if (pid_wc == -1)
                {
                    perror("SERVER: Erro no fork do wc");
                    _exit(1);
                }

                if (pid_wc == 0)
                {
                    // Processo wc
                    close(pipe_fd[1]); // Fecha escrita de grep
                    dup2(pipe_fd[0], 0);  // stdin <- grep
                    close(pipe_fd[0]);

                    close(wc_pipe[0]); // fecha leitura do novo pipe
                    dup2(wc_pipe[1], 1); // stdout -> wc_pipe
                    close(wc_pipe[1]);

                    execlp("wc", "wc", "-l", (char *)NULL);
                    perror("SERVER: Erro ao executar wc");
                    _exit(1);
                }

                // Processo original (pai dos dois forks)
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                close(wc_pipe[1]); // só vamos ler do wc_pipe

                waitpid(pid_grep, NULL, 0);
                waitpid(pid_wc, NULL, 0);

                char buffer[64];
                ssize_t n = read(wc_pipe[0], buffer, sizeof(buffer) - 1);
                close(wc_pipe[0]);

                Res_l res_l;
                res_l.count = 0;

                if (n > 0)
                {
                    buffer[n] = '\0';
                    res_l.count = atoi(buffer); // extrai número de linhas
                }

                int fd_response = openClientFifo(mensagem.pid);

                write(fd_response, &res_l, sizeof(Res_l));
                close(fd_response);

                printf("SERVER: Respondido para PID %d\n", mensagem.pid);
                _exit(0);
            }
            else if (child > 0)
            {
                active_children++;
            }
        }
        else if(mensagem.type == 's') // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   S
        {
            if (active_children >= MAX_CHILDREN)
            {
                wait(NULL);
                active_children--;
            }

            pid_t child = fork();
            if (child == 0)
            {
                int fd_local = openIndexDat();
                if (fd_local == -1)
                {
                    perror("SERVER: Erro ao abrir index.dat");
                    _exit(1);
                }

                int result_pipe[2];
                if (pipe(result_pipe) == -1)
                {
                    perror("SERVER: Erro ao criar pipe de resultados");
                    _exit(1);
                }

                int DocId = 1;
                int active = 0;
                int max_concurrent = mensagem.nr_processes;

                Res_s res_s;
                res_s.total = 0;
                res_s.keys = NULL;
                
                Doc doc;
                off_t pos = lseek(fd_local, 0, SEEK_SET);
                if (pos == -1)
                {
                    perror("SERVER: Erro ao reposicionar no ficheiro");
                    _exit(1);
                }
                while (read(fd_local, &doc, sizeof(Doc)) == sizeof(Doc)) //vamos ler todos os ficheiros
                {
                    if (doc.valid == 1)  //Se o documento for válido vai ser tratado
                    {
                        while (active >= max_concurrent)
                        {
                            wait(NULL);
                            active--;
                        }

                        pid_t pid_proc = fork();
                        if (pid_proc == 0)
                        {
                            char full_path[512];
                            sprintf(full_path, "%s/%s", argv[1], doc.path);     

                            int pipe_fd[2];
                            if (pipe(pipe_fd) == -1)
                            {
                                perror("SERVER: Erro ao criar pipe");
                                _exit(1);
                            }

                            pid_t pid_grep = fork();
                            if (pid_grep == -1)
                            {
                                perror("SERVER: Erro no fork do grep");
                                _exit(1);
                            }

                            if (pid_grep == 0)
                            {                            
                                close(pipe_fd[0]);   //Fecha leitura
                                dup2(pipe_fd[1], 1); // stdout -> pipe
                                close(pipe_fd[1]);

                                execlp("grep", "grep", mensagem.keyword, full_path, (char *)NULL);
                                perror("SERVER: Erro ao executar grep");
                                _exit(1);
                            }

                            int wc_pipe[2];
                            if (pipe(wc_pipe) == -1)
                            {
                                perror("SERVER: Erro ao criar pipe para wc");
                                _exit(1);
                            }

                            pid_t pid_wc = fork();
                            if (pid_wc == -1)
                            {
                                perror("SERVER: Erro no fork do wc");
                                _exit(1);
                            }

                            if (pid_wc == 0)
                            {
                                // Processo wc
                                close(pipe_fd[1]); // Fecha escrita de grep
                                dup2(pipe_fd[0], 0);  // stdin <- grep
                                close(pipe_fd[0]);

                                close(wc_pipe[0]); // fecha leitura do novo pipe
                                dup2(wc_pipe[1], 1); // stdout -> wc_pipe
                                close(wc_pipe[1]);

                                execlp("wc", "wc", "-l", (char *)NULL);
                                perror("SERVER: Erro ao executar wc");
                                _exit(1);
                            }

                            close(pipe_fd[0]);
                            close(pipe_fd[1]);
                            close(wc_pipe[1]);

                            waitpid(pid_grep, NULL, 0);
                            waitpid(pid_wc, NULL, 0);

                            char buffer[32];
                            read(wc_pipe[0], buffer, sizeof(buffer));
                            close(wc_pipe[0]);

                            int count = atoi(buffer);
                            if (count > 0)
                            {
                                write(result_pipe[1], &DocId, sizeof(int));
                            }
                            _exit(0);
                        }
                        else if (pid_proc > 0)
                        {
                            active++;
                        }
                    }
                    DocId++;
                }

                close(fd_local);
                // Esperar os filhos
                while (active > 0)
                {
                    wait(NULL);
                    active--;
                }

                close(result_pipe[1]);
                int id;
                while (read(result_pipe[0], &id, sizeof(int)) == sizeof(int))
                {
                    res_s.keys = realloc(res_s.keys, (res_s.total + 1) * sizeof(int));
                    if (res_s.keys == NULL)
                    {
                        perror("SERVER: Erro ao alocar memória");
                        close(result_pipe[0]);
                        _exit(1);
                    }

                    res_s.keys[res_s.total++] = id;
                }
                close(result_pipe[0]);

                int fd_response = openClientFifo(mensagem.pid);

                write(fd_response, &res_s.total, sizeof(int));
                if (res_s.total > 0)
                {
                    write(fd_response, res_s.keys, res_s.total * sizeof(int));
                }

                printf("SERVER: Respondido para PID %d\n", mensagem.pid);
                free(res_s.keys);
                _exit(0);
            }
            else if (child > 0)
            {
                active_children++;
            }
        }
        else
            continue;
    }

    // Espera por todos os filhos restantes
    while (wait(NULL) > 0)
        ;

    /////////////////////////////////////////////////////////////////// ANTES DE ENCERRAR
    cleanDoc(fd, &N); // limpa as entradas apagadas do ficheiro "index.dat" e atualiza N
    writeN(N);        // reescreve N no ficheiro "indexCount.dat"
    close(fd_request);
    unlink("request");
    return 0;
    /////////////////////////////////////////////////////////////////// ANTES DE ENCERRAR
}
