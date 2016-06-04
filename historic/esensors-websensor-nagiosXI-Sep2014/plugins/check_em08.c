// THIS PLUGIN IS ONLY FOR EM08


/* Print Debug Info. Uncomment the #define below to print debugging to stderr */

//#define DEBUG

/* Number of seconds to timeout */
#define CONNECT_TIMEOUT 5
#define MAX_RETRIES 5
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

Credits:
Duncan Robertson [Duncan.Robertson@vsl.com.au] - 64bits fix
Ali Rahimi [ali@XCF.Berkeley.EDU] - Sockets code
David W. Deley [deleyd@cox.net] - Random numbers

$Id: check_em08.c 192 2010-05-04 18:13:05Z tyarusso $

******************************************************************************/
/** This value is to multiple the Voltage value detected by a fixed constant. (
    * i.e. if you are using a regulator supply to measure AC voltage)
    */
const float VOLTAGE_MULTIPLIER = 1.0;
const float VOLTAGE_OFFSET = 0.0; 

/**
   *  Tells the plugin to reset contact closure on trigger. Change to 0 to turn this off.
   */
const int RESETCONTACT = 1;


const int DEFAULTPORT = 80;
const char *progname = "check_em01";
const char *revision = "$Revision: 2.1 $";
const char *copyright = "2014";
const char *email = "techhelp@eEsensors.com";

typedef int SOCKET;
typedef enum {E_NET_ERRNO=-1, E_NET_OK=0} NetErrnoType;

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>


void     INThandler(int);

static NetErrnoType net_errno = E_NET_OK;
static int saved_errno = 0;
static SOCKET s;


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

#ifdef DEBUG
	fprintf(stderr, "Creating Connection... ");
#endif

	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		NetSetErrno(E_NET_ERRNO);
		return -1;
	}

	NetSetErrno(E_NET_OK);
	return fd;
}



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
	//SOCKET s;
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

/*
This function is called when a timeout has occurred.
It shutsdown the socket and allows the main loop to continue.
*/
void  INThandler(int sig)
{
     signal(SIGALRM, SIG_IGN);
     shutdown(s, SHUT_RDWR);
     alarm(CONNECT_TIMEOUT);
     signal(SIGALRM, INThandler);
}

main(int argc, char **argv)
{
	int l, retry, i, random_backoff;
	long ms;
	float data;
	char iobuf[1024], timestr[32], datachar[7];
	char* pos;
	char rcvd_checksum, calc_checksum;
	time_t start_time, cur_time;
	double r, x;
	SOCKET contacts;

	progname = argv[0];
	if(argc < 2 || strcmp(argv[1],"--help") == 0) {
		print_help();
		return(3);
	}

	time(&cur_time);
	sprintf(timestr, "%s", ctime(&cur_time));

	signal(SIGALRM, INThandler);
	alarm(CONNECT_TIMEOUT);

	

	
	for(retry = 0; retry < MAX_RETRIES; retry++){

		srand((unsigned int)time(NULL));

		r = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) ); 
		x = (r * 20); 	
		random_backoff = (int) x; 

#ifdef DEBUG
		fprintf(stderr, "Sleeping: %d ", 100+random_backoff);
#endif
		for(ms=0; ms < 100+random_backoff; ms++){
			usleep(5000); 
		}


		/* make connection to websensor */
		s = connectWebsensor(argv[1], DEFAULTPORT);
#ifdef DEBUG
		fprintf(stderr, "Socket created ");
