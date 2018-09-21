/**
 *  Adapter for modbus.
 *  Currently only support IPv4.
 *
 *  ModbusTCP is connection based.  Will have ModbusTCP client
 *  connect to client side adapter, which will send a ControlMQ
 *  message to connect on server side adapter.  If fails, will 
 *  disconnect on client side.
 */


// change perror to log if make daemon!!!!


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
#include <fcntl.h>
#include <sys/file.h>

#include "controlmq_adapter.h"



#define MODBUSPORT 502   // Modbus TCP reserved port

#define BACKLOG 1  // how many pending connections queue will hold


// lowest segment numbers must be the same
// e.g. x.y.z.a and u.v.w.a
const int IP_root_addr_max_len = 12;
const char* adapter_IP_root_addr = "192.168.0.";
const char* modbus_addr_str = "192.168.0.0";

// IP addrs for test
const char* node_IP_addr = "111.99.88.1";
const char* node_IP_root_addr = "111.99.88.";

/** Adapter must support range of IP values.  
 *  ModbusTCP assumed to operate over lowest level subnet only.
 *  I.e. x.y.z.a  where only the values of a are allocated and varry.
 *  ModbusTCP client can connect to multiple servers.
 *  Adapter needs to support a local version of each server IP addr.
 *  This is done by replacing the actual network IP addr with a local
 *  IP addr that uses the same lowest level subnet values.
 *  I.e. x.y.z.a becomes 192.168.0.a, but 0 is reserved.
*/

// Assume only one connection per server for now!!!!
// Use lowest level value as index.  e.g. for x.y.z.a, a is the index
#define MAXMODBUSNODES 256
int client_fd[MAXMODBUSNODES] = {0};  //  new connection on client_fd

#define MAXCONTROLMQDATASIZE 1090

int modbus_fd = 0;

#define MODBUSREQSIZE 10



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


int handle_modbus_client_connect_server_side(char *client_side_ip_addr)
{
    int lowsubval = get_lowest_subnet_value(client_side_ip_addr);

    char adapter_addr_str[INET_ADDRSTRLEN];
    strncpy(adapter_addr_str, adapter_IP_root_addr, INET_ADDRSTRLEN);
    int tmp_len = strnlen(adapter_addr_str, IP_root_addr_max_len);
    char lowseg[4];
    sprintf(lowseg, "%d", lowsubval);
    strncpy( (adapter_addr_str + tmp_len), lowseg, 3);
    adapter_addr_str[INET_ADDRSTRLEN] = '\0';


    if((modbus_fd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 6)) == -1)
    {
	perror("adapter: socket");
	return -1;
    }


    // Bind to the equivilent local addr as lowest segment of client side node
    struct sockaddr_in client_side_addr;
    client_side_addr.sin_family = AF_INET;
    client_side_addr.sin_port = htons(MODBUSPORT);
    inet_pton(AF_INET, adapter_addr_str, &(client_side_addr.sin_addr));

    if((bind(modbus_fd, (struct sockaddr*)&client_side_addr, 
	     sizeof(struct sockaddr_in))) == -1)
    {
	perror("adapter: bind");
	close(modbus_fd);
	modbus_fd = 0;
	return -1;	
    }


    struct sockaddr_in modbus_addr;
    modbus_addr.sin_family = AF_INET;
    modbus_addr.sin_port = htons(MODBUSPORT);
    inet_pton(AF_INET, modbus_addr_str, &(modbus_addr.sin_addr));

    if((connect(modbus_fd, (struct sockaddr*)&modbus_addr, 
		sizeof(struct sockaddr_in))) == -1)
    {
	close(modbus_fd);
	modbus_fd = 0;
	perror("adapter: connect");
	return -1;	
    }


    return 0;
}


