/**
 *  Adapter for modbus.
 *  Currently only support IPv4.
 */


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
#include <netinet/tcp.h>

#include "controlmq_adapter.h"



#define PORT 502  // Modbus TCP reserved port

#define BACKLOG 1     // how many pending connections queue will hold

const char* listening_IP_addr = "192.168.0.1";

int client_fd = 0;  //  new connection on client_fd



int handle_modbus_client_message(char *buf, int numbytes)
{
    //int type = parse_modbus_message(buf, numbytes);


}



int modbustcp_adapter()
{

    // Create listening socket for stub network for modbusTCP
    // client to connect.
    int sockfd;  // listen on sock_fd
    int yes = 1;
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(PORT);  // ModbusTCP reserved port number
    // User reserved static IP addr for network 
    //inet_pton(AF_INET, listening_IP_addr, &listen_addr.sin_addr);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t listen_addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char addr_str[INET_ADDRSTRLEN];

    // TCP socket
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 6)) == -1) {
	perror("server: socket");
	return -1;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		   sizeof(int)) == -1) {
	perror("setsockopt");
	return -1;
    }
    
    if (setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &yes,
		   sizeof(int)) == -1) {
	perror("setsockopt");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes,
		   sizeof(int)) == -1) {
	perror("setsockopt");
	return -1;
    }


    if (bind(sockfd, (struct sockaddr*)&listen_addr, listen_addrlen) == -1) {
	close(sockfd);
	perror("server: bind");
	return -1;
    }


    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return -1;
    }



    // Need a loop that listens for both incoming connection requests
    // Client
    // May need to make function that is called periodically
    // May need O_NONBLOCK on socket

    // Modbus client may open multiple connections with server???
    while(1)
    {
	// just because this connect on this side worked doesn't
	// mean the other side worked.  If other side fails, must
	// immediately send close();
	sin_size = sizeof their_addr;
	client_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (client_fd == -1) {
	    perror("accept");
	    return -1;;
	}
	
	inet_ntop(their_addr.ss_family,
		  &(((struct sockaddr_in *)&their_addr)->sin_addr),
		  addr_str, sizeof addr_str);
	printf("server: got connection from %s\n", addr_str);
	
	
	// send message to other adapter to connect
	controlmq_adapter_connect();
    }

    // listen for messages from modbus on client_fd
    int MAXDATASIZE = 1090;
    char buf[MAXDATASIZE];
    int numbytes = 0;

    while(1)
    {
	memset(buf, 0, MAXDATASIZE);
	if((numbytes = recv(client_fd, buf, MAXDATASIZE-1, MSG_WAITALL)) == -1) 
	{
	    perror("recv");
	    continue;
	}

	handle_modbus_client_message(buf, numbytes);

    }



}
