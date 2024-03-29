/******************************************************************************
* File Name:   mqtt_task.c
*
* Description: This file contains the task that handles initialization & 
*              connection of Wi-Fi and the MQTT client. The task then starts 
*              the subscriber and the publisher tasks. The task also handles
*              all the cleanup operations to gracefully terminate the Wi-Fi and
*              MQTT connections in case of any failure.
*
* Related Document: See README.md
*
*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
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
// PSoC MCU Headers
#include "cyhal.h"
#include "cybsp.h"

// Middleware Headers
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "GUI.h"
#include "cy_retarget_io.h"

/* Task header files */
#include "mqtt_task.h"
#include "subscriber_task.h"
#include "publisher_task.h"
#include "temperature_task.h"
#include "display_task.h"
#include "capsense_task.h"

/* Configuration file for Wi-Fi and MQTT client */
#include "wifi_config.h"
#include "mqtt_client_config.h"

// Wi-Fi Headers
#include "cy_wcm.h"
#include "cy_network_mw_core.h"
#include "cy_nw_helper.h"
#include "cy_mqtt_api.h"
#include "clock.h"

/******************************************************************************
* Macros
******************************************************************************/
/* Queue length of a message queue that is used to communicate the status of 
 * various operations.
 */
#define MQTT_TASK_QUEUE_LENGTH           (3u)

/* Time in milliseconds to wait before creating the publisher task. */
#define TASK_CREATION_DELAY_MS           (2000u)

/* Flag Masks for tracking which cleanup functions must be called. */
#define WCM_INITIALIZED                  (1lu << 0)
#define WIFI_CONNECTED                   (1lu << 1)
#define LIBS_INITIALIZED                 (1lu << 2)
#define BUFFER_INITIALIZED               (1lu << 3)
#define MQTT_INSTANCE_CREATED            (1lu << 4)
#define MQTT_CONNECTION_SUCCESS          (1lu << 5)
#define MQTT_MSG_RECEIVED                (1lu << 6)

/* Macro to check if the result of an operation was successful and set the 
 * corresponding bit in the status_flag based on 'init_mask' parameter. When
 * it has failed, print the error message and return the result to the
 * calling function.
 */
#define CHECK_RESULT(result, init_mask, error_message...)      \
                     do                                        \
                     {                                         \
                         if ((int)result == CY_RSLT_SUCCESS)   \
                         {                                     \
                             status_flag |= init_mask;         \
                         }                                     \
                         else                                  \
                         {                                     \
                             printf(error_message);            \
                             return result;                    \
                         }                                     \
                     } while(0)

/******************************************************************************
* Global Variables
*******************************************************************************/
/* MQTT connection handle. */
cy_mqtt_t mqtt_connection;

/* Queue handle used to communicate results of various operations - MQTT 
 * Publish, MQTT Subscribe, MQTT connection, and Wi-Fi connection between tasks
 * and callbacks.
 */
QueueHandle_t mqtt_task_q;

/* Flag to denote initialization status of various operations. */
uint32_t status_flag;

/* Pointer to the network buffer needed by the MQTT library for MQTT send and
 * receive operations.
 */
uint8_t *mqtt_network_buffer = NULL;

// Defined in main.c
extern bool isConnected;
extern SemaphoreHandle_t isConnectedSemaphore;

/* Variables to hold string representation of the IP address (dot separated) */
static char ipAddrString[16];
static char ipAddrStringV6[39];

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static cy_rslt_t wifi_connect(void);
static cy_rslt_t mqtt_init(void);
static cy_rslt_t mqtt_connect(void);
void mqtt_event_callback(cy_mqtt_t mqtt_handle, cy_mqtt_event_t event, void *user_data);
static void cleanup(void);
static cy_rslt_t mqtt_get_unique_client_identifier(char *mqtt_client_identifier);

