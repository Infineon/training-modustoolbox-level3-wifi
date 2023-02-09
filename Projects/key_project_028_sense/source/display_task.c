/******************************************************************************
* File Name: display_task.c
*
* Description: This task manages the display
*
******************************************************************************/
// PSoC MCU Headers
#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cy_retarget_io.h"

// Task Headers
#include "display_task.h"
#include "mqtt_task.h"

// Middleware Headers
#include "semphr.h"
#include "mtb_ssd1306.h"
#include "GUI.h"


/* Positions on the OLED screen */
#define OLED_LABEL     (0)
#define OLED_VALUE     (100)
#define OLED_MODE      (50)
#define OLED_ROW_ONE   (0)
#define OLED_ROW_TWO   (15)
#define OLED_ROW_THREE (30)
#define OLED_ROW_FOUR  (45)

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void displayUpdate();

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Task handle for this task. */
TaskHandle_t display_task_handle;

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
* Function Name: display_task
********************************************************************************
* Summary:
*  Initializes the display, then waits for notifications to update the display
*
* Return:
*  void
*
*******************************************************************************/
void display_task(void *pvParameters){
	/* To avoid compiler warnings */
	(void)pvParameters;

    cy_rslt_t result;

    /* Initialize OLED display */
    result = mtb_ssd1306_init_i2c(&i2c);

    if (result != CY_RSLT_SUCCESS)
    {
    	printf("Display Init Failed\n");
    	CY_ASSERT(0);
    }

	GUI_Init();
	GUI_Clear();
	GUI_SetFont(&GUI_Font8x10_ASCII);
    GUI_DispStringAt("Actual Temp: ", OLED_LABEL, OLED_ROW_ONE);
	GUI_DispStringAt("Set Temp:    ", OLED_LABEL, OLED_ROW_TWO);
	GUI_DispStringAt("Mode: ", OLED_LABEL, OLED_ROW_THREE);

	printf("Display initialized\n");

    for (;;){
    	// Update the display whenever notified
    	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    	displayUpdate();
    }
}

/*******************************************************************************
* Function Name: displayUpdate
********************************************************************************
* Summary:
*  Updates the display with the current values
*
* Return:
*  void
*
*******************************************************************************/
void displayUpdate(){
	printf("Updating display\n");

	// Actual Temp
	xSemaphoreTake(actualTempSemaphore, portMAX_DELAY);
	GUI_GotoXY(OLED_VALUE, OLED_ROW_ONE);
	GUI_DispDecMin(actualTemp);
	xSemaphoreGive(actualTempSemaphore);

	// Set Temp
	xSemaphoreTake(setTempSemaphore, portMAX_DELAY);
	GUI_GotoXY(OLED_VALUE, OLED_ROW_TWO);
	GUI_DispDecMin(setTemp);
	xSemaphoreGive(setTempSemaphore);

	// Mode
	xSemaphoreTake(modeSemaphore, portMAX_DELAY);
	GUI_GotoXY(OLED_MODE, OLED_ROW_THREE);
	GUI_DispString(mode);
	GUI_DispString("   "); // Blank out extra characters if the new string is shorter
	xSemaphoreGive(modeSemaphore);

	// Connected Status
	xSemaphoreTake(isConnectedSemaphore, portMAX_DELAY);
	if(isConnected){
		GUI_DispStringAt("Connected    ", OLED_LABEL, OLED_ROW_FOUR);
	}
	else{
		GUI_DispStringAt("Not Connected", OLED_LABEL, OLED_ROW_FOUR);
	}
	xSemaphoreGive(isConnectedSemaphore);

}

/* [] END OF FILE */
