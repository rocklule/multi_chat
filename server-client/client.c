#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void init(char *fifo_read_name, char *fifo_write_name);
int fd_server, fd_write, fd_read;

int main(int argc, char const *argv[])
{
    unsigned int child_pid;
    char buf[1024];
    char fifo_read_name[32] = "";
    char fifo_write_name[32] = "";
    
    init(fifo_read_name, fifo_write_name);
    
    if ((child_pid = fork()) > 0)
    {
        FILE *fp_write = fdopen(fd_write, "w");
        while (memset(buf, 0, 1024), fgets(buf, 1024, stdin) != NULL)
        {
            fprintf(fp_write, "from %d: %s", getpid(), buf );
            fflush(fp_write);
        }
        char temp_char[50];
        sprintf(temp_char, "99999%u", getpid());
        FILE * server = fdopen(fd_server, "w");
        fprintf(server, "%s", temp_char); fflush(server);
        kill(child_pid, SIGQUIT);
        close(fd_write);
        wait(NULL);
        unlink(fifo_write_name);
        unlink(fifo_read_name);
    }
    else
    {
        while (memset(buf, 0, 1024), read(fd_read, buf, 1024) > 0)
        {
            write(1, buf, strlen(buf));
        }
    }
    return 0;
}


void init(char *fifo_read_name, char *fifo_write_name)
{
    sprintf(fifo_read_name, "%u_read.fifo", getpid());
    sprintf(fifo_write_name, "%u_write.fifo", getpid());
    mkfifo(fifo_read_name, 0777);
    mkfifo(fifo_write_name, 0777);
    printf("connecting server...\n");
    fd_server = open("server.fifo", O_WRONLY);
    if (fd_server == -1)
    {
        perror("failed to connect server");
        exit(-1);
    }
    FILE *fp_server = fdopen(fd_server, "w");
    fprintf(fp_server, "%u", getpid());
    fflush(fp_server);
printf("pid send!\n");
    fd_write = open(fifo_write_name, O_WRONLY);
    fd_read = open(fifo_read_name, O_RDONLY);
    printf("client %u connected successfully!\n", getpid());
}









