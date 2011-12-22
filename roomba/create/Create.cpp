#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>

#include "Packet.h"
#include "Create.h"

#define CREATE_SERIAL_PORT "/dev/ttyUSB0"
#define CREATE_SERIAL_BRATE B57600

Create::Create(int sock, struct sockaddr * createPort, unsigned long connectedHost)
{
	_fd = -1;
	_sock = sock;
	_createPort = createPort;
	_connectedHost = connectedHost;
	isEnding = false;
}

Create::~Create()
{
}

int Create::InitSerial()
{
	_fd = open(CREATE_SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

	if(_fd == -1) // if open is unsuccessful
	{
		printf("Unable to open %s.\n", CREATE_SERIAL_PORT);
		return -1;
	}
	else
	{
		fcntl(_fd, F_SETFL, 0);
		printf("Create serial port opened.\n");
	}
	
	// configure port
	struct termios portSettings;
	if (cfsetispeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetispeed\n");
	if (cfsetospeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetospeed\n");

	// set no parity, stop bits, databits
	portSettings.c_cflag &= ~PARENB;
	portSettings.c_cflag &= ~CSTOPB;
	portSettings.c_cflag &= ~CSIZE;
	portSettings.c_cflag |= CS8;

	if (tcsetattr(_fd, TCSANOW, &portSettings) != 0)
		printf("Failed pushing portSettings\n");
	
	return _fd;
}

void Create::CloseSerial()
{
	close(_fd);
}

void Create::SendSerial(char* buf, int bufLength)
{
	if (_fd == -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return;
	}
	if (write(_fd, buf, bufLength) == -1)
	{
		printf("ERROR: write error occured.\n");
		return;
	}
	printf("bufLength: %d\n", bufLength);
	for (int i = 0; i < bufLength; i++)
	{
		printf("%i ", int(buf[i]));
	}
	printf("\n");
}

int Create::RunSerialListener()
{
	char buf[MAXPACKETSIZE];
	int ret;
	int bufLength;
	int            max_fd;
	fd_set         input;
	struct timeval timeout;
	
	InitSerial();
	
	if (_fd = -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return -1;
	}

	while(1)
	{
		if(isEnding)
			break;
			
		/* Initialize the input set */
		FD_ZERO(&input);
		FD_SET(_fd, &input);
		max_fd = _fd + 1;
		
		/* Initialize the timeout structure */
		timeout.tv_sec  = 10;
		timeout.tv_usec = 0;

		/* Do the select */
		ret = select(max_fd, &input, NULL, NULL, &timeout);

		/* See if there was an error */
		if (ret < 0)
			printf("ERROR: select failed\n");
		else if (ret != 0)
		{
			/* We have input */
			if (FD_ISSET(fd, &input))
			{
				bufLength = read(fd, buf, MAXPACKETSIZE);
				if (sendto(_sock, buf, bufLength, 0, (const struct sockaddr *) _createPort, sizeof(struct sockaddr_in)) < 0) printf("ERROR: sendto\n");
				printf("%c", buf[0]);
			}
		}
		fflush(stdout);
	}
	return 0;
}

int Create::RunUDPListener()
{
	int sock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];

	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) printf("ERROR: Opening socket\n");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CREATE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) 
		printf("ERROR: binding\n");
	fromlen = sizeof(struct sockaddr_in);

	printf("Ready to listen to Create message ...\n");
	while(1)
	{
		if (isEnding)
			break;
			
		bzero(&buf, sizeof(buf));
		bufLength = recvfrom(sock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength < 0) printf("ERROR: recvfrom\n");
	
		if (_connectedHost != from.sin_addr.s_addr)
			continue;

		SendSerial(buf, bufLength);
	}
	CloseSerial(fd);

	printf("Ending RunUDPListener");
	return 0;
}