#ifndef _CAN_TEST_H_
#define _CAN_TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "libsocketcan.h"


typedef unsigned char u8;
typedef unsigned int u32;

#define BUF_SIZ	(255)

#define TRACE		do{printf("%s(%d):%s\n", __FILE__, __LINE__, __func__);}while(0)

void cmd_show_interface(const char *name);

void logPrintTime(void);

void logPrint(char *s);

#endif