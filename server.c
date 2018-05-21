
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MAXPORT 6
#define MAXBUFF 1024
#define fileName "log.txt"

struct Agent
{
  char agentName[30];
  time_t joinTime;
};

// ** A TCP server **

int main(int argc, char** argv) {

  char cPort[MAXPORT];
  int nPort;
  if(argc < 2)
  {
    printf("ERROR: No port number specified\n");
    return (1);
  }
  memset(cPort, 0, MAXPORT);
  sprintf(cPort,"%s",argv[1]);
  nPort = atoi(cPort);

  // create a server TCP socket
  int svsck;
  svsck=socket(AF_INET,SOCK_STREAM,0);
  if (svsck<0) { printf("socket open fail.\n"); return(1); }

  // set the server's address
  struct sockaddr_in svaddr;
  svaddr.sin_family=AF_INET;
  svaddr.sin_addr.s_addr=INADDR_ANY;
  svaddr.sin_port=htons(nPort);

  // make address reusable
  int optval=1;
  if (setsockopt(svsck,SOL_SOCKET,SO_REUSEADDR,(const void *)(&optval), sizeof(int))<0) { printf("address is in use.\n"); return(1); }

  // bind address to the server socket
  if ((bind(svsck, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in)))<0) { printf("cannot bind address.\n"); return(1); }
	
  // make the socket listen, no more than 10 pending clients
  if (listen(svsck,10)<0) { printf("socket cannot listen.\n"); return(1);}

  // start accept clients
  struct sockaddr_in claddr; // client's address
  int claddrlen=sizeof(struct sockaddr_in);
  int clsck;

  struct Agent agentList [10];
  int numAgents = 0;
  while(1) 
  {
     //open file
     FILE *fp = fopen(fileName, "ab");

     if (fp == NULL)
     {
       printf("Error opening file!\n");
       exit(1);
     }

    // receive a connection request
    clsck=accept(svsck,(struct sockaddr *)&claddr,(socklen_t*)&claddrlen);
    if (clsck<0) { printf("connection fail.\n"); }
      
    /* Print ------------------------------------------------------------------------*/
    char clntName[claddrlen];
    if(inet_ntop(AF_INET, &claddr.sin_addr.s_addr, clntName, sizeof(clntName))!=NULL)
    {
      //check if agent already exists
      int checkFlag = 0;
      int j;
      for(j = 0; j < numAgents; j++)
      {
        if(strcmp(agentList[j].agentName, clntName) == 0)
        {
          checkFlag = 1;
        }
      }

      //read in request from agent
      char readBuff[MAXBUFF];
      int bytes_read = 0;
      int total_bytes_read = 0;
      memset(readBuff,0,MAXBUFF);
      bytes_read = read(clsck, readBuff, MAXBUFF);
      
      //time when request was received
      struct timeval tp;
      gettimeofday(&tp, 0);
      time_t reqTime = tp.tv_sec;
      struct tm *t = localtime(&reqTime);
      int reqHour = t->tm_hour;
      int reqMin = t->tm_min;
      int reqSec = t->tm_sec;
      int reqMil = tp.tv_usec/1000; 

      if(strcmp(readBuff, "#JOIN") == 0)
      {
        //print info to logFile
        fprintf(fp,"%02d:%02d:%02d:%03d ", reqHour, reqMin, reqSec, reqMil);
        fprintf(fp, "Received a '#JOIN' action from '%s'\n", clntName);

        if(checkFlag)
        {
          write(clsck,"$ALREADY MEMBER",15);
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "Responded to '%s' with '$ALREADY MEMBER'\n", clntName);
         
        }
        else
        {
          //puts agent IP in list
          strncpy(agentList[numAgents].agentName, clntName, sizeof(clntName));
          
          //this is needed to calculate the time the agent has been active
          struct timeval tp;
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          struct tm *t = localtime(&agentList[numAgents].joinTime);
          
          numAgents++;
          //writes to client
          write(clsck,"$OK",3);
          
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "Responded to '%s' with '$OK'\n", clntName);
        }
        fclose(fp);
      }
      else if(strcmp(readBuff, "#LEAVE") == 0)
      {
        //print info to logFile 
        fprintf(fp,"%02d:%02d:%02d:%03d ", reqHour, reqMin, reqSec, reqMil);  
        fprintf(fp, "Received a '#LEAVE' action from '%s'\n", clntName); 
        
        if(checkFlag)
        {
          // remove agent
          int k;
	  for(k = 0; k < numAgents; k++)
	  {
            if(strcmp(agentList[k].agentName, clntName) == 0)
	    {
	      //remove and shift
	      agentList[k] = agentList[numAgents-1];
	      numAgents--;
	    }
          }
	  write(clsck,"$OK",3);
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);               
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "Responded to '%s' with '$OK'\n", clntName);
        }
        else
	{
	  write(clsck,"$NOT MEMBER",12);
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);               
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "Responded to '%s' with '$NOT MEMBER'\n", clntName);
	}
        fclose(fp);
      }
      else if(strcmp(readBuff, "#LIST") == 0) 
      {
        //print info to logFile 
        fprintf(fp,"%02d:%02d:%02d:%03d ", reqHour, reqMin, reqSec, reqMil);  
        fprintf(fp, "Received a '#LIST' action from '%s'\n", clntName); 
        
        if(checkFlag)
        {
          int k;
          for(k = 0; k < numAgents; k++)
          {
            //get the difference in time between first join and now
            double diff = difftime(reqTime, agentList[k].joinTime);     

            //convert the time into a string
            char buffer[20];
            int diffSize = 0;
	    diffSize = sprintf(buffer, "%.0fs", diff);
            //write info to client
	    write(clsck, "\n", 1);
            write(clsck, "<", 1);
            write(clsck, agentList[k].agentName, 14);
            write(clsck, ", ", 2);
            write(clsck, buffer, diffSize);
            write(clsck, ">", 1);
          }
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime); 
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "List of agents is supplied to '%s'\n", clntName); 
        }
        else
        {
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime); 
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "No response supplied to '%s' (not active agent)\n", clntName); 
        }
        fclose(fp);
      }
      else if(strcmp(readBuff, "#LOG") == 0) 
      {
        //print info to logFile 
        fprintf(fp,"%02d:%02d:%02d:%03d ", reqHour, reqMin, reqSec, reqMil);  
        fprintf(fp, "Received a '#LOG' action from '%s'\n", clntName);
        fclose(fp);

        if(checkFlag)
        {
          FILE *fs = fopen(fileName, "r");
          char sendBuf[MAXBUFF];
          bzero(sendBuf, MAXBUFF);
          int fs_block_size;
          while((fs_block_size = fread(sendBuf, sizeof(char), MAXBUFF, fs))>0)
          {
                write(clsck, sendBuf, fs_block_size);

                bzero(sendBuf, MAXBUFF);
          }
          fclose(fs);
          
          FILE *fp = fopen(fileName, "ab");
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "log.txt is supplied to '%s'\n", clntName);
          fclose(fp);
          
        }
        else
        {
          //calculate time and print info to logFile
          gettimeofday(&tp, 0);
          agentList[numAgents].joinTime = tp.tv_sec;
          t = localtime(&agentList[numAgents].joinTime);                             
          fprintf(fp,"%02d:%02d:%02d:%03d ", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
          fprintf(fp, "No response supplied to '%s' (not active agent)\n", clntName);
        }
      }
      else
      {
        write(clsck, "INVALID REQUEST", 15); 
      }
    } 
    else
    {
      printf("Unable to get address\n");
    }
    close(clsck);
  }
  close(svsck);
  return 0;
}
