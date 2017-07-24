#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

/*Global Variable*/
int incpro,inccon,incprotime,decprotime,inccontime, deccontime;
char prod[5],cons[5], producerRunningTime[5],consumerRunningTime[5];

int increaseProducer()
{
	printf("Enter the number of producer to be increased : \n");
	scanf("%d",&incpro);
	return(incpro);
	
}

int increaseConsumer()
{
	printf("Enter the number of consumer to be increased : \n");
	scanf("%d",&inccon);
	return(inccon);	
}

int increaseProductionTime()
{
	printf("Enter the time to increase the production function: \n");
	scanf("%d",&incprotime);
	return(incprotime);
	
}

int decreaseProductionTime()
{
	printf("Enter the time to decrease the production function\n");
	scanf("%d",&decprotime);
	return(decprotime);
}

int increaseConsumptionTime()
{
	printf("Enter the time to increase the consumption function\n");
	scanf("%d",&inccontime);
	return(inccontime);
}

int decreaseConsumptionTime()
{
	printf("Enter the time to decrease the consumption function\n");
	scanf("%d",&deccontime);
	return(deccontime);
}

int main(int argc,char **argv)
{
	int choice;
	const char ch[2]=",";
    char sendline[100],recvline[100];
    int clientsocket;
    int port;
    printf("Enter the port number :");
    scanf("%d",&port);
    /*Establishing conection with server*/
    struct sockaddr_in servaddr;
    clientsocket=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
    connect(clientsocket,(struct sockaddr *)&servaddr,sizeof(servaddr));   
    bzero( recvline, 100);
    read(clientsocket,recvline,100);
    strcpy(prod,strtok(recvline,","));
    strcpy(cons,strtok(NULL,","));
    strcpy(producerRunningTime,strtok(NULL,","));
    strcpy(consumerRunningTime,strtok(NULL,","));
    printf("current number of producer threads running in server : %s\n",prod);
	printf("current number of consumer threads running in server : %s\n",cons);
	printf("current speed of  prducer threads running in server : %s\n", producerRunningTime);
	printf("current speed of consumer threads running in server : %s\n",consumerRunningTime);
	/*Sending data to server*/
	while(1)
	{
	bzero( sendline, 100);
    printf("Please enter select one from below and enter appropriate values\n");
    printf("1) Increase the number of producers and by how many?\n2) Increase the number of consumers and by how many?\n3) Increase the production time and by how much?\n4) Decrease the production time and by how much?\n5) Increase the consumption time and by how much?\n6) Decrease the consumption time and by how much?\n7) Quit\n");
    scanf("%d", &choice);
    switch(choice)
    {
		case 1 :
				//Increase the number of producers and by how many?
				increaseProducer();
				sprintf(sendline,"p%d",incpro);
				break;
		case 2 :
				//Increase the number of consumers and by how many?
				increaseConsumer();
				sprintf(sendline,"c%d",inccon);
				break;
		case 3 :
				//Increase the production time and by how much?
				increaseProductionTime();
				sprintf(sendline,"i%d",incprotime);
				break;
		case 4 :
				//Decrease the production time and by how much?
				decreaseProductionTime();
				sprintf(sendline,"d%d",decprotime);
				break;
		case 5 :
				//Increase the consumption time and by how much?
				increaseConsumptionTime();
				sprintf(sendline,"j%d",inccontime);
				break;
		case 6 :
				//Decrease the consumption time and by how much?
				decreaseConsumptionTime();
				sprintf(sendline,"e%d",deccontime);
				break;
		case 7 :
				printf("Exiting the application\n");
				exit(0);
	}
	printf("sending new data to server\n");
	write(clientsocket, sendline, strlen(sendline)+1);
	}
	printf("Exiting application\n");
    close(clientsocket);
}

