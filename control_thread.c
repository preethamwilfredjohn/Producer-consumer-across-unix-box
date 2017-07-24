#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define	MAXNITEMS 	1000
#define	MAXNTHREADS	100
#define buffer 20

/*Threads*/
void *produce(void *arg);
void *consume(void *arg);
void *status(void *);

/* global variables */

int	nitems=1,i; 
int readindex=0,writeindex=0,buffercount=0,producerSleepTime, consumerSleepTime, sleeptime,total=0;
int serversocket, comm_fd; 
int	buff[buffer]={0};
int	Nsignals;
int consumption=buffer;
int counter;
char incProducerRunningTime[5],incConsumerRunningTime[5],decProducerRunningTime[5],decConsumerRunningTime[5];
int i, prod, con;
int newprod=0,newcon=0;

/*Threads and mutex declaration*/
pthread_t	tid_produce[MAXNTHREADS], tid_consume[MAXNTHREADS],tid_status;
pthread_mutex_t mutex;
pthread_attr_t attr;
/* the semaphores */
sem_t full;
sem_t empty;


struct tstat
{
int sDataReceived;
int sProd;
int sCon;
int sConsumption;
int sCounter;
};

void initialize()
{
  pthread_mutex_init(&mutex, NULL);
   sem_init(&full, 0, 0);
   sem_init(&empty, 0, buffer);
   pthread_attr_init(&attr);
}

int main(int argc, char **argv)
{
	
	int n;
    char sendline[100],recvline[100];    
    char overridepro = 'p';
    char overridecon = 'c';
    char incprotime = 'i';
    char inccontime = 'j';
    char decprotime = 'd';
    char deccontime = 'e';
    char prods[5],cons[5];
    int port;
    
   	printf("Enter the sleep time in seconds for producer: \n");
	scanf("%d",&producerSleepTime);
	printf("Enter the sleep time in seconds for consumer: \n");
	scanf("%d",&consumerSleepTime);
	printf("Enter the number of producers : \n");
	scanf("%d",&prod);
	printf("Enter the number of consumers: \n");
	scanf("%d",&con);
	initialize();
	printf("Enter the port number :");
    scanf("%d",&port);
	/* create all producers and one consumer */
	for (i = 0; i < prod; i++) 
	{
		pthread_create(&tid_produce[i], &attr,produce, NULL);
	}
	for (i = 0; i < con; i++)
	{
		pthread_create(&tid_consume[i], &attr, consume, NULL);
	}
	struct tstat ts;
	ts.sProd=prod;
	ts.sCon=con;
	ts.sConsumption=consumption;
	ts.sCounter=counter;
	
	/*thread to display status*/
	pthread_create(&tid_status,&attr,status,(void *) &ts);
	
	/*connection with client*/
	bzero( sendline, 100);	
	printf("Expecting control process connection\n");
	struct sockaddr_in servaddr;
    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    bzero( &servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);
    bind(serversocket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(serversocket, 10);
    comm_fd = accept(serversocket, (struct sockaddr*) NULL, NULL);
	printf("%s",sendline);
	sprintf(sendline,"%d,%d,%d,%d",prod,con,producerSleepTime,consumerSleepTime);
	write(comm_fd,sendline,strlen(sendline)+1);	
	for(;;)
	{
	bzero( recvline, 100);
	read(comm_fd,recvline,100);
	/*interpretting data from server and comparing the string to find which data is being sent from the server*/
	for(int i=0;i<strlen(recvline);i++)
	{
		if(recvline[i] == overridepro)
		{
			strcpy(prods,strtok(recvline,"p"));
			printf("\n---override value received from client for increasing number of producer threads: %s---\n",prods);
			newprod=prod+atoi(prods);
			printf("---Increasing the producer thread as per client requirement---\n\n");
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for(i = 0; i < newprod; i++)
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			} 
			for (i = 0; i < con; i++) 
			{
				pthread_create(&tid_consume[i], &attr, consume, NULL);
			}
			ts.sProd=newprod;
			ts.sCon=con;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
			break;
		}
		else if(recvline[i] == overridecon)
		{
			strcpy(cons,strtok(recvline,"c"));
			printf("\n\n---override value received from client for increasing number of consumer threads: %s---\n",cons);
			newcon = newcon+con+atoi(cons);
			printf("---Increasing the consumer thread as per client requirementn---\n\n");			
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for(i = 0; i < prod; i++)
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			} 
			for (i = 0; i < newcon; i++) 
			{
				pthread_create(&tid_consume[i], &attr, consume, NULL);
			}
			ts.sProd=prod;
			ts.sCon=newcon;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
			
			break;
		}
		else if(recvline[i] == incprotime)
		{
			strcpy(incProducerRunningTime,strtok(recvline,"i"));
			printf("\n\n---override value received from client for increasing the time of producer threads: %s---\n", incProducerRunningTime);
			producerSleepTime=producerSleepTime+atoi(incProducerRunningTime);
			printf("---increased producer time : %d---\n\n",producerSleepTime);
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for (i = 0; i < prod; i++) 
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			}
			for (i = 0; i < con; i++) {
					pthread_create(&tid_consume[i], &attr, consume, NULL);
			} 
			ts.sProd=prod;
			ts.sCon=con;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
			break;
		}
		else if(recvline[i] == decprotime)
		{
			strcpy(decProducerRunningTime,strtok(recvline,"d"));
			printf("\n\n---override value received from client for decreasing the time of producer threads: %s\n---", decProducerRunningTime);
			producerSleepTime=producerSleepTime-atoi(decProducerRunningTime);
			printf("---decreased producer time : %d---\n\n",producerSleepTime);
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for (i = 0; i < prod; i++) 
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			}
			for (i = 0; i < con; i++) {
					pthread_create(&tid_consume[i], &attr, consume, NULL);
			} 
			ts.sProd=prod;
			ts.sCon=con;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
			break;
		}
		else if(recvline[i] == inccontime)
		{
			strcpy(incConsumerRunningTime,strtok(recvline,"j"));
			printf("\n\n---override value received from client for increasing the time of consumer threads: %s---\n",incConsumerRunningTime);
			consumerSleepTime=consumerSleepTime+atoi(incConsumerRunningTime);
			
			printf("---increased consumer time %d---\n\n",consumerSleepTime);
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for (i = 0; i < prod; i++) 
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			}
			for (i = 0; i < con; i++) {
					pthread_create(&tid_consume[i], &attr, consume, NULL);
			} 
			ts.sProd=prod;
			ts.sCon=con;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
			break;
		}	
		else if(recvline[i] == deccontime)
		{
			strcpy(incConsumerRunningTime,strtok(recvline,"e"));
			printf("\n\n---override value received from client for decreasing the time of consumer threads: %s---\n",decConsumerRunningTime);
			consumerSleepTime=consumerSleepTime-atoi(incConsumerRunningTime);
			printf("---decreased consumer time %d---\n\n",consumerSleepTime);
			for (i = 0; i < prod; i++)
			{
				pthread_cancel(tid_produce[i]);
			}
			for (i = 0; i < con; i++)
			{
				pthread_cancel(tid_consume[i]);
			}
			pthread_cancel(tid_status);
			sleep(1);
			for (i = 0; i < prod; i++) 
			{
				pthread_create(&tid_produce[i], &attr,produce, NULL);
			}
			for (i = 0; i < con; i++) {
					pthread_create(&tid_consume[i], &attr, consume, NULL);
			} 
			ts.sProd=prod;
			ts.sCon=con;
			ts.sConsumption=consumption;
			ts.sCounter=counter;
			pthread_create(&tid_status,&attr,status,(void *) &ts);
		}				
	}
}
	/*waiting for all the threads to complete their work*/
	for (i = 0; i < prod; i++) 
	{
		pthread_join(tid_produce[i], NULL);
	}
	for (i = 0; i < con; i++) 
	{
		pthread_join(tid_consume[i], NULL);
	}
	pthread_join(tid_status,NULL);
	printf("exit the program\n");
	close(comm_fd);
	exit(0);
}

