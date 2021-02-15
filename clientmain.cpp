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

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass 
#define DEBUG

int main(int argc, char *argv[]){

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  int port=atoi(Destport);

#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	char buf[264];

	if (argc != 2) 
	{
	  fprintf(stderr,"usage: %s hostname (%d)\n",argv[0],argc);
	  exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], Destport, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	if (connect(sockfd,p->ai_addr, p->ai_addrlen) < 0 ) 
	{
	  perror("talker2: connect .\n");
	  exit(1);
	}
  
  //Operators for incoming data stream
  {
      /* Initialize the library, this is needed for this library. */
    initCalcLib();
    char *ptr;
    ptr=randomType(); // Get a random arithemtic operator. 

  double f1,f2,fresult;
  int i1,i2,iresult;
  /*
  printf("ptr = %p, \t", ptr );
  printf("string = %s, \n", ptr );
  */

  /* Act differently depending on what operator you got, judge type by first char in string. If 'f' then a float */
  
  if(ptr[0]=='f')
  {
    printf("Float\t");
    f1=randomFloat();
    f2=randomFloat();

    /* At this point, ptr holds operator, f1 and f2 the operands. Now we work to determine the reference result. */
   
    if(strcmp(ptr,"fadd")==0)        fresult=f1+f2;
    else if (strcmp(ptr, "fsub")==0) fresult=f1-f2;
    else if (strcmp(ptr, "fmul")==0) fresult=f1*f2;
    else if (strcmp(ptr, "fdiv")==0) fresult=f1/f2;

    printf("%s %8.8g %8.8g = %8.8g\n",ptr,f1,f2,fresult);
  } 
  else 
  {
    printf("Int\t");
    i1=randomInt();
    i2=randomInt();

    if(strcmp(ptr,"add")==0)        iresult=i1+i2;
    else if (strcmp(ptr, "sub")==0) iresult=i1-i2;
    else if (strcmp(ptr, "mul")==0) iresult=i1*i2;
    else if (strcmp(ptr, "div")==0) iresult=i1/i2;

    printf("%s %d %d = %d \n",ptr,i1,i2,iresult);
  }

  /* This section shows how to read a line from stdin, process and do a similar operation as above. */
  
  char *lineBuffer=NULL;
  size_t lenBuffer=0;
  ssize_t nread=0;
  printf("Print a command: ");
  nread=getline(&lineBuffer,&lenBuffer,stdin);
  
  printf("got:> %s \n",lineBuffer);

  int rv;
  char command[10];

  
  rv=sscanf(lineBuffer,"%s",command);

  printf("Command: |%s|\n",command);
  
  if(command[0]=='f')
  {
    printf("Float\t");
    rv=sscanf(lineBuffer,"%s %lg %lg",command,&f1,&f2);
    if(strcmp(command,"fadd")==0)         fresult=f1+f2;
    else if (strcmp(command, "fsub")==0)  fresult=f1-f2;
    else if (strcmp(command, "fmul")==0)  fresult=f1*f2;
    else if (strcmp(command, "fdiv")==0)  fresult=f1/f2;

    printf("%s %8.8g %8.8g = %8.8g\n",command,f1,f2,fresult);
  } 
  else 
  {
    printf("Int\t");
    rv=sscanf(lineBuffer,"%s %d %d",command,&i1,&i2);

    if(strcmp(command,"add")==0) iresult=i1+i2;
    else if (strcmp(command, "sub")==0)
    {
      iresult=i1-i2;
      printf("[%s %d %d = %d ]\n",command,i1,i2,iresult);
    } 
    else if (strcmp(command, "mul")==0) iresult=i1*i2;
    else if (strcmp(command, "div")==0) iresult=i1/i2;
    else  printf("No match\n");

    printf("%s %d %d = %d \n",command,i1,i2,iresult);
  }

  free(lineBuffer); // This is needed for the getline() as it will allocate memory (if the provided buffer is NUL).
  }

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