/* Print Debug Info. Uncomment the #define below to print debugging to stderr */

  #undef DEBUG

/******************************************************************************

	Esensors EM01 Plugin.
	Description: This plugin is written mainly for Nagios, but can be
				 easily used for other software too.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 $Id: check_em01.c,v 1.5 2005/08/11 23:40:30 Esensors, Inc Exp $
 
******************************************************************************/
const int DEFAULTPORT = 80;
const char *progname = "check_em01";
const char *revision = "$Revision: 1.5 $";
const char *copyright = "2005";
const char *email = "techhelp@eEsensors.com";

/* 
*****: TCP/IP client code.
This code was obtained from koders.com with a GNU GPL license.
It is clearly marked with ----- BEGIN KODERS.COM COPIED CODE -----  
and ends with ----- END KODERS.COM COPIED CODE ----- 
The reason for copying code - No need to reinvent the wheel.
*/

/* ----- BEGIN KODERS.COM COPIED CODE -----   */
typedef int SOCKET;
typedef enum {E_NET_ERRNO=-1, E_NET_OK=0} NetErrnoType;

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

static NetErrnoType net_errno = E_NET_OK;
static int saved_errno = 0;

/* Translations between ``net_errno'' values and human readable strings.
 */
static const char *net_syserrlist[] = {
  "All was chill"
};




#ifdef STRERROR_NOT_DEFINED
const char *strerror(int errno) { return sys_errlist[errno]; }
#endif

static void NetSetErrno(NetErrnoType e)
{
  if(e == E_NET_ERRNO)saved_errno = errno;
  net_errno = e;
}



/* NetErrStr()
 *--------------------------------------------------------------------
 * Returns a diagnostic message for the last failure.
 */
const char *NetErrStr()
{
  return net_errno==E_NET_ERRNO ? strerror(saved_errno) :
    net_syserrlist[net_errno];
}

/* NetErrNo()
 *--------------------------------------------------------------------
 * Returns a diagnostic number for the last failure.
 */
NetErrnoType NetErrNo()
{
  return net_errno;
}

/* NetMakeContact()
 *--------------------------------------------------------------------
 * Makes a tcp connection to a host:port pair.
 *--------------------------------------------------------------------
 * ``Hostname'' can either be in the form of a hostname or an IP address
 * represented as a string. If the hostname is not found as it is,
 * ``hostname'' is assumed to be an IP address, and it is treated as such.
 *
 * If the lookup succeeds, a TCP connection is established with the
 * specified ``port'' number on the remote host and a stream socket is
 * returned.
 *
 * On any sort of error, an error code can be obtained with @NetErrNo()
 * and a message with @NetErrStr().
 */
SOCKET
NetMakeContact(const char *hname, int port)
{
  int fd;
  struct sockaddr_in addr;
  struct hostent *hent;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
    {
      NetSetErrno(E_NET_ERRNO);
      return -1;
    }


  hent = gethostbyname(hname);
  if(hent == NULL)
    addr.sin_addr.s_addr = inet_addr(hname);
  else
    memcpy(&addr.sin_addr, hent->h_addr, hent->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
      NetSetErrno(E_NET_ERRNO);
      return -1;
    }

  NetSetErrno(E_NET_OK);
  return fd;
}

/* ----- END KODERS.COM COPIED CODE -----   */


/*********************************************************************************/

float Exp10(int n)
{
  int i;
  float result = 1;
  for(i =n; i; i--)
    result *= 10;
  return result;
}

