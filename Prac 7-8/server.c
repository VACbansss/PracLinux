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


//функция для отправки двоичного содержимого файла после того, как всё остальное будет успешно выполнено
void sendfiles(FILE *fp, int sockfd)
{
    int n;
    char line[MAX_LINE] = {0};
    //пока идёт чтение из файла
    while ((n = fread(line, sizeof(char), MAX_LINE, fp)) > 0)
    {
        //Если возникнут проблемы с чтением
        if (ferror(fp) && (n != MAX_LINE))
        {
            perror("Read Error");
        }
        //Если файл готов к отправке, попробуйте его отправить
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
  //обычное объявление для серверной программы
    int sockfd, portno, n, newsockfd;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
      error("error opening socket");
    }
    //очистка структуры serv_addr и заполнение её корректными данными
    //для IPv4 и TCP с указанным номером порта
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    portno = atoi (argv[1]);
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(portno);

    //Попробуйте связать дескриптор файла с IP-адресом и портом (привяжите их).
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Binding fail\n");
    }

//Если привязка прошла успешно, это означает, что серверная программа теперь подключена
//Внешний мир, а это значит, что теперь она может продолжать работать бесконечно, ожидая.
    while(1){

        // попытка установить соединение для конкретного клиента через newsockfd
        listen(sockfd, 5);
        clilen=sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        //Если подключение к этому конкретному клиенту не удалось, закройте его и продолжайте прослушивание.
        if(newsockfd<0){
            close(newsockfd);
            continue;
        }

        //попытка получить желаемое имя файла
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n<0){
          close(newsockfd);
          continue;
        }
        //попробуй открыть этот файл
        FILE *fp = fopen(buffer, "rb");
        //если открыть не удалось — отправьте [F]ail и переходите к обслуживанию других
        if (fp == NULL)
        {
            char errors[2]="F";
            bzero(buffer, 256);
            strncpy (buffer, errors, 2);
            n = write(newsockfd, buffer, strlen(buffer));
            close(newsockfd);
            continue;
       }
       //если открыто [S]uccess

        buffer[0]='\0';
        char errorss[2]="S";
        errorss[1]='\0';
        n = write(newsockfd, errorss, strlen(errorss));

        if (n<0){
          close(newsockfd);
          continue;
        }

        //Попробуй отправить файл. Вне зависимости от того, получится или нет
        sendfiles(fp, newsockfd);
        fclose(fp);
        close(newsockfd);
    }
  return 0;
}
