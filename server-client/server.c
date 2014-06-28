#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

typedef struct tag
{
    int s_read ;
    int s_write ;
} node;
node client[500];

int fd_server;
int inner_pipe[2];   //0读，1写
FILE * in_p;
unsigned int cli_pid[500];
int indx;
unsigned int pid;
fd_set read_set ,temp_set;

void refresh(int num);
void init(void);
void login_server(void);



int main(int argc, char const *argv[])
{
    pipe(inner_pipe);
    memset(cli_pid, 0, 500 * sizeof(unsigned int));
    memset(client,0, sizeof(client));

    in_p = fdopen(inner_pipe[0], "r");
    
    // init();
    
    if (0 == fork())
    {
        close(inner_pipe[0]);
        login_server();
    }
    else
    {
        // close(inner_pipe[1]);
        signal(19, refresh);
        in_p = fdopen(inner_pipe[0], "r");
        FD_ZERO(&read_set);
// printf("father:%u\n", getpid());
        struct timeval tm;
        char buf[1024];
        while (1)
        {
         tm.tv_sec = 0 ;
         tm.tv_usec = 1000;
            temp_set = read_set ;
// printf("S in\n");
            int iret  = select(1024,&temp_set, NULL,NULL,NULL);//&tm);

    if(errno != 0)
{
    perror("select");
    printf("errno:%d\n", errno);
    printf("iret:%d\n", iret);
    printf("%d\n",errno == EINTR );
}
            //printf("select cnt : %d \n",iret);
            // if(iret==0) continue;
            // for (int i = 0; i < 10; ++i)
            // {
            //     printf("%d\n", client[i].s_read);
            // }

            for (int i = 1; i < 500; ++i)
            {            
                 if(FD_ISSET(client[i].s_read,&temp_set))
                {   
                    memset(buf, 0, sizeof(buf));
// printf("in, i=%d\n", i);
                    read(client[i].s_read,buf,1024);
// printf("out\n");
                    for (int j = 0; j < 500; ++j)
                    {   
                        if(client[j].s_write != 0)
                        {
                            write(client[j].s_write,buf,strlen(buf));
                        }
                    }

                }
//printf("big out\n");
            }
// printf("super out\n"); 
        }
    }
    
    
    return 0;
}



void login_server(void)
{
    char buff[1024];
    unsigned int temp_pid;

    printf("Initialing server...\n");

    fd_server = open("server.fifo", O_RDONLY);
    printf("Succseefully initialed server!\n");
    sleep(1);
    printf("There is 0 client now.\n");
    while (memset(buff, 0, 1024), read(fd_server, buff, 1024) > 0)
    {
        // printf("pid received! .%s.\n", buff);
        int i = 0;
        if(strlen(buff) < 6){
            temp_pid = atoi(buff);
            while (i<500 && cli_pid[i] != 0)
                ++i;
            if(i < 500) cli_pid[i] = temp_pid;
            else printf("full!\n");
        }
        else
        {
            temp_pid = atoi(buff+5);
            while(i<500 && cli_pid[i] != 0)
                cli_pid[i] = 0;
        }
       
        char temp1[10];
        sprintf(temp1, "%d ", i);
        char temp2[10];
        sprintf(temp2, " %u\n", cli_pid[i]);    //回车很重要！！
        
        int temp11 = write(inner_pipe[1], temp1, strlen(temp1));
        int temp22 = write(inner_pipe[1], temp2, strlen(temp2));
        kill(getppid(), 19);
//printf("sent:%s len:%d\n%s len:%d\n", temp1,temp11, temp2,temp22);        
    }
}



void refresh(int num)
{
    fscanf(in_p, "%d", &indx);
    fscanf(in_p, "%u", &pid);

    char fifo_read_name[32] = "";
    char fifo_write_name[32] = "";
    int fd_read, fd_write;
    
    if (pid != 0)
    {
        sprintf(fifo_read_name, "%u_write.fifo", pid);
        sprintf(fifo_write_name, "%u_read.fifo", pid);
        fd_read = open(fifo_read_name, O_RDONLY);        
        fd_write = open(fifo_write_name, O_WRONLY);
        client[indx].s_read = fd_read;
        client[indx].s_write = fd_write;
        FD_SET(fd_read,&read_set); 
// //temp_set = read_set ;
        printf("%u login!\n" , pid);
    }
}














