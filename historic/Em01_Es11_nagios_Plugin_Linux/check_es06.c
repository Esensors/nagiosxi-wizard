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
  char shttp[200];
  char* pos;
  time_t cur_time;

  progname = argv[0];
  if(argc < 3 || strcmp(argv[1],"--help") == 0) {
    print_help();
    exit(3);
  }

  sprintf(shttp, "GET /index.html?em%s HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", argv[2]);
 
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
    //write(s, "GET /index.html?em123456 HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 82);
    write(s, shttp, 82);		

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
	
    /* The http data is not properly formatted. */
    if(pos[2] != 'E') {
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

  sprintf(shttp, "OK R1:%s R2:%s CC1:%s CC2:%s CC3:%s CC4:%s [%s]\n",
 	(pos[7] == 'F'? "Off" : "On"),
  	(pos[8] == 'F'? "Off" : "On"),
	(pos[9] == '1'? "High" : "Low"),
        (pos[10] == '1'? "High" : "Low"),
        (pos[11] == '1'? "High" : "Low"),
        (pos[12] == '1'? "High" : "Low"),
 	argv[2]
  );

  printf("%s\t%s", shttp, timestr);

  //printf("OK Voltage: %s V\n", datachar);
  //return(0);
    //iobuf[195] = 0;
    //printf("%s\t%s", pos+2, timestr);

  return 0;
}

int print_help (void)
{
  fprintf (stdout, "Copyright (c) 2005 Esensors, Inc <TechHelp@eEsensors.com>\n");
  fprintf (stdout,("This plugin is written mainly for Nagios, but can work standalone too. \nIt returns the Es06 data from the EM01 Websensor\n\n"));
  print_usage ();
  return 0;
}



int print_usage (void)
{
  fprintf(stdout, "usage: \n %s [hostname] [DeviceID]\n\n", progname);
  fprintf(stdout, "The hostname and DeviceID are  mandatory.\n");
  fprintf(stdout, "or available for download from http://www.eesensors.com\n");
  return 0;
}
