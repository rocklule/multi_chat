#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void child_death(int sig);
pid_t pid1, pid2;
int pipe_12, pipe_32, pipe_21, pipe_23;
FILE * fp21, * fp23;

int main(int argc, char const *argv[])
{
    char buf[1024];

    if(0==(pid1=fork()))
    {
        pipe_12 = open("rec_2_1.fifo", O_RDONLY);
        printf("ok, 21\n");

        while(memset(buf, 0, 1024),read(pipe_12, buf, 1024)> 0)
        {
            if(buf[0]=='/' && buf[1]=='/')
                exit(0);
            write(1, buf, strlen(buf));
        }


    }
    else if(0==(pid2=fork()))
    {
        pipe_32 = open("rec_2_2.fifo", O_RDONLY);
        printf("ok, 22\n");

        while(memset(buf, 0, 1024),read(pipe_32, buf, 1024)> 0)
        {
            if(buf[0]=='/' && buf[1]=='/')
                exit(0);
            write(1, buf, strlen(buf));
        }

    }
    else
    {
        signal(20, child_death);

        pipe_21 = open("rec_1_1.fifo", O_WRONLY);
        pipe_23 = open("rec_3_2.fifo", O_WRONLY);
        fp21 = fdopen(pipe_21,"w");
        fp23 = fdopen(pipe_23,"w");
        printf("ok, 23\n");

        while(memset(buf, 0, 1024),fgets(buf, 1024,stdin)!= NULL)
        {
            fprintf(fp21,"from c2: %s",buf); fflush(fp21);
            fprintf(fp23,"from c2: %s",buf); fflush(fp23);  
        }

        child_death(0);
    }

    return 0;
}

void child_death(int sig)
{
    printf("2,end.\n");

    fprintf(fp21,"//");  fflush(fp21);
    fprintf(fp23,"//");  fflush(fp23); 

    kill(pid1, 2);  
    kill(pid2, 2);     

    close(pipe_21);
    close(pipe_23);

    wait(NULL);

}