void *produce(void *arg)
{
		
	while(1) 
	{
		sleep(producerSleepTime);
		sem_wait(&empty);      
		pthread_mutex_lock(&mutex);
		if(buffercount==buffer)
		{
				printf("Buffer full\n");
		}
		else
		{
			buff[writeindex]=nitems;
			if(nitems<=10000)
			{
				printf("Producer %lu produced %d",pthread_self(),nitems);
				writeindex=(writeindex+1)%buffer;
				nitems++;
				buffercount++;
				printf("\tBuffer Representation : ");
				for(i=0;i<buffer;i++)
				{
					printf("%d ",buff[i]);
				}
				printf("\n");
			}
			else
			{
				printf("Producer finished producing item\n");
				pthread_mutex_unlock(&mutex);
				sem_post(&full);
				exit(0);
			}
		}
		pthread_mutex_unlock(&mutex);
        sem_post(&full);
	}
}

void *consume(void *arg)
{
	int consu;
	while(1)
	{
		sleep(consumerSleepTime);
		sem_wait(&full);
        pthread_mutex_lock(&mutex);
        if(buffercount==0)
        {
			printf("Buffer empty\n");
		}
		else
		{
			consu=buff[readindex];
			buff[readindex]=0;
			printf("Consumer %lu consumed %d\n",pthread_self(),consu);
			total=total+consu;
			readindex=(readindex+1)%buffer;
			--buffercount;	
					
		}
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}
}

void *status(void *arg)
{
	for(int i=0;i<MAXNITEMS;i++)
    {
		float relativeSpeed=(float)producerSleepTime/(float)consumerSleepTime;
		struct tstat *ts=arg;
		sleep(5);
		printf("\n\n---Status of the application ---\n");
		printf("---Producer: %d Consumer %d, Relative speed P/C: %f, Buffer:%d ful slots Read Index : %d Write Index : %d  sum of consumer: %d ",ts->sProd, ts->sCon,relativeSpeed,19-buffercount, readindex, writeindex, total);
		printf(" ---\n\n");
	}
}
