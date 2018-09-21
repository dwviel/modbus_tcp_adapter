/**
 *  Adapter for modbus.
 *  Currently only support IPv4.
 *
 *  ModbusTCP is connection based.  Will have ModbusTCP client
 *  connect to client side adapter, which will send a ControlMQ
 *  message to connect on server side adapter.  If fails, will 
 *  disconnect on client side.
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

/** Adapter must support range of IP values.  
 *  ModbusTCP assumed to operate over lowest level subnet only.
 *  I.e. x.y.z.a  where only the values of a are allocated and varry.
 *  ModbusTCP client can connect to multiple servers.
 *  Adapter needs to support a local version of each server IP addr.
 *  This is done by replacing the actual network IP addr with a local
 *  IP addr that uses the same lowest level subnet values.
 *  I.e. x.y.z.a becomes 192.168.0.a, but 0 is reserved.
*/

// Assume only one connection per server for now
// Use lowest level value as index.  e.g. for x.y.z.a, a is the index
#define MAXMODBUSNODES 256
int client_fd[MAXMODBUSNODES] = {0};  //  new connection on client_fd



int handle_modbus_client_message(char *buf, int numbytes)
{
    //int type = parse_modbus_message(buf, numbytes);

    return 0;
}


int get_lowest_subnet_value(char *ip_str)
{
    // ip_str expected to be 'dot' notation.  e.g. 145.22.89.3
    // In this case 3 is the value returned.
    char addr_str[INET_ADDRSTRLEN];
    strncpy(addr_str, ip_str, INET_ADDRSTRLEN);
    addr_str[INET_ADDRSTRLEN] = '\0';

    char *substr = NULL;
    substr = strtok(addr_str, ".");
    substr = strtok(NULL, ".");
    substr = strtok(NULL, ".");
    substr = strtok(NULL, ".");

    if(substr != NULL)
    {
	char *end = NULL;
	int lowval = strtoul(substr, &end, 10);
	if( (lowval < 0) || (lowval > 255) )
	{
	    return -1;
	}
	return lowval;
    }

    return -1; // error, or misconfigured server ip addr
}


int accept_modbus_client_connection(int sockfd)
{
    // Just because the connect on this side worked doesn't
    // mean the other side worked.  If other side fails, must
    // immediately send close() on this side and remove fd from
    // array.
    // Socket sockfd is nonblocking.

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char addr_str[INET_ADDRSTRLEN];
    sin_size = sizeof their_addr;
    int tmp_fd = 0;

    tmp_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (tmp_fd == -1) {
	perror("accept");
	return -1;;
    }
    
    inet_ntop(their_addr.ss_family,
	      &(((struct sockaddr_in *)&their_addr)->sin_addr),
	      addr_str, sizeof addr_str);
    //printf("server: got connection from %s\n", addr_str);
    // Get lowest level subnet value, i.e. for x.y.z.a, get a
    int subnetval = get_lowest_subnet_value(addr_str);
    if(subnetval != 0)
    {
	client_fd[subnetval] = tmp_fd;

	// send message to other side adapter to connect
	controlmq_adapter_connect(subnetval); 
    }


    return 0;
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

    // TCP socket
    if ((sockfd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 6)) == -1) {
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
  }

    // listen for messages from modbus on client_fd
    int MAXDATASIZE = 1090;
    char buf[MAXDATASIZE];
    int numbytes = 0;

    while(1)
    {
	memset(buf, 0, MAXDATASIZE);
	//if((numbytes = recv(client_fd, buf, MAXDATASIZE-1, MSG_WAITALL)) == -1) 
	//{
	//    perror("recv");
	//    continue;
	//}

	handle_modbus_client_message(buf, numbytes);

    }



}