int handle_modbus_client_request_server_side(uint16_t trans_id,
					     uint16_t proto_id,
					     uint16_t length,
					     uint8_t unit_id,
					     uint8_t function,
					     uint16_t request)
{
    if(modbus_fd <= 0)
    {
	return -1;
    }

    // modbus buffer
    char modbusbuf[MODBUSREQSIZE] = {0};

    char *buf = modbusbuf;
 
    *(uint16_t*)buf = trans_id;
    buf++;
    *(uint16_t*)buf = proto_id;
    buf++;
    *(uint16_t*)buf = length;
    buf++;
    *(uint8_t*)buf = unit_id;
    buf++;
    *(uint8_t*)buf = function;
    buf++;
    *(uint16_t*)buf = request;

    ssize_t ret = send(modbus_fd, (void*)buf, MODBUSREQSIZE, 0);

    return 0;
}


int handle_modbus_client_request(int lowsubval, char *buf, int numbytes)
{
    if(numbytes < MODBUSREQSIZE)
    {
	// missing data
	return -1;
    }

    uint16_t trans_id = *(uint16_t*)buf;
    buf++;
    uint16_t proto_id = *(uint16_t*)buf;
    buf++;
    uint16_t length = *(uint16_t*)buf;
    buf++;
    uint8_t unit_id = *(uint8_t*)buf;
    buf++;
    uint8_t function = *(uint8_t*)buf;
    buf++;
    uint16_t request = *(uint16_t*)buf;


    return controlmq_send_modbus_client_request(lowsubval,
						trans_id,
						proto_id,
						length,
						unit_id,
						function,
						request);


    return 0;
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
    
    int flags = fcntl(tmp_fd, F_GETFL, 0);
    if(flags == -1)
    {
	return -1;
    }
    fcntl(tmp_fd, F_SETFL, flags | O_NONBLOCK);
    

    inet_ntop(their_addr.ss_family,
	      &(((struct sockaddr_in *)&their_addr)->sin_addr),
	      addr_str, sizeof addr_str);
    //printf("server: got connection from %s\n", addr_str);
    // Get lowest level subnet value, i.e. for x.y.z.a, get a
    int subnetval = get_lowest_subnet_value(addr_str);
    // Lowest subnet 0 is reserved.
    if(subnetval != 0)
    {
	client_fd[subnetval] = tmp_fd;

	// send message to other side adapter to connect
	controlmq_adapter_connect(subnetval); 
    }


    return 0;
}


int close_modbus_client_connection_other_side(int lowest_subnet)
{
    // If the connection is lost on the other adapter side to its
    // modbus device then this side must be closed to inform this
    // side modbus device of loss of connection.
    if( (lowest_subnet < 0) || (lowest_subnet > 255) )
    {
	return -1;
    }

    int res = close(client_fd[lowest_subnet]);
    if(res == -1)
    {
	if(errno == EINTR) // try once more
	{
	    res = close(client_fd[lowest_subnet]);
	    return res;
	}
    }

    return 0;
}


int read_modbus_client_requests()
{
    // Listen for messages from modbus on client_fd
    // client_fd sockets are nonblocking.
    char buf[MAXCONTROLMQDATASIZE];
    buf[MAXCONTROLMQDATASIZE] = '\0';
    int numbytes = 0;
    
    // Node 0 is reserved.  I.e. subnet value 0 is reserved.
    int ii = 1;
    for(ii=1; ii<MAXMODBUSNODES; ii++)
    {
	if(client_fd[ii] != 0)
	{
	    memset(buf, 0, MAXCONTROLMQDATASIZE);
	    if((numbytes = 
		recv(client_fd[ii], buf, MAXCONTROLMQDATASIZE-1, 
		     MSG_WAITALL)) == -1) 
	    {
		//perror("recv");
		continue;
	    }
	    
	    handle_modbus_client_request(ii, buf, numbytes);
	}

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
    listen_addr.sin_port = htons(MODBUSPORT);  // ModbusTCP reserved port number
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

    // Modbus client may open multiple connections with server

    // Call ControlMQ setup and loop here

 
    return 0;
}
