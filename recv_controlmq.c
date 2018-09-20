// Handle received controlMQ messages

#include "recv_controlmq.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


#define PORT 502  // Modbus TCP reserved port

// This must be read from a config file or configured
// on the modbus tcp device
const char* connecting_IP_addr = "192.168.0.100";

int conn_sockfd;

int recv_controlmq_message()
{
    int msg = 0;

    // Connect message.  i.e. a client
    if(msg == 1)
    {
	int yes = 1;
	struct sockaddr_in connecting_addr;
	connecting_addr.sin_family = AF_INET;
        connecting_addr.sin_port = htons(PORT);  // ModbusTCP reserved port number
	// User reserved static IP addr for network 
	inet_pton(AF_INET, connecting_IP_addr, &connecting_addr.sin_addr);
	socklen_t connecting_addrlen = sizeof(struct sockaddr_in);
      

	if ((conn_sockfd = socket(PF_INET, SOCK_STREAM, 6)) == -1) {
	    perror("server: socket");
	    return -1;
	}

	if (setsockopt(conn_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		       sizeof(int)) == -1) {
	    perror("setsockopt");
	    return -1;
	}
	
	if (setsockopt(conn_sockfd, SOL_TCP, TCP_NODELAY, &yes,
		       sizeof(int)) == -1) {
	    perror("setsockopt");
	    return -1;
	}
	
	if (setsockopt(conn_sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes,
		       sizeof(int)) == -1) {
	    perror("setsockopt");
	    return -1;
	}
	


        if (connect(conn_sockfd, (struct sockaddr*)&connecting_addr, 
		    connecting_addrlen) == -1) {
            close(conn_sockfd);
            perror("client: connect");
            return -1;
        }

    }


}