/******************************************************************************
 * Function Name: mqtt_client_task
 ******************************************************************************
 * Summary:
 *  Task for handling initialization & connection of Wi-Fi and the MQTT client.
 *  The task also creates and manages the subscriber and publisher tasks upon 
 *  successful MQTT connection. The task also handles the WiFi and MQTT
 *  connections by initiating reconnection on the event of disconnections.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void mqtt_client_task(void *pvParameters){

	/* Structures that store the data to be sent/received to/from various
	 * message queues.
	 */
	mqtt_task_cmd_t mqtt_status;

	/* Configure the Wi-Fi interface as a Wi-Fi STA (i.e. Client). */
	cy_wcm_config_t config = {.interface = CY_WCM_INTERFACE_TYPE_STA};

    /* To avoid compiler warnings */
    (void)pvParameters;

    /* Create a message queue to communicate with other tasks and callbacks. */
	mqtt_task_q = xQueueCreate(MQTT_TASK_QUEUE_LENGTH, sizeof(mqtt_task_cmd_t));

	// Take the connection semaphore
	xSemaphoreTake(isConnectedSemaphore, portMAX_DELAY);

	/* Initialize the Wi-Fi Connection Manager and jump to the cleanup block
	 * upon failure.
	 */
	if (CY_RSLT_SUCCESS != cy_wcm_init(&config)){
		printf("\nWi-Fi Connection Manager initialization failed!\n");
		goto exit_cleanup;
	}

	/* Set the appropriate bit in the status_flag to denote successful
	 * WCM initialization.
	 */
	status_flag |= WCM_INITIALIZED;
	printf("\nWi-Fi Connection Manager initialized.\n");

	/* Initiate connection to the Wi-Fi AP and cleanup if the operation fails. */
	if (CY_RSLT_SUCCESS != wifi_connect()){
		goto exit_cleanup;
	}

	/* Set-up the MQTT client and connect to the MQTT broker. Jump to the
	 * cleanup block if any of the operations fail.
	 */
	if ( (CY_RSLT_SUCCESS != mqtt_init()) || (CY_RSLT_SUCCESS != mqtt_connect()) ){
		isConnected = false;
		goto exit_cleanup;

	}
	else{
		isConnected = true;
	}

    // Only start subscriber and publisher task if we are connected to wifi and broker
    if(isConnected){
		/* Create the subscriber task and cleanup if the operation fails. */
		if (pdPASS != xTaskCreate(subscriber_task, "Subscriber task", SUBSCRIBER_TASK_STACK_SIZE, NULL, SUBSCRIBER_TASK_PRIORITY, &subscriber_task_handle)){
			printf("Failed to create the Subscriber task!\n");
			goto exit_cleanup;
		}

		// Wait for the subscribe operation to complete
		vTaskDelay(pdMS_TO_TICKS(TASK_CREATION_DELAY_MS));

		/* Create the publisher task and cleanup if the operation fails.*/
		if (pdPASS != xTaskCreate(publisher_task, "Publisher task", PUBLISHER_TASK_STACK_SIZE, NULL, PUBLISHER_TASK_PRIORITY, &publisher_task_handle)){
			printf("Failed to create Publisher task!\n");
			goto exit_cleanup;
		}
    }
    // Give the connection semaphore
    xSemaphoreGive(isConnectedSemaphore);

    /* Create the temperature task.*/
	if (pdPASS != xTaskCreate(temperature_task, "Temperature task", TEMPERATURE_TASK_STACK_SIZE, NULL, TEMPERATURE_TASK_PRIORITY, &temperature_task_handle)){
		printf("Failed to create the Temperature task!\n");
	}
	printf("Created Temperature Task\n");

	/* Create the display task.*/
	if (pdPASS != xTaskCreate(display_task, "Display task", DISPLAY_TASK_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY, &display_task_handle)){
		printf("Failed to create the Display task!\n");
	}
	printf("Created Display Task\n");

	/* Create the capsense task.*/
	if (pdPASS != xTaskCreate(capsense_task, "Capsense task", CAPSENSE_TASK_STACK_SIZE, NULL, CAPSENSE_TASK_PRIORITY, &capsense_task_handle)){
		printf("Failed to create the Capsense task!\n");
	}
	printf("Created Capsense Task\n");

    while (true){
    	if(isConnected){

			/* Wait for results of MQTT operations from other tasks and callbacks. */
			if(pdTRUE == xQueueReceive(mqtt_task_q, &mqtt_status, portMAX_DELAY)){
				switch(mqtt_status){
					case HANDLE_MQTT_PUBLISH_FAILURE:
					case HANDLE_MQTT_SUBSCRIBE_FAILURE:
					case HANDLE_DISCONNECTION:
					{
						/* Delete the subscriber and publisher tasks and go to the
						 * cleanup label as MQTT subscribe/publish has failed.
						 */
						cy_mqtt_disconnect(mqtt_connection);
						printf("Terminating the publisher and subscriber tasks...\n\n");
						if (subscriber_task_handle != NULL){
							vTaskDelete(subscriber_task_handle);
						}
						if (publisher_task_handle != NULL){
							vTaskDelete(publisher_task_handle);
						}
						isConnected = false;
						goto exit_cleanup;
						break;
					}
					default:
						break;
				}
			}
		}
    	vTaskDelay(100);
    }

    /* Cleanup section: Delete subscriber and publisher tasks and perform
	 * cleanup for various operations based on the status_flag.
	 */
	exit_cleanup:
	cleanup();
	printf("\nCleanup Done\nTerminating the MQTT task...\n\n");
	vTaskDelete(NULL);
}

