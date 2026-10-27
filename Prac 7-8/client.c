#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 4096

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//some function to write the recieved chunks into the desired file
void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    //if recieving, keep going
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0)
    {
        if (n == -1)
        {
            error("Receive Error");
        }
        // if coludnt write for some reason exit with 1
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            error("Write Error");
        }
        memset(buff, 0, MAX_LINE);
    }
}

int main(int argc, char *argv[])
{
    //the regular declarations of a client
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    //check if they have the right number of args
    if (argc < 5) {
       error("You should have ip, portno, source file, dest file\n");
    }
    //fill in the ip and port number of server to connect to it
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        error("ERROR, this host dne\n");
    }
    //filling the right Ip address in serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    //connect based on the info above
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("Connecting error");

    //Intillaize the succuses or faulire pointer
    bzero(buffer,256);
    char sorf[2]; //success or fauilure
    sorf[1]='\0';
    //send the desired file name
    strncpy(buffer, argv[3], strlen(argv[3]));
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         error("Error writing");
    //check if this desired file exisits or not
    n = read(sockfd,sorf,1);
    if (n < 0)
         error("Error reading");

    printf("Server: %s\n",sorf);
    //if failed to find file exit with 1;
    int i = strncmp("F" , sorf , 2);
    if(i == 0)
           exit(1);
    //if file exisits, lets try to copy it.
    //create an empty file with the desired name
    FILE *fp = fopen(argv[4], "wb");
    if (fp == NULL)
    {
        error("Can't open file");
    }
    //start filling the file. and ofc when done just close everything and success
    writefile(sockfd, fp);
    fclose(fp);
    close(sockfd);
    return 0;
}
