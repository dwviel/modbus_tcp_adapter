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

#include "modbus_adapter.h"


// This must be read from a config file or configured
// on the modbus tcp device


int conn_sockfd;

int recv_controlmq_message()
{
    int msg = 0;

    // Connect message.  i.e. a client
    if(msg == 1)
    {
	char *client_side_ip_addr;  // included in message
	if(handle_modbus_client_connect_server_side(client_side_ip_addr) < 0)
	{
	    
	    return -1;
	}
    }

    return 0;
}
