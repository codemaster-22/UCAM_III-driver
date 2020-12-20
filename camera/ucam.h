#include <stdio.h>
#include "uart.h"
#define CAM uart_instance[1]
#define HOST uart_instance[2]
int init_cam();
void get_pic();
void send_full_reset();