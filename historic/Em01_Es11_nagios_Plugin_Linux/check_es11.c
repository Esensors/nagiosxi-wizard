/* Print Debug Info. Uncomment the #define below to print debugging to stderr */

  #undef DEBUG

/******************************************************************************

	Esensors ES11 Plugin.
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

 $Id: check_es11.c,v 1.2 2008/05/30 Esensors, Inc Exp $
 
******************************************************************************/
const int DEFAULTPORT = 80;
const char *progname = "check_es11";
const char *revision = "$Revision: 1.2 $";
const char *copyright = "2008";
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


int main(int argc, char **argv)
{
  SOCKET s;
  int l, retry;
  float data;
  char iobuf[1024], timestr[32], datachar[7];
  char water_str[82];
  char* pos;
  time_t cur_time;

  progname = argv[0];
  if(argc < 2 || strcmp(argv[1],"--help") == 0) {
    print_help();
    return(3);
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
    if(argc > 2){
		  
      switch (toupper(argv[2][0])){
 	  case 'W':
		if(strlen(argv[3]) != 6){
			printf("Invalid device ID length. Length must be exactly 6 digits. Please check your input arguments again.");
			return(3);	
		}
		strcpy(water_str,"GET /index.html?em");
		strcpy(water_str+18, argv[3]);
		strcpy(water_str+24, " HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n");
		write(s, water_str, 82);
		break;
		
	 case 'R':
		if(strlen(argv[3]) != 6){
			printf("Invalid device ID length. Length must be exactly 6 digits. Please check your input arguments again.\n");
			return(3);	
		}
		strcpy(water_str,"GET /index.html?em");
		strcpy(water_str+18, argv[3]);
		strcpy(water_str+24, "ER HTTP/1.1\r\nUser-Agent: EsensorsPlug\r\nHost: localhost\r\n\r\n");
		//printf("%s", water_str);
		write(s, water_str, 82);
		break;
		
      default:
		write(s, "GET /index.html?em HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 82);
		break;
      }
    }
		
    else{ // Not enough arguments from command line. Use default websensor command.
      write(s, "GET /index.html?em HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 82);
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
    
    pos = strstr(iobuf, "<body>");
    if(pos == 0){
	  printf("Invalid data received.\n");
	  return(3);  
    }
    pos = pos + 20;
	//printf("%s", pos);
    //pos = &iobuf[165];
	
    /* Unsupported command. Exit */
    if(pos[0] == '#'){
	    printf("Invalid Command. Option %c selected may not be available for this websensor.\n", argv[2][0]);
	    return 3;
    }	
    
    /* The http data is not properly formatted. */
    if(pos[5] != 'L'){

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
    
	case 'R':
		printf("Latch Reset. Water Sensor Now Dry\n");
		return(0);
	break;    
	    
	case 'W':
    {
	  datachar[0] = pos[2];
	  if(datachar[0] == 'F'){
		printf("OK ES11 Water Sensor Dry\n");
		return(0);	  
	  }  
	  else if(datachar[0] == 'T'){
	  	printf("CRITICAL ES11 Water Sensor Wet\n");
	  	return(2);
	  }
	  else{
		printf("Unknown status. Please check the data manually.\n");
	  	return(3);
	  }
    }
    break;
    
    default:
    	printf("Invalid option selected. Please choose only 'W' or 'R' as options.\n");
    	return(3);
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
  fprintf (stdout, "Copyright (c) 2006-2007 Esensors, Inc <TechHelp@eEsensors.com>\n");
  fprintf (stdout,("This plugin is written mainly for Nagios, but can work standalone too. \nIt returns the ES11 Flood Sensor status (Dry/Wet)\n\n"));
  print_usage ();
  return 0;
}



int print_usage (void)
{
  fprintf(stdout, "usage: \n %s [hostname] [W/R]\n\n", progname); 
  fprintf(stdout, "W is for Flood Sensor Data\n");
  fprintf(stdout, "R is to reset the latch if the status is latched to Wet\nExamples:\n");
  fprintf(stdout, "This will return ES11 Status: \n %s 192.168.0.2 W 300100\n\n", progname); 
  fprintf(stdout, "This will reset the latch: \n %s 192.168.0.2 R 300100\n\n", progname); 
  fprintf(stdout, "For further information, please refer to the README file included with the package\n");
  fprintf(stdout, "or available for download from http://www.eesensors.com\n");
  return 0;
}
