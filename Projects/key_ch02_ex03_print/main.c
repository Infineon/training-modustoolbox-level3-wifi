/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Empty PSoC6 Application
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/* FreeRTOS header files */
#include "FreeRTOS.h"
#include "task.h"

/* Configuration file for Wi-Fi */
#include "wifi_config.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "cy_secure_sockets.h"
#include "cy_wcm.h"
#include "stdlib.h"

void wifi_connect(void *arg){
	cy_rslt_t result;
	cy_wcm_connect_params_t connect_param;
	cy_wcm_ip_address_t ip_address;
	uint32_t retry_count;

	/* Configure the interface as a Wi-Fi STA (i.e. Client). */
	cy_wcm_config_t config = {.interface = CY_WCM_INTERFACE_TYPE_STA};

	/* Initialize the Wi-Fi Connection Manager and return if the operation fails. */
	result = cy_wcm_init(&config);

	printf("\nWi-Fi Connection Manager initialized.\n");

	/* Configure the connection parameters for the Wi-Fi interface. */
	memset(&connect_param, 0, sizeof(cy_wcm_connect_params_t));
	memcpy(connect_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
	memcpy(connect_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
	connect_param.ap_credentials.security = WIFI_SECURITY;

	/* Connect to the Wi-Fi AP. */
	for (retry_count = 0; retry_count < MAX_WIFI_CONN_RETRIES; retry_count++)
	{
		printf("Connecting to Wi-Fi AP '%s'\n", connect_param.ap_credentials.SSID);
		result = cy_wcm_connect_ap(&connect_param, &ip_address);

		if (result == CY_RSLT_SUCCESS)
		{
			printf("Successfully connected to Wi-Fi network '%s'.\n",
					connect_param.ap_credentials.SSID);

			// Print IP Address
			if (ip_address.version == CY_WCM_IP_VER_V4)
			{
				printf("IPv4 Address Assigned: %d.%d.%d.%d\n", (uint8_t)ip_address.ip.v4,
						(uint8_t)(ip_address.ip.v4 >> 8), (uint8_t)(ip_address.ip.v4 >> 16),
						(uint8_t)(ip_address.ip.v4 >> 24));
			}
			else if (ip_address.version == CY_WCM_IP_VER_V6)
			{
				printf("IPv6 Address Assigned: %0X:%0X:%0X:%0X\n", (unsigned int)ip_address.ip.v6[0],
						(unsigned int)ip_address.ip.v6[1], (unsigned int)ip_address.ip.v6[2],
						(unsigned int)ip_address.ip.v6[3]);
			}

			// Print Netmask
			printf("Netmask: %d.%d.%d.%d\n", (uint8_t)connect_param.static_ip_settings->netmask.ip.v4,
					(uint8_t)(connect_param.static_ip_settings->netmask.ip.v4 >> 8), (uint8_t)(connect_param.static_ip_settings->netmask.ip.v4 >> 16),
					(uint8_t)(connect_param.static_ip_settings->netmask.ip.v4 >> 24));

			//Print Gateway
			printf("Gateway: %d.%d.%d.%d\n", (uint8_t)connect_param.static_ip_settings->gateway.ip.v4,
					(uint8_t)(connect_param.static_ip_settings->gateway.ip.v4 >> 8), (uint8_t)(connect_param.static_ip_settings->gateway.ip.v4 >> 16),
					(uint8_t)(connect_param.static_ip_settings->gateway.ip.v4 >> 24));

			// Print hostname lookup
			cy_socket_ip_address_t infineonIP;
			cy_socket_gethostbyname("www.infineon.com",CY_SOCKET_IP_VER_V4, &infineonIP);
			printf("www.infineon.com IP Address: %d.%d.%d.%d\n", (uint8_t)infineonIP.ip.v4,
					(uint8_t)(infineonIP.ip.v4 >> 8), (uint8_t)(infineonIP.ip.v4 >> 16),
					(uint8_t)(infineonIP.ip.v4 >> 24));

			// Print MAC Address
			cy_wcm_mac_t MAC_addr;
			cy_wcm_get_mac_addr(CY_WCM_INTERFACE_TYPE_STA, &MAC_addr);
			printf("MAC Address: %X:%X:%X:%X:%X:%X\n", MAC_addr[0], MAC_addr[1], MAC_addr[2], MAC_addr[3], MAC_addr[4], MAC_addr[5]);
			break;
		}
	}
	
	for(;;)
    {
    	if(result == CY_RSLT_SUCCESS)
    	{
    		/* Turn on LED and exit */
    		cyhal_gpio_write(CYBSP_USER_LED,CYBSP_LED_STATE_ON);
    		vTaskDelete(NULL);
    	}
    	else
    	{
    		cyhal_gpio_toggle(CYBSP_USER_LED);
    	}
   		vTaskDelay(100);
    }
}

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port. */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

	/* Initaize LED pin */
    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
	printf("\x1b[2J\x1b[;H");
	printf("============================================================\n");
	printf("ModusToolbox-Level3-WiFi - 2: Print\n");
	printf("============================================================\n\n");

    /* Create the WiFi connection task. It must have a priority of 3 or lower. */
	xTaskCreate(wifi_connect, "wifi_connect_task", 1024, NULL, 1, NULL);

	/* Never Returns */
	vTaskStartScheduler();
}

/* [] END OF FILE */
