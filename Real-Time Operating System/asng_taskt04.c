
/* XDC module Headers */
#include <xdc/std.h>
#include <stddef.h>                     //Added in
#include <xdc/runtime/System.h>

/* BIOS module Headers */
#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/drivers/Timer.h>                     //Added in
#include <ti/drivers/GPIO.h>                     //Added in
#include <ti/drivers/Board.h>
#include <ti/drivers/ADC.h>                     //Added In
#include <ti/drivers/PWM.h>                     //Added In
#include <ti/drivers/UART.h>                    //Added In

/* Board Header file */
#include "ti_drivers_config.h"                     //Added in

#define TASKSTACKSIZE   512

/* Callback used for toggling the LED. */
void timerCallback(Timer_Handle myHandle, int_fast16_t status);             //Added in

//Semaphore Variables
Semaphore_Struct semStruct; //Task1
Semaphore_Handle semHandle;

Semaphore_Struct semStruct2;    //Task2
Semaphore_Handle semHandle2;

Semaphore_Struct semStruct3;    //Task3
Semaphore_Handle semHandle3;

//Global Variables
volatile uint32_t tickCount = 0;
uint16_t adc_value;
int_fast16_t res;
char uart_val[14] = {0};
volatile int uart_length = 0;
uint32_t   dutyCycle;

//HeartBeat Idle Function
Void heartBeatFxn(UArg arg0, UArg arg1)   //HeartBeat Idle
{
    while (1) {
        Task_sleep((UInt)arg0);
        GPIO_toggle(CONFIG_GPIO_LED_0);   //Red LED
    }
}

//Task 1
void adcRead()
{
    ADC_Handle adc;
    ADC_Params adc_params;

    adc = ADC_open(CONFIG_ADC_0, &adc_params);

    while(1)
    {
        Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);
        res = ADC_convert(adc, &adc_value);
        if (res == ADC_STATUS_SUCCESS)
        {
            //Nothing
        }
        else
        {
            //Display Error
            System_printf("Read ADC failed\n");
            System_flush();
        }
    }
}

//Task 2
void uartDisplay()
{
    //------------------Configure UART-------------------------------
    UART_Handle uart;
    UART_Params uart_params;

    UART_Params_init(&uart_params);
    uart_params.writeDataMode = UART_DATA_BINARY;
    uart_params.baudRate = 115200;
    uart_params.writeMode = UART_MODE_BLOCKING;
    uart_params.readEcho  = UART_ECHO_OFF;
    uart = UART_open(CONFIG_UART_0, &uart_params);
    //---------------------------------------------------------------

    while(1)
    {
        Semaphore_pend(semHandle2, BIOS_WAIT_FOREVER);
        //System_printf("Task 2 CONFIG_ADC_0 raw result: %d\n", adc_value);
        //System_flush();
        uart_length = System_sprintf(uart_val, "Task 2: ADC Value: %d\r\n", adc_value);
        UART_write(uart, uart_val,  uart_length++);
    }
}

//Task 3
void switchDisplay()
{
    //------------------Configure PWM--------------------------------
    PWM_Handle pwm;
    PWM_Params pwm_params;

    PWM_Params_init(&pwm_params);
    pwm_params.idleLevel = PWM_IDLE_LOW;
    pwm_params.periodUnits = PWM_PERIOD_US;
    pwm_params.periodValue = 1000;
    pwm_params.dutyUnits = PWM_DUTY_FRACTION;
    pwm_params.dutyValue = 0;
    pwm = PWM_open(CONFIG_PWM_0, &pwm_params);
    PWM_start(pwm);
    //---------------------------------------------------------------

    while(1)
    {
        Semaphore_pend(semHandle3, BIOS_WAIT_FOREVER);
        dutyCycle = ((uint64_t) PWM_DUTY_FRACTION_MAX *(uint32_t)adc_value/4000);   //Scale Adc Value
        PWM_setDuty(pwm, dutyCycle);    //Update PWM
    }
}

//TimerCallBack Every 1ms
void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    //Counter to post semaphores to each task
    tickCount++;
    if(tickCount == 5)  //Task 1 (5 ms)
    {
        Semaphore_post(semHandle);
    }
    if(tickCount == 10) //Task 2 (10 ms)
    {
        Semaphore_post(semHandle2);
    }
    if(tickCount == 15) //Task 3 (15 ms), and reset counter
    {
        Semaphore_post(semHandle3);
        tickCount = 0;
    }
}

/*
 *  ======== main ========
 */
int main()
 {
    /* Call driver init functions */
    Board_init();
    GPIO_init();
    Timer_init();   //Added In
    ADC_init();     //Added In
    PWM_init();     //Added In
    UART_init();    //Added In

    //---------------------Configure Timer------------------------------

    Timer_Handle timer0;
    Timer_Params params;

    Timer_Params_init(&params);
    //params.period = 1000000;                //1 Second
    params.period = 1000;                //1 ms
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;

    timer0 = Timer_open(CONFIG_TIMER_0, &params);

    if (timer0 == NULL) {
        System_printf("Failed to initialized timer\n");
        /* Failed to initialized timer */
        while (1) {}
    }

    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        System_printf("Failed to start timer\n");
        /* Failed to start timer */
        while (1) {}
    }
    //----------------------------------------------------------------


    //---------------------Configure Semaphores-----------------------

    Semaphore_Params semParams;   //Added In

    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&semStruct, 1, &semParams);
    Semaphore_construct(&semStruct2, 1, &semParams);
    Semaphore_construct(&semStruct3, 1, &semParams);

    /* Obtain instance handle */
    semHandle = Semaphore_handle(&semStruct);
    semHandle2 = Semaphore_handle(&semStruct2);
    semHandle3 = Semaphore_handle(&semStruct3);
    //----------------------------------------------------------------

    BIOS_start();    /* Does not return */

    return(0);
}

