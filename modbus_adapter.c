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


#define PORT 502  // Modbus TCP reserved port

#define BACKLOG 1     // how many pending connections queue will hold

const char* listening_IP_addr = "192.168.0.1";



int modbustcp_adapter()
{

    // Create listening socket for stub network for modbusTCP
    // client to connect.
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    int yes = 1;
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(PORT);  // ModbusTCP reserved port number
    // User reserved static IP addr for network 
    inet_pton(AF_INET, listening_IP_addr, &listen_addr.sin_addr);
    socklen_t listen_addrlen = sizeof(struct sockaddr_in);

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





}
