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

//функция для записи полученных фрагментов в нужный файл
void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    //если получаешь — продолжай
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0)
    {
        if (n == -1)
        {
            error("Receive Error");
        }
        // Если по какой-либо причине запись невозможна, завершить работу с кодом 1.
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            error("Write Error");
        }
        memset(buff, 0, MAX_LINE);
    }
}

int main(int argc, char *argv[])
{
    //регулярные заявления клиента
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    //проверьте, верное ли у них количество аргументов
    if (argc < 5) {
       error("You should have ip, portno, source file, dest file\n");
    }
    //Введите IP-адрес и номер порта сервера, чтобы подключиться к нему.
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        error("ERROR, this host dne\n");
    }
    //указание правильного IP-адреса в serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    //установите связь на основе приведенной выше информации
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("Connecting error");

    //Инициализируйте указатель на результат (успех или неудачу)
    bzero(buffer,256);
    char sorf[2]; //успех или неудача
    sorf[1]='\0';
    //отправьте желаемое имя файла
    strncpy(buffer, argv[3], strlen(argv[3]));
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
         error("Error writing");
    //проверьте, существует ли нужный файл
    n = read(sockfd,sorf,1);
    if (n < 0)
         error("Error reading");

    printf("Server: %s\n",sorf);
    //если файл не найден, завершить работу с кодом 1;
    int i = strncmp("F" , sorf , 2);
    if(i == 0)
           exit(1);
    //Если файл существует, давайте попробуем его скопировать.
    //создайте пустой файл с нужным именем
    FILE *fp = fopen(argv[4], "wb");
    if (fp == NULL)
    {
        error("Can't open file");
    }
    //Начинайте заполнять файл. По завершении просто закройте всё
    writefile(sockfd, fp);
    fclose(fp);
    close(sockfd);
    return 0;
}
