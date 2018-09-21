#ifndef CONTROLMQ_ADAPTER
#define CONTROLMQ_ADAPTER

#include <stdint.h>


int controlmq_adapter_connect(int subnetval);

int controlmq_send_modbus_client_request(int lowsubval,
					 uint16_t trans_id,
					 uint16_t proto_id,
					 uint16_t length,
					 uint8_t unit_id,
					 uint8_t function,
					 uint16_t request);



#endif
