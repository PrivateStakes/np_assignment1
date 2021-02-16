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

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass 
#define DEBUG

int main(int argc, char *argv[]){

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */

	if (argc != 2) 
	{
	  fprintf(stderr,"usage: %s hostname (%d)\n",argv[0],argc);
	  exit(1);
	}

  char delim[] = ":";
  char *Desthost = strtok(argv[1],delim);
  char *Destport = strtok(NULL,delim);

  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  int port = atoi(Destport);

#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	char buf[264];
  char msg[] = "OK\n";
  char server_error[] = "ERROR\n";



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], Destport, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
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
    printf("Response: %s \n", buf);

    if (strcmp(buf, "TEXT TCP 1.0\n\n") == 0 || strcmp(buf, "TEXT TCP 1.1\n\n") == 0) printf("matching protocols! proceeding.. \n");
    else 
    {
      perror("Incorrect server format, ending connection..");
      exit(1);
    }
  }
  
  if (!send(sockfd, msg, sizeof(msg), 0 /*alt. NULL*/))
  {
    perror("Invalid message or reciever");
    exit(1);
  }
  else printf("Message sent, awaiting response..\n");

  if (recv(sockfd, buf, sizeof(buf), 0 /*alt. NULL*/) == -1)
  {
    perror("Failed to recieve information");
    close(sockfd);
    exit(1);
  }
  else printf("Response, operation: %s \n", buf);

  //Operators for incoming data stream
  bool is_float = false;
  double f1,f2,fresult;
  int i1,i2,iresult;
  {
    // Initialize the library, this is needed for this library. 
    initCalcLib();

    
    //printf("ptr = %p, \t", ptr );
    //printf("string = %s, \n", ptr );

    // This section shows how to read a line from stdin, process and do a similar operation as above.
  
    char *lineBuffer = buf;
  
    //printf("got:> %s \n",lineBuffer);
    char command[10];

    int rv; //return value
    rv = sscanf(lineBuffer,"%s",command);

   //printf("Command: |%s|\n",command);
  
    if(command[0]=='f')
    {
      is_float = true;
      //printf("Float\t");
      rv=sscanf(lineBuffer,"%s %lg %lg",command,&f1,&f2);
      if(strcmp(command,"fadd")==0)         fresult=f1+f2;
      else if (strcmp(command, "fsub")==0)  fresult=f1-f2;
      else if (strcmp(command, "fmul")==0)  fresult=f1*f2;
      else if (strcmp(command, "fdiv")==0)  fresult=f1/f2;

      //printf("%s %8.8g %8.8g = %8.8g\n",command,f1,f2,fresult);
    } 
    else 
    {
      //printf("Int\t");
      rv=sscanf(lineBuffer,"%s %d %d",command,&i1,&i2);

      if(strcmp(command,"add")==0) iresult=i1+i2;
      else if (strcmp(command, "sub")==0)
      {
        iresult=i1-i2;
        //printf("[%s %d %d = %d ]\n",command,i1,i2,iresult);
      } 
      else if (strcmp(command, "mul")==0) iresult=i1*i2;
      else if (strcmp(command, "div")==0) iresult=i1/i2;
      else  printf("No match\n");

    //printf("%s %d %d = %d \n",command,i1,i2,iresult);
    }

  free(lineBuffer); // This is needed for the getline() as it will allocate memory (if the provided buffer is NUL).
  }

  if (is_float) printf("value is %f \n", fresult);
  else printf("value is %i \n", iresult);

  char sendervalue[256];
  if(is_float) sprintf(sendervalue, "%8.8g\n", fresult);
  else sprintf(sendervalue, "%d\n", iresult);
  
  if (send(sockfd, sendervalue, strlen(sendervalue), 0) == -1)
  {
    perror("Invalid message or reciever");
    exit(1);
  }
  else
  {
    if (is_float) printf("Message sent, float value %s Awaiting response..\n", sendervalue);
    else printf("Message sent, int value %s Awaiting response..\n", sendervalue);
  }

  char vbuf[256];
  if (recv(sockfd, vbuf, sizeof(vbuf), 0) == -1)
  {
    perror("Failed to recieve information");
    close(sockfd);
    exit(1);
  }
  else printf("Response: %s \n", vbuf);

  return 1;
}


//ToDo:
/*
- Create socket
- Connect
- Respond ("OK\n")
- recv
- process data: send through 'operators' scope
- Submit response using correct format (float or int) -  For floating-point use "%8.8g "for the formatting
- Await response ('OK' = success, 'ERROR' = incorrect response)

-Send msg
    -Read response
    -Stäng socket
    EOF
*/