#endif

		if(NetErrNo() != 0){
#ifdef DEBUG
			fprintf(stderr, "Could not connect to Websensor because %s, will retry %d more times.\n", NetErrStr(), 10-retry);
#endif
			shutdown(s, SHUT_RDWR);
			continue;
		}


		/* send HTTP GET request to obtain data */
		if(argc>2){

			switch (toupper(argv[2][0])){
	

	default:
		write(s, "GET /ssettings.xml HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 83);
		break;
			}
		}

		else{ // Not enough arguments from command line. Use default websensor command.
			write(s, "GET /ssettings.xml HTTP/1.1\r\nUser-Agent: EsensorsPlugin\r\nHost: localhost\r\n\r\n", 83);
			
		}

#ifdef DEBUG
		fprintf(stderr, "Wrote to Socket ");
#endif

		l = read(s, iobuf, sizeof(iobuf));

#ifdef DEBUG
		fprintf(stderr, "Read from socket ");
#endif
	
		/* No data returned from websensor. Will retry again. */

		if(l<=0){
#ifdef DEBUG
			fprintf(stderr, "No Data Read, will retry %d more times.\n", MAX_RETRIES-retry);
#endif
			shutdown(s, SHUT_RDWR);
			continue;
		}

		//pos = strstr(iobuf, "<body>");
		//if(pos==NULL) {printf("NULL was found");return 3;}
		pos = strstr(iobuf, "<sensorsSW>");
		if(pos!=NULL)
		{
			pos=strstr(iobuf, "<sid0>");
			if(pos!=NULL){
				break; /* All data looks good. Break out of loop. */
			}
			else{
				printf("Invalid data received.\n");
				return(3);  
			}
		}



		//Search for the sensor data string. This is removed. AVC. XML parser method used.
		
	}

	time(&cur_time);
	sprintf(timestr, "%s", ctime(&cur_time));

    /* Retried 3 times earlier and still no good data. Time to exit */
	if(retry >= MAX_RETRIES){
		printf("NO DATA\n");
		return 3;
	}
	else{
		shutdown(s, SHUT_RDWR);
	}

	if(argc > 2){
		switch(toupper(argv[2][0])){

		
	case 'G':  //Cacti/RRDTool Output TempF:** Humid:**
		{
		
				//Check if SHT Sensor is enabled
				//========================================================================================================
			    const char *shtCheck1=strstr(iobuf, "<sht>")+5;//adding 5 to indicate length of <tm0>
			    const char *shtCheck2 = strstr(iobuf, "</sht>");
			    size_t len = shtCheck2-shtCheck1;
			    char *shtStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(shtStatus, shtCheck1, len);
			    shtStatus[len] = '\0';
			    if(strcmp(shtStatus,"inline") == 0){//SHT Sensor is present/ enabled. Get readings.

					//Get Temperature Data
				    const char *p1 = strstr(iobuf, "<tm0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</tm0>");
				    size_t len = p2-p1;
				    char *shtTemp = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(shtTemp, p1, len);
				    shtTemp[len] = '\0';
				    //printf("%s\n", res);
					
					//Get Humidity Data
					const char *p1 = strstr(iobuf, "<hu0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</hu0>");
				    size_t len = p2-p1;
				    char *shtHum = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(shtHum, p1, len);
				    shtHum[len] = '\0';

					//Get Temperature Unit
				    const char *p3 = strstr(iobuf, "<tun0>")+6;
				    const char *p4 = strstr(iobuf, "</tun0>");
				    len = p4-p3;
				    char *tempUnit = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(tempUnit, p3, len);
				    tempUnit[len] = '\0';
			    			    
					printf("Temp:%3.2f\n", shtTemp);	
					printf("Humid:%3.2f\n", shtHum);
				}
				else{ // SHT Sensor is NOT present/ enabled.
					printf("Temp:0.0\n");		
					printf("Humid:0.0\n");
				}
				
				//Illumination Sensor
				//Check if Sensor is enabled
				//========================================================================================================
			    const char *illumCheck1=strstr(iobuf, "<ilum>")+6;//adding 5 to indicate length of <tm0>
			    const char *illumCheck2 = strstr(iobuf, "</ilum>");
			    size_t len = illumCheck2-illumCheck1;
			    char *illumStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(illumStatus, illumCheck1, len);
			    illumStatus[len] = '\0';
			    if(strcmp(illumStatus,"inline") == 0){//illumination Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<il0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</il0>");
				    size_t len = p2-p1;
				    char *illum = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(illum, p1, len);
				    illum[len] = '\0';
					printf("Illum:%3.2f\n", illum);
					
				}
				else{//illumination Sensor is NOT present/ enabled.
					printf("Illum:0.0\n");
				}		
			
				//Check if Door Switch/ contact Sensor is enabled
				//========================================================================================================
			    const char *doorSWCheck1=strstr(iobuf, "<ecin>")+6;//adding 5 to indicate length of <tm0>
			    const char *doorSWCheck2 = strstr(iobuf, "</ecin>");
			    size_t len = doorSWCheck2-doorSWCheck1;
			    char *doorSWStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(doorSWStatus, doorSWCheck1, len);
			    doorSWStatus[len] = '\0';
			    if(strcmp(doorSWStatus,"inline") == 0){//Door Switch/ contact Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<cin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</cin>");
				    size_t len = p2-p1;
				    char *doorSW = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(doorSW, p1, len);
				    doorSW[len] = '\0';				   
					if(strcmp(doorSW,"1") == 0){
						printf("Door:1\n");						
					}
					else  if(strcmp(doorSW,"0") == 0){
						printf("Door:0\n");	
					}	
				}
				else{// Door Switch Sensor is NOT present/ enabled.
					printf("Door:0\n");	
				}
				
				//Check if Thermistor Sensor is enabled
				//========================================================================================================
			    const char *thmCheck1=strstr(iobuf, "<ethm>")+6;//adding 5 to indicate length of <tm0>
			    const char *thmCheck2 = strstr(iobuf, "</ethm>");
			    size_t len = thmCheck2-thmCheck1;
			    char *thmStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(thmStatus, thmCheck1, len);
			    thmStatus[len] = '\0';
			    if(strcmp(thmStatus,"inline") == 0){//External Thermistor Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<thm>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</thm>");
				    size_t len = p2-p1;
				    char *thmTemp = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(thmTemp, p1, len);
				    thmTemp[len] = '\0';
				    //printf("%s\n", res);

				    const char *p3 = strstr(iobuf, "<tun0>")+6;
				    const char *p4 = strstr(iobuf, "</tun0>");
				    len = p4-p3;
				    char *tempUnit = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(tempUnit, p3, len);
				    tempUnit[len] = '\0';
				   

					strncpy(datachar, pos+4, 6);
					datachar[6] = '\0';
					data = myatof(thmTemp);
					data += 0.01;
					printf("TempTHM:%3.2f\n", thmTemp);
					
				}
				else{// Thermistor Sensor is NOT present/ enabled.
					printf("TempTHM:0.0\n", thmTemp);
				}
				
				//Check if Voltage Sensor is enabled
				//========================================================================================================
			    const char *vinCheck1=strstr(iobuf, "<evin>")+6;//adding 5 to indicate length of <tm0>
			    const char *vinCheck2 = strstr(iobuf, "</evin>");
			    size_t len = vinCheck2-vinCheck1;
			    char *vinStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(vinStatus, vinCheck1, len);
			    vinStatus[len] = '\0';
			    if(strcmp(vinStatus,"inline") == 0){//Voltage Sensor is present/ enabled. Get readings.
				    const char *p1 = strstr(iobuf, "<vin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</vin>");
				    size_t len = p2-p1;
				    char *voltage = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(voltage, p1, len);
				    voltage[len] = '\0';

					strncpy(datachar, pos+21, 5);
					datachar[5] = '\0';
					data = myatof(voltage);
					//data += 0.01;
					printf("Voltage:%3.2f\n", data);					
				}
				else{// Voltage Sensor is NOT present/ enabled.
					printf("Voltage:0.0\n");
				}
				
				//Check if Flood Sensor is enabled
				//========================================================================================================
			    const char *floodCheck1=strstr(iobuf, "<efld>")+6;//adding 5 to indicate length of <tm0>
			    const char *floodCheck2 = strstr(iobuf, "</efld>");
			    size_t len = floodCheck2-floodCheck1;
			    char *floodStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(floodStatus, floodCheck1, len);
			    floodStatus[len] = '\0';			 
			    if(strcmp(floodStatus,"inline") == 0){//Door Switch/ contact Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<fin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</fin>");
				    size_t len = p2-p1;
				    char *flood = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(flood, p1, len);
				    flood[len] = '\0';
			        if(strcmp(flood,"0") == 0){
						printf("Flood:0\n");						
				    }
				    else  if(strcmp(flood,"1") == 0){
						printf("Flood:1\n");						
				    }				    
			}
			else{// Flood Sensor is NOT present/ enabled.
				printf("Flood:0\n");	
			}
				
				return(0);
		}
		break;
		
		
		
	case 'T': 
		{

			    //Check if Sensor is enabled
			    const char *shtCheck1=strstr(iobuf, "<sht>")+5;//adding 5 to indicate length of <tm0>
			    const char *shtCheck2 = strstr(iobuf, "</sht>");
			    size_t len = shtCheck2-shtCheck1;
			    char *shtStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(shtStatus, shtCheck1, len);
			    shtStatus[len] = '\0';
			    if(strcmp(shtStatus,"inline") == 0){//SHT Sensor is present/ enabled. Get readings.


				    const char *p1 = strstr(iobuf, "<tm0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</tm0>");
				    size_t len = p2-p1;
				    char *shtTemp = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(shtTemp, p1, len);
				    shtTemp[len] = '\0';
				    //printf("%s\n", res);

				    const char *p3 = strstr(iobuf, "<tun0>")+6;
				    const char *p4 = strstr(iobuf, "</tun0>");
				    len = p4-p3;
				    char *tempUnit = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(tempUnit, p3, len);
				    tempUnit[len] = '\0';
			    			    


					strncpy(datachar, pos+5, 5);
					datachar[5] = '\0';
					data = myatof(shtTemp);
					data += 0.01;
					if(argc != 7){
						printf("(No limits specified) Temperature: %s %s | Temp%s=%3.2f\n", shtTemp, tempUnit, tempUnit, data);
						return(0);
					}
					if(data < myatof(argv[5]) || data > myatof(argv[6])){
						printf("CRITICAL ( %s< or >%s ) Temperature: %s %s | Temp%s=%3.2f\n", argv[5], argv[6], shtTemp, tempUnit, tempUnit, data);
						return(2);
					}
					else if(data < myatof(argv[3]) || data > myatof(argv[4])){
						printf("WARNING ( %s< or >%s ) Temperature: %s %s | Temp%s=%3.2f\n", argv[3], argv[4], shtTemp, tempUnit, tempUnit, data);
						return(1);
					}
					else{
						printf("OK Temperature: %s %s | Temp%s=%3.2f\n", shtTemp, tempUnit, tempUnit, data);
						return(0);
					}
				}
				else{ // SHT Sensor is NOT present/ enabled.
					printf("Temperature Sensor N/A");
					return(0);
				}
		}
		break;

	case 'H': 
		{
			    //Check if Sensor is enabled
			    const char *shtCheck1=strstr(iobuf, "<sht>")+5;//adding 5 to indicate length of <tm0>
			    const char *shtCheck2 = strstr(iobuf, "</sht>");
			    size_t len = shtCheck2-shtCheck1;
			    char *shtStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(shtStatus, shtCheck1, len);
			    shtStatus[len] = '\0';
			    if(strcmp(shtStatus,"inline") == 0){//SHT Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<hu0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</hu0>");
				    size_t len = p2-p1;
				    char *shtHum = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(shtHum, p1, len);
				    shtHum[len] = '\0';


					strncpy(datachar, pos+13, 4);
					datachar[4] = '\0';
					data = myatof(shtHum);
					data += 0.01;
					if(argc != 7){
						printf("(No limits specified) %s% | Humid=%3.2f\n", shtHum, data);
						return(0);
					}
					if(data < myatof(argv[5]) || data > myatof(argv[6])){
						printf("CRITICAL ( %s< or >%s ) Humidity: %s% | Humid=%3.2f\n", argv[5], argv[6], shtHum, data);
						return(2);
					}
					else if(data < myatof(argv[3]) || data > myatof(argv[4])){
						printf("WARNING ( %s< or >%s ) Humidity: %s% | Humid=%3.2f\n", argv[3], argv[4], shtHum, data);
						return(1);
					}
					else{
						printf("OK Humidity: %s% | Humid=%3.2f\n", shtHum, data);
						return(0);
					}
				}
				else{// Humidity Sensor is NOT present/ enabled.
					printf("Humidity Sensor N/A");
					return(0);
				}
		}
		break;

	case 'I': 
		{
			    //Check if Sensor is enabled
			    const char *illumCheck1=strstr(iobuf, "<ilum>")+6;//adding 5 to indicate length of <tm0>
			    const char *illumCheck2 = strstr(iobuf, "</ilum>");
			    size_t len = illumCheck2-illumCheck1;
			    char *illumStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(illumStatus, illumCheck1, len);
			    illumStatus[len] = '\0';
			    if(strcmp(illumStatus,"inline") == 0){//illumination Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<il0>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</il0>");
				    size_t len = p2-p1;
				    char *illum = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(illum, p1, len);
				    illum[len] = '\0';

					strncpy(datachar, pos+21, 5);
					datachar[5] = '\0';
					data = myatof(illum);
					data += 0.01;
					if(argc != 7){
						printf("(No limits specified) Illumination: %s | Illum=%3.2f\n", illum, data);
						return(0);
					}
					if(data < myatof(argv[5]) || data > myatof(argv[6])){
						printf("CRITICAL ( %s< or >%s ) Illumination: %s | Illum=%3.2f\n", argv[5], argv[6], illum, data);
						return(2);
					}
					else if(data < myatof(argv[3]) || data > myatof(argv[4])){
						printf("WARNING ( %s< or >%s ) Illumination: %s | Illum=%3.2f\n", argv[3], argv[4], illum, data);
						return(1);
					}
					else{
						printf("OK Illumination: %s | Illum=%3.2f\n", illum,data);
						return(0);
					}
				}
				else{//illumination Sensor is NOT present/ enabled.
					printf("Illumination Sensor N/A");
					return(0);
				}

		}
		break;

	case 'D':
		{
			    //Check if Door Switch/ contact Sensor is enabled
			    const char *doorSWCheck1=strstr(iobuf, "<ecin>")+6;//adding 5 to indicate length of <tm0>
			    const char *doorSWCheck2 = strstr(iobuf, "</ecin>");
			    size_t len = doorSWCheck2-doorSWCheck1;
			    char *doorSWStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(doorSWStatus, doorSWCheck1, len);
			    doorSWStatus[len] = '\0';
			    if(strcmp(doorSWStatus,"inline") == 0){//Door Switch/ contact Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<cin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</cin>");
				    size_t len = p2-p1;
				    char *doorSW = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(doorSW, p1, len);
				    doorSW[len] = '\0';
				    if(myatof(argv[3])==0){//Alarm when Door is open.
					    if(strcmp(doorSW,"1") == 0){
							printf("OK Contacts/ Door Closed. | Contacts=1\n");
							return(0);
					    }
					    else  if(strcmp(doorSW,"0") == 0){
							printf("CRITICAL Contacts/ Door Open! | Contacts=0\n");
							return(2);
					    }
					    else{
						printf("WARNING Unknown status. Please Check Door.\n");
						return(1);
					    }
				    }
				    else{//Alarm when Door is Closed.
					    if(strcmp(doorSW,"1") == 0){
							printf("CRITICAL Contacts/ Door Closed! | Contacts=1\n");
							return(2);
					    }
					    else  if(strcmp(doorSW,"0") == 0){
							printf("OK Contacts/ Door Open. | Contacts=0\n");							
							return(0);
					    }
						else{
						printf("WARNING Unknown status. Please Check Door.\n");
						return(1);
					    }
				    }
				   
			}
			else{// Door Switch Sensor is NOT present/ enabled.
				printf("Door Switch Sensor N/A.\n");
				return(1);
			}
		}
		break;

	case 'R':
		{
                            //Check if Thermistor Sensor is enabled
			    const char *thmCheck1=strstr(iobuf, "<ethm>")+6;//adding 5 to indicate length of <tm0>
			    const char *thmCheck2 = strstr(iobuf, "</ethm>");
			    size_t len = thmCheck2-thmCheck1;
			    char *thmStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(thmStatus, thmCheck1, len);
			    thmStatus[len] = '\0';
			    if(strcmp(thmStatus,"inline") == 0){//External Thermistor Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<thm>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</thm>");
				    size_t len = p2-p1;
				    char *thmTemp = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(thmTemp, p1, len);
				    thmTemp[len] = '\0';
				    //printf("%s\n", res);

				    const char *p3 = strstr(iobuf, "<tun0>")+6;
				    const char *p4 = strstr(iobuf, "</tun0>");
				    len = p4-p3;
				    char *tempUnit = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(tempUnit, p3, len);
				    tempUnit[len] = '\0';
				   

					strncpy(datachar, pos+4, 6);
					datachar[6] = '\0';
					data = myatof(thmTemp);
					data += 0.01;
					if(argc != 7){
						printf("(No limits specified) Thermistor Temp: %s %s | XTemp%s=%3.2f\n", thmTemp, tempUnit, tempUnit, data);
						return(0);
					}
					if(data < myatof(argv[5]) || data > myatof(argv[6])){
						printf("CRITICAL ( %s< or >%s ) Ext. Temperature: %s %s | XTemp%s=%3.2f\n", argv[5], argv[6], thmTemp, tempUnit, tempUnit, data);
						return(2);
					}
					else if(data < myatof(argv[3]) || data > myatof(argv[4])){
						printf("WARNING ( %s< or >%s ) Ext. Temperature: %s %s | XTemp%s=%3.2f\n", argv[3], argv[4], thmTemp, tempUnit, tempUnit, data);
						return(1);
					}
					else{
						printf("OK Ext. Temperature: %s %s | XTemp%s=%3.2f\n", thmTemp, tempUnit, tempUnit, data);
						return(0);
					}
				}
				else{// Thermistor Sensor is NOT present/ enabled.
					printf("Thermistor Sensor N/A.\n");
					return(1);
				}
		}
		break;

	case 'V':
		{
			    //Check if Voltage Sensor is enabled
			    const char *vinCheck1=strstr(iobuf, "<evin>")+6;//adding 5 to indicate length of <tm0>
			    const char *vinCheck2 = strstr(iobuf, "</evin>");
			    size_t len = vinCheck2-vinCheck1;
			    char *vinStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(vinStatus, vinCheck1, len);
			    vinStatus[len] = '\0';
			    if(strcmp(vinStatus,"inline") == 0){//Voltage Sensor is present/ enabled. Get readings.
				    const char *p1 = strstr(iobuf, "<vin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</vin>");
				    size_t len = p2-p1;
				    char *voltage = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(voltage, p1, len);
				    voltage[len] = '\0';

					strncpy(datachar, pos+21, 5);
					datachar[5] = '\0';
					data = myatof(voltage);
					//data += 0.01;
						
					if(argc != 7){
						printf("(No limits specified) Voltage: %3.2f V | Voltage=%3.2f\n", data,data);
						return(0);
					}
					if(data < myatof(argv[5]) || data > myatof(argv[6])){
						printf("CRITICAL ( %s< or >%s ) Voltage: %3.2f V | Voltage=%3.2f\n", argv[5], argv[6], data, data);
						return(2);
					}
					else if(data < myatof(argv[3]) || data > myatof(argv[4])){
						printf("WARNING ( %s< or >%s ) Voltage: %3.2f V | Voltage=%3.2f\n", argv[3], argv[4], data, data);
						return(1);
					}

					else{
						printf("OK Voltage: %3.2f V | Voltage=%3.2f\n", data,data);
						return(0);
					}
				}
				else{// Voltage Sensor is NOT present/ enabled.
					printf("AC Voltage Sensor N/A.\n");
					return(1);
				}
		}
		break;
	case 'F':
		{
			    //Check if Flood Sensor is enabled
			    const char *floodCheck1=strstr(iobuf, "<efld>")+6;//adding 5 to indicate length of <tm0>
			    const char *floodCheck2 = strstr(iobuf, "</efld>");
			    size_t len = floodCheck2-floodCheck1;
			    char *floodStatus = (char*)malloc(sizeof(char)*(len+1));
			    strncpy(floodStatus, floodCheck1, len);
			    floodStatus[len] = '\0';			 
			    if(strcmp(floodStatus,"inline") == 0){//Door Switch/ contact Sensor is present/ enabled. Get readings.

				    const char *p1 = strstr(iobuf, "<fin>")+5;//adding 5 to indicate length of <tm0>
				    const char *p2 = strstr(iobuf, "</fin>");
				    size_t len = p2-p1;
				    char *flood = (char*)malloc(sizeof(char)*(len+1));
				    strncpy(flood, p1, len);
				    flood[len] = '\0';
			            if(strcmp(flood,"0") == 0){
						printf("OK Flood Sensor DRY. | Flood Sensor=0\n");
						return(0);
				    }
				    else  if(strcmp(flood,"1") == 0){
						printf("CRITICAL Flood Sensor WET! | Flood Sensor=1\n");
						return(2);
				    }
				    else{
					printf("WARNING Unknown status. Please Check Sensor.\n");
					return(1);
				    }
			}
			else{// Flood Sensor is NOT present/ enabled.
				printf("Flood Sensor N/A.\n");
				return(1);
			}
		}
		break;

	default : 
		printf("Please choose only 'T', 'H', 'I', 'R', 'V' , 'F' , 'D' or 'C'.\n Please refer to README for further instructions.\n");
		break;
		}
	}
	else{
		iobuf[195] = 0;
		fprintf(stderr, "2");
		printf("%s\t%s", pos+2, timestr);
		fprintf(stderr, "3");
	}

	return 0;
}

int print_help (void)
{
	fprintf (stdout, "Copyright (c) 2005-2009 Esensors, Inc <TechHelp@eEsensors.com>\n");
	fprintf (stdout,("This plugin is written mainly for Nagios/Cacti/RRDTool, but can work standalone too. \nIt returns the HVAC data from the EM01 Websensor\n\n"));
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
	fprintf(stdout, "This will return humidity data with status: \n %s 192.168.0.2 H 25.5 50 15 70.5\n\n", progname); 
	fprintf(stdout, "This will return Cacti/RRDTool format: \n %s 192.168.0.2 G\n\n", progname); 	
	fprintf(stdout, "For further information, please refer to the README file included with the package\n");
	fprintf(stdout, "or available for download from http://www.eesensors.com\n");
	return 0;
}
