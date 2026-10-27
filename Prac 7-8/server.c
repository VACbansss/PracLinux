#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <libgen.h>
#include <arpa/inet.h>

#define MAX_LINE 4096

void error(const char *meassage)
{
    perror(meassage);
    exit(1);
}


//a function to send the binary contents of the file after everything else is Success
void sendfiles(FILE *fp, int sockfd)
{
    int n;
    char line[MAX_LINE] = {0};
    //while its still reading from the file
    while ((n = fread(line, sizeof(char), MAX_LINE, fp)) > 0)
    {
        //if there is problem reading terminiate
        if (ferror(fp) && (n != MAX_LINE))
        {
            perror("Read Error");
        }
        //if file is ready to send, try to send it
        if (send(sockfd, line, n, 0) == -1)
        {
            perror("Can't send");
        }
        memset(line, 0, MAX_LINE);
    }
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    error("port num not provided\n");
  }
  //regular decleration for the server program
    int sockfd, portno, n, newsockfd;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
      error("error opening socket");
    }
    //cleaning the struct serv_addr and filling it with the corect info
    //for IPv4 and TCP with the provided port number
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    portno = atoi (argv[1]);
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(portno);

    //try to connect the file descriptor with the ip add, and port.(bind them)
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Binding fail\n");
    }

//if binding was success, that means the server program is connected to then
//outside world, which means it now can keep running forever waiting.
    while(1){

        // trying to coonnect for any specfic client through newsockfd
        listen(sockfd, 5);
        clilen=sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        //if connecting to that specfic client failed, close them and keep listening
        if(newsockfd<0){
            close(newsockfd);
            continue;
        }

        //trying to recieve the desieried file name
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n<0){
          close(newsockfd);
          continue;
        }
        //try to open that file
        FILE *fp = fopen(buffer, "rb");
        //if couldnt open send [F]ail and start serving others
        if (fp == NULL)
        {
            char errors[2]="F";
            bzero(buffer, 256);
            strncpy (buffer, errors, 2);
            n = write(newsockfd, buffer, strlen(buffer));
            close(newsockfd);
            continue;
       }
       //if opened say [S]uccess

        buffer[0]='\0';
        char errorss[2]="S";
        errorss[1]='\0';
        n = write(newsockfd, errorss, strlen(errorss));

        if (n<0){
          close(newsockfd);
          continue;
        }

        //try to send the file. no matter if u can or cant, just close the client
        //and file, and start serving others
        sendfiles(fp, newsockfd);
        fclose(fp);
        close(newsockfd);
    }
  return 0;
}
