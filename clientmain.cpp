#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <calcLib.h>
#include <string>

#define DEBUG

int main(int argc, char *argv[])
{
	if (argc != 2) 
	{
	  fprintf(stderr,"usage: %s hostname (%d)\n",argv[0], argc);
	  exit(1);
	}

  char delim[] = ":";
  char *Desthost = strtok(argv[1],delim);
  char *Destport = strtok(NULL,delim);
  int port = atoi(Destport);

  if (Desthost == NULL)
  {
    perror("Host  missing");
    exit(1);
  }

  if (Destport == NULL)
  {
    perror("Port missing");
    exit(1);
  }

#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int numbytes;
	char buf[264];
  char msg[] = "OK\n";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

  int rv;
	if ((rv = (getaddrinfo(Desthost, Destport, &hints, &servinfo))) == -1) 
	{
    fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

  for(p = servinfo; p != NULL; p = p->ai_next) 
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
    {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
    {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }
    

  if (p == NULL) 
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  if ((numbytes = recv(sockfd, buf, sizeof(buf), 0 /*alt. NULL*/)) == -1)
  {
    perror("Failed to recieve information");
    close(sockfd);
    exit(1);
  }
  else
  {
    #ifdef DEBUG
      printf("%s\n", buf);
      #endif

    if (strcmp(buf, "TEXT TCP 1.0\n\n") == 0 || strcmp(buf, "TEXT TCP 1.1\n\n") == 0)
    {
      #ifdef DEBUG
      printf("Connected to %s:%s\n", Desthost, Destport);
      #endif
    }
    else 
    {
      perror("Incorrect server format, ending connection..");
      exit(1);
    }
  }
  
  if (!send(sockfd, msg, strlen(msg), 0 /*alt. NULL*/))
  {
    perror("Invalid message or reciever");
    exit(1);
  }

  if (recv(sockfd, buf, sizeof(buf), 0 /*alt. NULL*/) == -1)
  {
    perror("Failed to recieve information");
    close(sockfd);
    exit(1);
  }
  else printf("ASSIGNMENT: %s \n", buf);

  //Operators for incoming data stream
  bool is_float = false;
  double f1,f2,fresult;
  int i1,i2,iresult;
  {
    // Initialize the library, this is needed for this library. 
    initCalcLib();

    char *lineBuffer = buf;
    char command[10];
    sscanf(lineBuffer,"%s",command);
  
    if(command[0]=='f')
    {
      is_float = true;
      sscanf(lineBuffer,"%s %lg %lg",command,&f1,&f2);
      if(strcmp(command,"fadd")==0)         fresult=f1+f2;
      else if (strcmp(command, "fsub")==0)  fresult=f1-f2;
      else if (strcmp(command, "fmul")==0)  fresult=f1*f2;
      else if (strcmp(command, "fdiv")==0)  fresult=f1/f2;
    } 
    else 
    {
      sscanf(lineBuffer,"%s %d %d",command,&i1,&i2);

      if(strcmp(command,"add")==0) iresult=i1+i2;
      else if (strcmp(command, "sub")==0)
      {
        iresult=i1-i2;
      } 
      else if (strcmp(command, "mul")==0) iresult=i1*i2;
      else if (strcmp(command, "div")==0) iresult=i1/i2;
    }
  }
  char sendvalue[256];
  if(is_float) sprintf(sendvalue, "%8.8g\n", fresult);
  else sprintf(sendvalue, "%d\n", iresult);
  
  if (send(sockfd, sendvalue, strlen(sendvalue), 0) == -1)
  {
    perror("Invalid message or reciever");
    exit(1);
  }
  else
  {
    printf("My result: %s\n", sendvalue);
  }

  char vbuf[256];
  if (recv(sockfd, vbuf, sizeof(vbuf), 0) == -1)
  {
    perror("Failed to recieve information");
    close(sockfd);
    exit(1);
  }
  else printf("%s \n", vbuf);

  return 1;
}