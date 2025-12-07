/*
 * Main_Server.c
 *
 *  Created on: Nov 17, 2025
 *      Author: doomnuke
 */

#include "Main_Server.h"

#include <string.h>

#include "lwip.h"
#include "lwip/udp.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"


#include "../Peripherials_Check/Peripherials_Check.h"

/*
 * The UDP receive callback which receives the packets,
 * then we zero out the struct that holds the values for the received buffer,
 * We make sure we add a null terminator just so we can successfully compare strings since we work
 * with strings, then we call the functions inside of each peripherial, allocate the string to the
 * buffer we would send back to the user
 */

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *ip, uint16_t port)
{
	int msg_len;
	newTask NewTask;
	struct pbuf *pb_tx;

	if (p == NULL)
	{
		return;
	}

		memset(&NewTask, 0, sizeof(newTask));
	    size_t copy_len = (p->tot_len < sizeof(NewTask.recv_buf) - 1) ? p->tot_len : sizeof(NewTask.recv_buf) - 1;
	    pbuf_copy_partial(p, NewTask.recv_buf, copy_len, 0);
		NewTask.recv_buf[copy_len] = '\0';



		if (strcmp(NewTask.recv_buf, "I2C") == 0)
		{
			NewTask.Peripherial = I2C;
			I2C_Test();
		}
		else if(strcmp(NewTask.recv_buf, "SPI") == 0)
		{
			NewTask.Peripherial = SPI;
			SPI_Test();
		}
		else if(strcmp(NewTask.recv_buf, "UART") == 0)
		{
			NewTask.Peripherial = UART;
			UART_Test();
		}
		else
		{
			msg_len = snprintf(NewTask.msg, sizeof(NewTask.msg), "No Peripherial Has Selected or the peripherial has not been implemented... \r\n");
			HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len , 500);
		}

		msg_len = snprintf(NewTask.result, sizeof(NewTask.result),
		"Peripherial :%d\r\n ADC Before:%lu\r\nADC After: %s\r\nTime: %lu\r\n",
		NewTask.Peripherial,
		NewTask.adc_known_val,
		NewTask.adc_result,
		NewTask.Measured_Time);

		pb_tx = pbuf_alloc(PBUF_TRANSPORT, msg_len, PBUF_RAM);

		if(pb_tx == NULL)
		{
			msg_len = snprintf(NewTask.msg ,sizeof(NewTask.msg), "Error, couldn't allocate the result, please try again\n\r");
			HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len, 500);
			pbuf_free(p);
			return;
		}

		pbuf_take(pb_tx, NewTask.result, msg_len);

		udp_sendto(pcb, pb_tx, ip, port);

		pbuf_free(pb_tx);

		pbuf_free(p);
}

void Main_Server_init(void)
{
	newTask NewTask;
	struct udp_pcb *pcb;
	int msg_len;
	err_t err;
	ip_addr_t MyIP;

	/*
	 * Creating a new control block which handles all of the UDP operations
	 */
	pcb = udp_new();
	if(pcb == NULL)
	{
		msg_len = snprintf(NewTask.msg,sizeof(NewTask.msg), "Error, couldn't create new UDP protocol block\n\r");
		HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len, 500);
		return;
	}

	/*
	 * Setting manually an IP
	 * and checking if the binding has been successful so we can start accepting the UDP packets
	 */
	IP_ADDR4(&MyIP,192,168,1,40);
	err = udp_bind(pcb, &MyIP, PORT_S);


	if (err == ERR_OK)
	{
		udp_recv(pcb, udp_receive_callback, NULL);
	}
	else
	{
		msg_len = snprintf(NewTask.msg,sizeof(NewTask.msg), "Error, couldn't bind the address and the port to the server...\n\r");
		HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len, 500);
		udp_remove(pcb);
	}

}



