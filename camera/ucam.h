#include <stdio.h>
#include <assert.h>
#define CAM uart_instance[0]
#define uint8_t unsigned char // temporary only, remove afterwards
void init_cam();
void get_pic();
void reset(char tt); // Imediate response with xx = ff

