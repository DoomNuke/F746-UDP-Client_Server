/*
 * Main_Server.h
 *
 *  Created on: Nov 17, 2025
 *      Author: doomnuke
 */

#ifndef MAIN_SERVER_MAIN_SERVER_H_
#define MAIN_SERVER_MAIN_SERVER_H_

#define MSG_LEN 128
#define PORT_S 8456

#include "lwip.h"
#include "lwip/ip_addr.h"

extern struct netif gnetif;

typedef enum
{
  UART = 0,
  I2C,
  SPI,
}Peri;

typedef struct Task
{
  Peri Peripherial;
  char msg[MSG_LEN];
  char recv_buf[MSG_LEN];
  char result[128];
  char adc_result[64];
  uint32_t adc_known_val;
  uint32_t Measured_Time;
}newTask;


void Main_Server_init(void);


#endif /* MAIN_SERVER_MAIN_SERVER_H_ */