/******************************************************************************
 * Function Name: wifi_connect
 ******************************************************************************
 * Summary:
 *  Function that initializes the Wi-Fi Connection Manager and then connects 
 *  to the Wi-Fi Access Point using the specified SSID and PASSWORD.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int : EXIT_SUCCESS on successful connection with a Wi-Fi Access Point,
 *        else EXIT_FAILURE.
 *
 ******************************************************************************/
static cy_rslt_t wifi_connect(void){
	cy_rslt_t result = CY_RSLT_SUCCESS;
	cy_wcm_connect_params_t connect_param;
	cy_wcm_ip_address_t ip_address;

	/* Check if Wi-Fi connection is already established. */
	if (cy_wcm_is_connected_to_ap() == 0){
		/* Configure the connection parameters for the Wi-Fi interface. */
		memset(&connect_param, 0, sizeof(cy_wcm_connect_params_t));
		memcpy(connect_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
		memcpy(connect_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
		connect_param.ap_credentials.security = WIFI_SECURITY;

		printf("\nConnecting to Wi-Fi AP '%s'\n\n", connect_param.ap_credentials.SSID);

		/* Connect to the Wi-Fi AP. */
		for (uint32_t retry_count = 0; retry_count < MAX_WIFI_CONN_RETRIES; retry_count++){
			result = cy_wcm_connect_ap(&connect_param, &ip_address);

			if (result == CY_RSLT_SUCCESS){
				printf("\nSuccessfully connected to Wi-Fi network '%s'.\n", connect_param.ap_credentials.SSID);

				/* Set the appropriate bit in the status_flag to denote
				 * successful Wi-Fi connection, print the assigned IP address.
				 */
				status_flag |= WIFI_CONNECTED;
				if (ip_address.version == CY_WCM_IP_VER_V4){
                    cy_nw_ntoa((cy_nw_ip_address_t *)&(ip_address), ipAddrString);
					printf("IPv4 Address Assigned: %s\n\n", ipAddrString);
				}
				else if (ip_address.version == CY_WCM_IP_VER_V6){
                    cy_nw_ntoa_ipv6((cy_nw_ip_address_t *)&(ip_address), ipAddrStringV6);
					printf("IPv6 Address Assigned: %s\n\n", ipAddrStringV6);
				}
				return result;
			}

			printf("Connection to Wi-Fi network failed with error code 0x%0X. Retrying in %d ms. Retries left: %d\n",
			(int)result, WIFI_CONN_RETRY_INTERVAL_MS, (int)(MAX_WIFI_CONN_RETRIES - retry_count - 1));
			vTaskDelay(pdMS_TO_TICKS(WIFI_CONN_RETRY_INTERVAL_MS));
		}

		printf("\nExceeded maximum Wi-Fi connection attempts!\n");
		printf("Wi-Fi connection failed after retrying for %d mins\n\n",
		(int)(WIFI_CONN_RETRY_INTERVAL_MS * MAX_WIFI_CONN_RETRIES) / 60000u);
	}
	return result;
}

/******************************************************************************
 * Function Name: mqtt_init
 ******************************************************************************
 * Summary:
 *  Function that initializes the MQTT library and creates an instance for the
 *  MQTT client. The network buffer needed by the MQTT library for MQTT send
 *  send and receive operations is also allocated by this function.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  cy_rslt_t : CY_RSLT_SUCCESS on a successful initialization, else an error
 *              code indicating the failure.
 *
 ******************************************************************************/
static cy_rslt_t mqtt_init(void){
    /* Variable to indicate status of various operations. */
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize the MQTT library. */
    result = cy_mqtt_init();
    CHECK_RESULT(result, LIBS_INITIALIZED, "MQTT library initialization failed!\n\n");

    /* Allocate buffer for MQTT send and receive operations. */
    mqtt_network_buffer = (uint8_t *) pvPortMalloc(sizeof(uint8_t) * MQTT_NETWORK_BUFFER_SIZE);
    if(mqtt_network_buffer == NULL){
        result = ~CY_RSLT_SUCCESS;
    }
    CHECK_RESULT(result, BUFFER_INITIALIZED, "Network Buffer allocation failed!\n\n");

    /* Create the MQTT client instance. */
    result = cy_mqtt_create(mqtt_network_buffer, MQTT_NETWORK_BUFFER_SIZE,
                            security_info, &broker_info,
                            (cy_mqtt_callback_t)mqtt_event_callback, NULL,
                            &mqtt_connection);
    CHECK_RESULT(result, MQTT_INSTANCE_CREATED, "MQTT instance creation failed!\n\n");
    printf("MQTT library initialization successful.\n\n");

    return result;
}

/******************************************************************************
 * Function Name: mqtt_connect
 ******************************************************************************
 * Summary:
 *  Function that initiates MQTT connect operation. The connection is retried
 *  a maximum of 'MAX_MQTT_CONN_RETRIES' times with interval of
 *  'MQTT_CONN_RETRY_INTERVAL_MS' milliseconds.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  cy_rslt_t : CY_RSLT_SUCCESS upon a successful MQTT connection, else an
 *              error code indicating the failure.
 *
 ******************************************************************************/
static cy_rslt_t mqtt_connect(void){

    /* Variable to indicate status of various operations. */
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* MQTT client identifier string. */
    char mqtt_client_identifier[(MQTT_CLIENT_IDENTIFIER_MAX_LEN + 1)] = MQTT_CLIENT_IDENTIFIER;

    /* Configure the user credentials as a part of MQTT Connect packet */
    if (strlen(MQTT_USERNAME) > 0){
        connection_info.username = MQTT_USERNAME;
        connection_info.password = MQTT_PASSWORD;
        connection_info.username_len = sizeof(MQTT_USERNAME) - 1;
        connection_info.password_len = sizeof(MQTT_PASSWORD) - 1;
    }

    /* Generate a unique client identifier with 'MQTT_CLIENT_IDENTIFIER' string
     * as a prefix.
     */
    result = mqtt_get_unique_client_identifier(mqtt_client_identifier);
    CHECK_RESULT(result, 0, "Failed to generate unique client identifier for the MQTT client!\n");

    /* Set the client identifier buffer and length. */
    connection_info.client_id = mqtt_client_identifier;
    connection_info.client_id_len = strlen(mqtt_client_identifier);

    printf("\nMQTT client '%.*s' connecting to MQTT broker '%.*s'...\n\n",
           connection_info.client_id_len,
           connection_info.client_id,
           broker_info.hostname_len,
           broker_info.hostname);

    for (uint32_t retry_count = 0; retry_count < MAX_MQTT_CONN_RETRIES; retry_count++){
        if (cy_wcm_is_connected_to_ap() == 0){
            printf("Unexpectedly disconnected from Wi-Fi network! Initiating Wi-Fi reconnection...\n");
            status_flag &= ~(WIFI_CONNECTED);

            /* Initiate Wi-Fi reconnection. */
            result = wifi_connect();
            if (CY_RSLT_SUCCESS != result)
            {
                return result;
            }
        }

        /* Establish the MQTT connection. */
        result = cy_mqtt_connect(mqtt_connection, &connection_info);

        if (result == CY_RSLT_SUCCESS){
            printf("MQTT connection successful.\n\n");

            /* Set the appropriate bit in the status_flag to denote successful
             * MQTT connection, and return the result to the calling function.
             */
            status_flag |= MQTT_CONNECTION_SUCCESS;
            return result;
        }

        printf("MQTT connection failed with error code 0x%0X. Retrying in %d ms. Retries left: %d\n",
               (int)result, MQTT_CONN_RETRY_INTERVAL_MS, (int)(MAX_MQTT_CONN_RETRIES - retry_count - 1));
        vTaskDelay(pdMS_TO_TICKS(MQTT_CONN_RETRY_INTERVAL_MS));
    }

    printf("\nExceeded maximum MQTT connection attempts\n");
    printf("MQTT connection failed after retrying for %d mins\n\n",
           (int)(MQTT_CONN_RETRY_INTERVAL_MS * MAX_MQTT_CONN_RETRIES) / 60000u);
    return result;
}

/******************************************************************************
 * Function Name: mqtt_event_callback
 ******************************************************************************
 * Summary:
 *  Callback invoked by the MQTT library for events like MQTT disconnection,
 *  incoming MQTT subscription messages from the MQTT broker.
 *    1. In case of MQTT disconnection, the MQTT client task is communicated
 *       about the disconnection using a message queue.
 *    2. When an MQTT subscription message is received, the subscriber callback
 *       function implemented in subscriber_task.c is invoked to handle the
 *       incoming MQTT message.
 *
 * Parameters:
 *  cy_mqtt_t mqtt_handle : MQTT handle corresponding to the MQTT event (unused)
 *  cy_mqtt_event_t event : MQTT event information
 *  void *user_data : User data pointer passed during cy_mqtt_create() (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void mqtt_event_callback(cy_mqtt_t mqtt_handle, cy_mqtt_event_t event, void *user_data){
    cy_mqtt_publish_info_t *received_msg;
    mqtt_task_cmd_t mqtt_task_cmd;

    (void) mqtt_handle;
    (void) user_data;

    switch(event.type){
        case CY_MQTT_EVENT_TYPE_DISCONNECT:
        {
            /* Clear the status flag bit to indicate MQTT disconnection. */
            status_flag &= ~(MQTT_CONNECTION_SUCCESS);

            /* MQTT connection with the MQTT broker is broken as the client
             * is unable to communicate with the broker. Set the appropriate
             * command to be sent to the MQTT task.
             */
            printf("\nUnexpectedly disconnected from MQTT broker!\n");
            mqtt_task_cmd = HANDLE_DISCONNECTION;

            /* Send the message to the MQTT client task to handle the
             * disconnection.
             */
            xQueueSend(mqtt_task_q, &mqtt_task_cmd, portMAX_DELAY);
            break;
        }

        case CY_MQTT_EVENT_TYPE_SUBSCRIPTION_MESSAGE_RECEIVE:
        {
            status_flag |= MQTT_MSG_RECEIVED;

            /* Incoming MQTT message has been received. Send this message to
             * the subscriber callback function to handle it.
             */
            received_msg = &(event.data.pub_msg.received_message);
            mqtt_subscription_callback(received_msg);
            break;
        }
        default :
        {
            /* Unknown MQTT event */
            printf("\nUnknown Event received from MQTT callback!\n");
            break;
        }
    }
}