float myatof(const char *s)
{
  float result;
  int val = 0, dec = 0, n = 0;;
  int neg = 0;

  /* skip white space */
  while (*s == ' ' || *s == '\t') {
    s++;
  }

  if (*s == '-') {
    neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  while (*s >= '0' && *s <= '9') {
    val *= 10;
    val += *s++ - '0';
  }
  result = val;

  if(*s == '.') {
    *s++;
    while (*s >= '0' && *s <= '9') {
      dec *= 10;
      dec += *s++ - '0';
      n++;
    }

    if(n)
      result += dec/Exp10(n);
  }

  if (neg) {
    result = -result;
  }

  return result;
}

/*
  This function is an abstraction layer between app and sockets.
  Currently, it only passes down all the arguments it receives from
  app. However, in the future, it will be easier to change 
  sockets library and not affect the main app code at all.
*/
SOCKET connectWebsensor (char* hostname, int port){
  SOCKET s;
  fd_set readfds;

  int i;
  s = NetMakeContact(hostname,port);
  if(s==-1) {
    return -1;
  }
  else{
    /* Make the socket non-blocking */
    ioctl(s, FIONBIO, 1);
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    FD_SET(s, &readfds);
    return s;

  }
}


main(int argc, char **argv)
{
  SOCKET s;
  int l, retry;
  float data;
  char iobuf[1024], timestr[32], datachar[7];
  char* pos;
  time_t cur_time;

  progname = argv[0];
  if(argc < 2 || strcmp(argv[1],"--help") == 0) {
    print_help();
    exit(3);
  }

  time(&cur_time);


  sprintf(timestr, "%s", ctime(&cur_time));

  for(retry = 0; retry < 3; retry++){

    /* make connection to websensor */
    s = connectWebsensor(argv[1], DEFAULTPORT); 
    if(NetErrNo() != 0){
#ifdef DEBUG
      fprintf(stderr, "Could not connect to Websensor because %s, will retry %d more times.\n", NetErrStr(), 3-retry);
#endif
	  shutdown(s, SHUT_RDWR);
      continue;
    }


    /* send HTTP GET request to obtain data */
    if(argc>2){
		  
      switch (toupper(argv[2][0])){
      case 'R':
	write(s, "GET /index.html?eR HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 76);
	break;
		    
      case 'V':
	write(s, "GET /index.html?ev HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 76);
	break;

      default:
	write(s, "GET /index.html?em123456 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 82);
	break;
      }
    }
		
    else{ // Not enough arguments from command line. Use default websensor command.
      write(s, "GET /index.html?em123456 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 82);
    }
		

    l = read(s, iobuf, sizeof(iobuf));

    /* No data returned from websensor. Will retry again. */
    if(l<=0){
#ifdef DEBUG
      fprintf(stderr, "No Data Read, will retry %d more times.\n", 3-retry);
#endif
	  shutdown(s, SHUT_RDWR);
      continue;
    }

    /*
      pos = strstr(iobuf, "01T");
      printf("POS: %d\n", pos-iobuf);
    */

    pos = &iobuf[165];
	
    /* Unsupported command. Exit */
    if(pos[0] == '#'){
	    printf("Invalid Command. Option %c selected may not be available for this websensor.\n", argv[2][0]);
	    return 3;
    }	
    
    /* The http data is not properly formatted. */
    if(pos[2] != 'T' && pos[2] != 'R' && pos[1] != 'v'){

#ifdef DEBUG
      fprintf(stderr, "Data input incorrect, will retry %d more times.\n", 3-retry);
#endif
	  shutdown(s, SHUT_RDWR);
      continue;
    }
    else{
      break; /* All data looks good. Break out of loop. */
    }
  }

  /* Retried 3 times earlier and still no good data. Time to exit */
  if(retry == 3){
    printf("NO DATA\n");
    return 3;
  }

  if(argc > 2){
    switch(toupper(argv[2][0])){
    case 'T': 
      {
	strncpy(datachar, pos+5, 5);
	datachar[5] = '\0';
	data = myatof(datachar);
	data += 0.01;
	if(argc != 7){
	  printf("(No limits specified) Temperature: %s %c\n", datachar, pos[3]);
	  return(0);
	}
	if(data < myatof(argv[5]) || data > myatof(argv[6])){
	  printf("CRITICAL ( %s< or >%s ) Temperature: %s %c\n", argv[5], argv[6], datachar, pos[3]);
	  return(2);
	}
	else if(data < myatof(argv[3]) || data > myatof(argv[4])){
	  printf("WARNING ( %s< or >%s ) Temperature: %s %c\n", argv[3], argv[4], datachar, pos[3]);
	  return(1);
	}
	else{
	  printf("OK Temperature: %s %c\n", datachar, pos[3]);
	  return(0);
	}
      }
      break;

    case 'H': 
      {
	strncpy(datachar, pos+13, 4);
	datachar[4] = '\0';
	data = myatof(datachar);
	data += 0.01;
	if(argc != 7){
	  printf("(No limits specified) %s%\n", datachar);
	  return(0);
	}
	if(data < myatof(argv[5]) || data > myatof(argv[6])){
	  printf("CRITICAL ( %s< or >%s ) Humidity: %s%\n", argv[5], argv[6], datachar);
	  return(2);
	}
	else if(data < myatof(argv[3]) || data > myatof(argv[4])){
	  printf("WARNING ( %s< or >%s ) Humidity: %s%\n", argv[3], argv[4], datachar);
	  return(1);
	}
	else{
	  printf("OK Humidity: %s%\n", datachar);
	  return(0);
	}
      }
      break;

    case 'I': 
      {
	strncpy(datachar, pos+21, 5);
	datachar[5] = '\0';
	data = myatof(datachar);
	data += 0.01;
	if(argc != 7){
	  printf("(No limits specified) Illumination: %s\n", datachar);
	  return(0);
	}
	if(data < myatof(argv[5]) || data > myatof(argv[6])){
	  printf("CRITICAL ( %s< or >%s ) Illumination: %s%\n", argv[5], argv[6], datachar);
	  return(2);
	}
	else if(data < myatof(argv[3]) || data > myatof(argv[4])){
	  printf("WARNING ( %s< or >%s ) Illumination: %s%\n", argv[3], argv[4], datachar);
	  return(1);
	}
	else{
	  printf("OK Illumination: %s\n", datachar);
	  return(0);
	}
      }
      break;

    case 'C':
      {
	if(iobuf[160] == 'W'){
	  printf("OK Contacts Close.\n");
	  return(0);
	}
	else if(iobuf[160] == 'N'){
	  printf("CRITICAL Contacts Open!\n");
	  return(2);
	}
	else{
	  printf("WARNING Unknown status. Try Reset Device.\n");
	  return(1);
	}
      }
      break;
						
    case 'R':
      {
	strncpy(datachar, pos+4, 6);
	datachar[6] = '\0';
	data = myatof(datachar);
	data += 0.01;
	if(argc != 7){
	  printf("(No limits specified) RTD Temperature: %s %c\n", datachar, pos[3]);
	  return(0);
	}
	if(data < myatof(argv[5]) || data > myatof(argv[6])){
	  printf("CRITICAL ( %s< or >%s ) RTD Temperature: %s %c\n", argv[5], argv[6], datachar, pos[3]);
	  return(2);
	}
	else if(data < myatof(argv[3]) || data > myatof(argv[4])){
	  printf("WARNING ( %s< or >%s ) RTD Temperature: %s %c\n", argv[3], argv[4], datachar, pos[3]);
	  return(1);
	}
	else{
	  printf("OK RTD Temperature: %s %c\n", datachar, pos[3]);
	  return(0);
	}
      }
      break;
      
    case 'V':
      {
	strncpy(datachar, pos+22, 5);
	datachar[5] = '\0';
	data = myatof(datachar);
	data += 0.01;
	if(argc != 7){
	  printf("(No limits specified) Voltage: %s V\n", datachar);
	  return(0);
	}
	if(data < myatof(argv[5]) || data > myatof(argv[6])){
	  printf("CRITICAL ( %s< or >%s ) Voltage: %s V\n", argv[5], argv[6], datachar);
	  return(2);
	}
	else if(data < myatof(argv[3]) || data > myatof(argv[4])){
	  printf("WARNING ( %s< or >%s ) Voltage: %s V\n", argv[3], argv[4], datachar);
	  return(1);
	}
	else{
	  printf("OK Voltage: %s V\n", datachar);
	  return(0);
	}
      }

    default : 
      printf("Please choose only 'T', 'H', 'I', 'R', 'V' or 'C'.\n Please refer to README for further instructions.\n");
      break;
    }
  }
  else{
    iobuf[195] = 0;
    printf("%s\t%s", pos+2, timestr);
  }

  return 0;
}

int print_help (void)
{
  fprintf (stdout, "Copyright (c) 2005 Esensors, Inc <TechHelp@eEsensors.com>\n");
  fprintf (stdout,("This plugin is written mainly for Nagios, but can work standalone too. \nIt returns the HVAC data from the EM01 Websensor\n\n"));
  print_usage ();
  return 0;
}



int print_usage (void)
{
  fprintf(stdout, "usage: \n %s [hostname] [T/H/I] [LowWarning HighWarning LowCritical HighCritical]\n\n", progname); 
  fprintf(stdout, "Only the hostname is mandatory. The rest of the arguments are optional\n");
  fprintf(stdout, "T is for Temperature data, H for Humidity Data and I for Illumination data\nExamples:\n");
  fprintf(stdout, "This will return all HVAC data: \n %s 192.168.0.2\n\n", progname); 
  fprintf(stdout, "This will return only Illumination data: \n %s 192.168.0.2 I\n\n", progname); 
  fprintf(stdout, "This will return temperature data with status: \n %s 192.168.0.2 T 65 85 60 90\n\n", progname); 
  fprintf(stdout, "This will return humidity data with status: \n %s 192.168.0.2 H 25.5 50 15 70.5\n", progname); 
  fprintf(stdout, "For further information, please refer to the README file included with the package\n");
  fprintf(stdout, "or available for download from http://www.eesensors.com\n");
  return 0;
}
