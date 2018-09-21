#ifndef MODBUS_ADAPTER
#define MODBUS_ADAPTER

/**
 *  Modbus adapter
 */

/**
 * Run modbus adapater functionality.
 */
int modbustcp_adapter();


/**
 * get the integer that represents the lowest subnet of an
 * IPv4 IP address expressed in "dot" notation.  E.g. 10.22.134.7
 * In this case 7 is the value returned.
 */
int get_lowest_subnet_value(char *ip_str);


#endif
