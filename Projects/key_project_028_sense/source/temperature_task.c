/******************************************************************************
* File Name: temperature_task.c
*
* Description: This example measures an external voltage using CSDADC and
*              displays the voltage along with the equivalent digital count
*              via the UART terminal.
*
******************************************************************************/
//PSoC MCU Headers
#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cy_retarget_io.h"

// Task Headers
#include "temperature_task.h"
#include "mqtt_task.h"

// Middleware Headers
#include "semphr.h"
#include "xensiv_dps3xx_mtb.h"

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Task handle for this task. */
TaskHandle_t temperature_task_handle;

/* Temperature is read from the pressure sensor */
xensiv_dps3xx_t pressure_sensor;

// Defined in main.c
extern cyhal_i2c_t i2c;
extern int actualTemp, setTemp;
extern char * mode;
extern bool isConnected;
extern SemaphoreHandle_t actualTempSemaphore;
extern SemaphoreHandle_t setTempSemaphore;
extern SemaphoreHandle_t modeSemaphore;
extern SemaphoreHandle_t isConnectedSemaphore;

/*******************************************************************************
* Function Name: temperature_task
********************************************************************************
* Summary:
*  This function performs initial setup of device,
*  configures CSD ADC block, converts the input voltage in equivalent digital
*  value, converts to a temperature, and sends that value to the publish task.
*
* Return:
*  void
*
*******************************************************************************/
void temperature_task(void *pvParameters){

	/* To avoid compiler warnings */
	(void)pvParameters;

	// status variable
	cy_rslt_t result;

	// Delay
	const TickType_t sampleDelay = pdMS_TO_TICKS(100);

    // Variables to hold pressure and temperature values read from the sensor
    float pressure, temperature;

	// Variable to update temp as an integer value
    int newActualTemp;

    /* Initialize pressure sensor */
    result = xensiv_dps3xx_mtb_init_i2c(&pressure_sensor, &i2c, XENSIV_DPS3XX_I2C_ADDR_DEFAULT);

    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    for (;;){
    	/* Read the temperature value from the pressure sensor */
         xensiv_dps3xx_read(&pressure_sensor, &pressure, &temperature);

         /* Convert from C to F and round off temperature to an integer value */
         newActualTemp = (int)( (temperature*(9.0/5.0) + 32) + 0.5);

		// If the temp changed, record it, then notify the publisher task so it can publish the new value
		xSemaphoreTake(actualTempSemaphore, portMAX_DELAY);
		if(newActualTemp != actualTemp){
			actualTemp = newActualTemp;
			// Notify the display task and publisher task that the actualTemp has changed
			xTaskNotifyGive(display_task_handle);
			xSemaphoreTake(isConnectedSemaphore, portMAX_DELAY);
			if(isConnected){
				xTaskNotify(publisher_task_handle, ACTUALTEMP, eSetValueWithoutOverwrite);
			}
			xSemaphoreGive(isConnectedSemaphore);
		}

		// Determine what mode the thermostat should be in
		xSemaphoreTake(modeSemaphore, portMAX_DELAY);
		xSemaphoreTake(setTempSemaphore, portMAX_DELAY);
		// Var to store the previous mode
		char *modePrev = mode;

		// Cooling
		if(actualTemp > setTemp){
			mode = MODE_COOL;
			cyhal_gpio_write(CYBSP_LED_RGB_RED, CYBSP_LED_STATE_OFF);
			cyhal_gpio_write(CYBSP_LED_RGB_BLUE, CYBSP_LED_STATE_ON);
		}
		// Heating
		else if(actualTemp < setTemp){
			mode = MODE_HEAT;
			cyhal_gpio_write(CYBSP_LED_RGB_RED, CYBSP_LED_STATE_ON);
			cyhal_gpio_write(CYBSP_LED_RGB_BLUE, CYBSP_LED_STATE_OFF);
		}
		// Idle
		else{
			mode = MODE_IDLE;
			cyhal_gpio_write(CYBSP_LED_RGB_RED, CYBSP_LED_STATE_OFF);
			cyhal_gpio_write(CYBSP_LED_RGB_BLUE, CYBSP_LED_STATE_OFF);
		}
		xSemaphoreGive(actualTempSemaphore);
		xSemaphoreGive(setTempSemaphore);
		// If the mode changed notify the display task to update the display and notify the publisher task to update the cloud
		if(modePrev != mode){
			xTaskNotifyGive(display_task_handle);
			xSemaphoreTake(isConnectedSemaphore, portMAX_DELAY);
			if(isConnected){
				xTaskNotify(publisher_task_handle, MODE, eSetValueWithoutOverwrite);
			}
			xSemaphoreGive(isConnectedSemaphore);
		}
		xSemaphoreGive(modeSemaphore);


		/* Give delay between samples */
		vTaskDelay(sampleDelay);
    }
}

/* [] END OF FILE */
