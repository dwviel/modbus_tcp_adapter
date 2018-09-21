/**
 * Handles controlmq messaging going to the network.
 */

#include "controlmq_adapter.h"

#include <netinet/in.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>


extern const char* node_IP_root_addr;
extern const int IP_root_addr_max_len;


int controlmq_adapter_connect(int subnetval)
{
    // Send controlMQ connect message to other adapter 

    return 0;
}


int controlmq_send_modbus_client_request(int lowsubval,
					 uint16_t trans_id,
					 uint16_t proto_id,
					 uint16_t length,
					 uint8_t unit_id,
					 uint8_t function,
					 uint16_t request)
{
    // Target IP addr of server side adapter
    char targ_addr_str[INET_ADDRSTRLEN];
    strncpy(targ_addr_str, node_IP_root_addr, INET_ADDRSTRLEN);
    int tmp_len = strnlen(targ_addr_str, IP_root_addr_max_len);
    char lowseg[4];
    sprintf(lowseg, "%d", lowsubval);
    strncpy( (targ_addr_str + tmp_len), lowseg, 3);
    targ_addr_str[INET_ADDRSTRLEN] = '\0';

    // Call ControlMQ send function
/*    return send_reliable_modbus_client_request(targ_addr_str,
					       trans_id,
					       proto_id,
					       length,
					       unit_id,
					       function,
					       request);
*/
    return 0;
}



