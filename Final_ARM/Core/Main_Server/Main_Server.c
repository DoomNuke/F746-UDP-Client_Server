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


static void UDP_Receive_Callback(void *arg, struct udp_pcb *udpcb, struct pbuf *p, const ip_addr_t *ip, uint16_t port)
{
	if (p == NULL) return;

	newTask NewTask;
	int msg_len;
	struct pbuf *pb_tx;

	/*
	 * Starting with getting the packet, processing the data, comparing the strings into
	 * commands and after that we send a response back to the client it came from
	 */

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

		udp_connect(udpcb, ip, port);

		udp_send(udpcb, pb_tx);

		udp_disconnect(udpcb);

		pbuf_free(pb_tx);

		pbuf_free(p);
}

void Main_Server_init(void)
{
	newTask NewTask;
	struct udp_pcb *udpcb;
	int msg_len;
	err_t err;

	udpcb = udp_new();
	if(udpcb == NULL)
	{
		msg_len = snprintf(NewTask.msg,sizeof(NewTask.msg), "Error, couldn't create new UDP protocol block\n\r");
		HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len, 500);
		return;
	}

	err = udp_bind(udpcb, IP_ADDR_ANY, PORT_S);
	if (err == ERR_OK)
	{
		udp_recv(udpcb, UDP_Receive_Callback, NULL);
	}
	else
	{
		msg_len = snprintf(NewTask.msg,sizeof(NewTask.msg), "Error, couldn't bind the address and the port to the server...\n\r");
		HAL_UART_Transmit(&huart3, (uint8_t *)NewTask.msg, msg_len, 500);
	}

}
