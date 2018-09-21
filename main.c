/**
 *  ModbusTCP adapter for ControlMQ.
 *  Interposes on adapter platform between ModbusTCP endpoint
 *  and network.  Deployed in pairs, onne on each end of network,
 *  such that the two adapters convert the ModbusTCP message to 
 *  ControlMQ format, send that over the network, and then
 *  convert it back to ModbusTCP format for delivery to the 
 *  receiving device.
 */

#include "modbus_adapter.h"
#include "controlmq_adapter.h"

int main(int argc, char *argv[])
{
//    char *ip_str = "1.2.3.400";
    //  int val = get_lowest_subnet_value(ip_str);

    int val = controlmq_send_modbus_client_request(111, 1, 2, 3, 4, 5, 6);





    modbustcp_adapter();

    return 0;
}