/******************************************************************************
 * Function Name: mqtt_get_unique_client_identifier
 ******************************************************************************
 * Summary:
 *  Function that generates unique client identifier for the MQTT client by
 *  appending a timestamp to a common prefix 'MQTT_CLIENT_IDENTIFIER'.
 *
 * Parameters:
 *  char *mqtt_client_identifier : Pointer to the string that stores the 
 *                                 generated unique identifier
 *
 * Return:
 *  cy_rslt_t : CY_RSLT_SUCCESS on successful generation of the client
 *              identifier, else a non-zero value indicating failure.
 *
 ******************************************************************************/
static cy_rslt_t mqtt_get_unique_client_identifier(char *mqtt_client_identifier){
    cy_rslt_t status = CY_RSLT_SUCCESS;

    /* Check for errors from snprintf. */
    if (0 > snprintf(mqtt_client_identifier,
                     (MQTT_CLIENT_IDENTIFIER_MAX_LEN + 1),
                     MQTT_CLIENT_IDENTIFIER "%lu",
                     (long unsigned int)Clock_GetTimeMs()))
    {
        status = ~CY_RSLT_SUCCESS;
    }

    return status;
}

/******************************************************************************
 * Function Name: cleanup
 ******************************************************************************
 * Summary:
 *  Function that invokes the deinit and cleanup functions for various
 *  operations based on the status_flag.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void cleanup(void){
    /* Disconnect the MQTT connection if it was established. */
    if (status_flag & MQTT_CONNECTION_SUCCESS)
    {
        printf("Disconnecting from the MQTT Broker...\n");
        cy_mqtt_disconnect(mqtt_connection);
    }
    /* Delete the MQTT instance if it was created. */
    if (status_flag & MQTT_INSTANCE_CREATED)
    {
        cy_mqtt_delete(mqtt_connection);
    }
    /* Deallocate the network buffer. */
    if (status_flag & BUFFER_INITIALIZED)
    {
        vPortFree((void *) mqtt_network_buffer);
    }
    /* Deinit the MQTT library. */
    if (status_flag & LIBS_INITIALIZED)
    {
        cy_mqtt_deinit();
    }
    /* Disconnect from Wi-Fi AP. */
    if (status_flag & WIFI_CONNECTED)
    {
        if (cy_wcm_disconnect_ap() == CY_RSLT_SUCCESS)
        {
            printf("Disconnected from the Wi-Fi AP!\n");
        }
    }
    /* De-initialize the Wi-Fi Connection Manager. */
    if (status_flag & WCM_INITIALIZED)
    {
        cy_wcm_deinit();
    }
}

/* [] END OF FILE */
