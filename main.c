/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting is used to select between the two.
 * The simply blinky demo is implemented and described in main_blinky.c.  The
 * more comprehensive test and demo application is implemented and described in
 * main_full.c.
 *
 * This file implements the code that is not demo specific, including the
 * hardware setup and FreeRTOS hook functions.
 *
 *******************************************************************************
 * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * https://www.FreeRTOS.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 *
 *
 *******************************************************************************
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* Visual studio intrinsics used so the __debugbreak() function is available
 * should an assert get hit. */
#include <intrin.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+Trace includes. */
#include "trcRecorder.h"

/* This project provides two demo applications.  A simple blinky style demo
 * application, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting is used to select between the two.
 *
 * If mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is 1 then the blinky demo will be built.
 * The blinky demo is implemented and described in main_blinky.c.
 *
 * If mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is not 1 then the comprehensive test and
 * demo application will be built.  The comprehensive test and demo application is
 * implemented and described in main_full.c. */
#define mainCREATE_SIMPLE_BLINKY_DEMO_ONLY    1

/* This demo uses heap_5.c, and these constants define the sizes of the regions
 * that make up the total heap.  heap_5 is only used for test and example purposes
 * as this demo could easily create one large heap region instead of multiple
 * smaller heap regions - in which case heap_4.c would be the more appropriate
 * choice.  See http://www.freertos.org/a00111.html for an explanation. */
#define mainREGION_1_SIZE                     8201
#define mainREGION_2_SIZE                     23905
#define mainREGION_3_SIZE                     16807

/* This demo allows for users to perform actions with the keyboard. */
#define mainNO_KEY_PRESS_VALUE                -1
#define mainOUTPUT_TRACE_KEY                  't'
#define mainINTERRUPT_NUMBER_KEYBOARD         3

/* This demo allows to save a trace file. */
#define mainTRACE_FILE_NAME                   "Trace.dump"

/*-----------------------------------------------------------*/

/*
 * main_blinky() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1.
 * main_full() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 0.
 */
extern void main_blinky( void );
extern void main_full( void );

/*
 * Only the comprehensive demo uses application hook (callback) functions.  See
 * https://www.FreeRTOS.org/a00016.html for more information.
 */
extern void vFullDemoTickHookFunction( void );
extern void vFullDemoIdleFunction( void );

/*
 * This demo uses heap_5.c, so start by defining some heap regions.  It is not
 * necessary for this demo to use heap_5, as it could define one large heap
 * region.  Heap_5 is only used for test and example purposes.  See
 * https://www.FreeRTOS.org/a00111.html for an explanation.
 */
static void prvInitialiseHeap( void );

/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask,
                                    char * pcTaskName );
void vApplicationTickHook( void );
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize );

/*
 * Writes trace data to a disk file when the trace recording is stopped.
 * This function will simply overwrite any trace files that already exist.
 */
static void prvSaveTraceFile( void );

/*
 * Windows thread function to capture keyboard input from outside of the
 * FreeRTOS simulator. This thread passes data safely into the FreeRTOS
 * simulator using a stream buffer.
 */
static DWORD WINAPI prvWindowsKeyboardInputThread( void * pvParam );

/*
 * Interrupt handler for when keyboard input is received.
 */
static uint32_t prvKeyboardInterruptHandler( void );

/*
 * Keyboard interrupt handler for the blinky demo. 
 */
extern void vBlinkyKeyboardInterruptHandler( int xKeyPressed );

/*-----------------------------------------------------------*/

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It is
 * declared here, as a global, so it can be checked by a test that is implemented
 * in a different file. */
StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];


/* Thread handle for the keyboard input Windows thread. */
static HANDLE xWindowsKeyboardInputThreadHandle = NULL;

/* This stores the last key pressed that has not been handled.
 * Keyboard input is retrieved by the prvWindowsKeyboardInputThread
 * Windows thread and stored here. This is then read by the idle
 * task and handled appropriately. */
static int xKeyPressed = mainNO_KEY_PRESS_VALUE;

/*-----------------------------------------------------------*/

//#include "geradores.c"

void DetectorPresencaTask() {
    while (1) {
        printf("Sensoriando Presenca...\n");
        // Tempo de execucao = 20ms
        // Alteracao de uma variavel que indica o numero de pessoas no ambiente

        vTaskDelay(150);
    }
}


void SensorTemperaturaTask() {

    int temperatura_atual[30], maxArray = 30;
    FILE* arqDadosTemperatura;

    while (1) {
        
        printf("Medindo a temperatura...\n");
        // tempo de execucao = 20ms
        // alteracao de uma variavel que indica a temperatura do ambiente
       
        arqDadosTemperatura = fopen("DadosTemperatura.txt", "r");

        if (arqDadosTemperatura == NULL) {
            printf("arquivo dos dados da temperatura n�o pode ser aberto... \n");
        }

        /*for (int i = 0; i < maxArray; i++) {
            fscanf(arqDadosTemperatura, "%d ", &temperatura_atual[i]);
            printf("A temperatura atual medida eh %d\n", temperatura_atual[i]);
        }*/

        //fclose(arqDadosTemperatura);

        vTaskDelay(250);
    }
}

void MedidorTensaoTask() {
   while (1) {
        printf("Medindo a Tensao da Ventoinha e do Compressor de Ar...\n");
        // Tempo de execucao = 20ms
        // Alteracao de duas variaveis que s�o: 1 - Tensao Nula e 0 - Tensao diferente de zero
        vTaskDelay(2000);
    }
}

void SensorParticulasTask() {

    int const tamanhoAmbiente = 90; // m^3
    int const minParticula = 0.05; //  mg/m^3
    int maxArray = 30, quantidadeParticula[30];
    FILE* arqDadosParticula;

    while (1) {
        printf("Sensoriando as particulas na saida de ar...\n");
        // Tempo de execucao = 20ms
        // Alteracao de variavel que ser�: 1 - Muitas particulas e 0 - N de particulas normal

        /*for (int i = 0; i < maxArray; i++) {
            fscanf(arqDadosParticula, "%d ", quantidadeParticula[i]);
            quantidadeParticula[i] = quantidadeParticula[i] / tamanhoAmbiente;

             // 0.05mg/m^3 � a qunatidade de part�culas de poeira que o ser humano pode respirar
            if (quantidadeParticula[i] > minParticula) {
                return 1;
            }
            else {
                return 0;
            }
        }*/

        vTaskDelay(2000);
    }
}

void SensorPresencaGasRefrigeranteTask() {
    while (1) {
        printf("Sensoriando presenca de gas refrigerante...\n");
        // Tempo de execucao = 20ms
        // Alteracao de variavel que ser�: 1 - Presenca de gas e 0 - Nao Presenca de Gas
        vTaskDelay(2000);
    }
}

void LigarArCondicionadoTask() {
    printf("Ligando o ar Condicionado...\n");
    // Tempo de execucao = 40ms
    // Deadline = 500ms
    // Acionado quando variavel de numero de pessoas variar de 0 para 1
}

void ControlarTemperaturaTask() {
    printf("Mudando Temperatura do Ar Condicionado...\n");
    // Tempo de execucao = 30ms
    // Deadline = 250ms
    // Acionado quando variavel de numero de pessoas ou a temperatura do ambiente variarem
}

void ControlarFluxoArTask() {
    printf("Mudando Direcao do Fluxo de Ar...\n");
    // Tempo de execucao = 30ms
    // Deadline = 250ms
    // Acionado quando variavel de numero de pessoas variar
}

void DesligarArCondicionadoTask() {
    printf("Desligando o ar Condicionado...\n");
    // Tempo de execucao = 40ms
    // Deadline = 500ms
    // Acionado quando variavel de numero de pessoas variar para 0
}

void NotificarDispositivoMovelTask() {
    printf("Desligando o ar Condicionado...\n");
    // Tempo de execucao = 15ms
    // Deadline = 250ms
    // Acionado quando uma das tarefas T3, T4 ou T5 tiver retorno = 1
}

int main(void)
{
    /* This demo uses heap_5.c, so start by defining some heap regions.  heap_5
     * is only used for test and example reasons.  Heap_4 is more appropriate.  See
     * http://www.freertos.org/a00111.html for an explanation. */
    prvInitialiseHeap();

    /* Initialise the trace recorder.  Use of the trace recorder is optional.
     * See http://www.FreeRTOS.org/trace for more information. */

    vTraceEnable(TRC_START);
    xTaskHandle HT1;
    xTaskHandle HT2;
    xTaskHandle HT3;
    xTaskHandle HT4;
    xTaskHandle HT5;

    /* create task */
    xTaskCreate(DetectorPresencaTask, (signed char*)"DetectorPresencaTask", configMINIMAL_STACK_SIZE, (void*)NULL, 5, &HT1);
    xTaskCreate(SensorTemperaturaTask, (signed char*)"SensorTemperaturaTask", configMINIMAL_STACK_SIZE, (void*)NULL, 4, &HT2);
    xTaskCreate(MedidorTensaoTask, (signed char*)"MedidorTensaoTask", configMINIMAL_STACK_SIZE, (void*)NULL, 3, &HT3);
    xTaskCreate(SensorParticulasTask, (signed char*)"SensorParticulasTask", configMINIMAL_STACK_SIZE, (void*)NULL, 2, &HT4);
    xTaskCreate(SensorPresencaGasRefrigeranteTask, (signed char*)"SensorPresencaGasRefrigeranteTask", configMINIMAL_STACK_SIZE, (void*)NULL, 1, &HT5);

    // Ver quest�o do Deferrable Server para tarefas aperi�dicas

    /* start the scheduler */
    vTaskStartScheduler();

    return 0;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the	heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled( __LINE__, __FILE__ );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */

    #if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY != 1 )
        {
            /* Call the idle task processing used by the full demo.  The simple
             * blinky demo does not use the idle task hook. */
            vFullDemoIdleFunction();
        }
    #endif
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
                                    char * pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
     * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     * function is called if a stack overflow is detected.  This function is
     * provided as an example only as stack overflow checking does not function
     * when running the FreeRTOS Windows port. */
    vAssertCalled( __LINE__, __FILE__ );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    * added here, but the tick hook is called from an interrupt context, so
    * code must not attempt to block, and only the interrupt safe FreeRTOS API
    * functions can be used (those that end in FromISR()). */

    #if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY != 1 )
        {
            vFullDemoTickHookFunction();
        }
    #endif /* mainCREATE_SIMPLE_BLINKY_DEMO_ONLY */
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
     * execute	(sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled( unsigned long ulLine,
                    const char * const pcFileName )
{
    static BaseType_t xPrinted = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
     * http://www.freertos.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    ( void ) ulLine;
    ( void ) pcFileName;

    taskENTER_CRITICAL();
    {
        printf("ASSERT! Line %ld, file %s, GetLastError() %ld\r\n", ulLine, pcFileName, GetLastError());

        /* Stop the trace recording and save the trace. */
        ( void ) xTraceDisable();
        prvSaveTraceFile();

        /* Cause debugger break point if being debugged. */
        __debugbreak();

        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
            __asm {
                NOP
            };
            __asm {
                NOP
            };
        }

        /* Re-enable the trace recording. */
        ( void ) xTraceEnable( TRC_START );
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

static void prvSaveTraceFile( void )
{
    FILE * pxOutputFile;

    fopen_s( &pxOutputFile, mainTRACE_FILE_NAME, "wb" );

    if( pxOutputFile != NULL )
    {
        fwrite( RecorderDataPtr, sizeof( RecorderDataType ), 1, pxOutputFile );
        fclose( pxOutputFile );
        printf( "\r\nTrace output saved to %s\r\n\r\n", mainTRACE_FILE_NAME );
    }
    else
    {
        printf( "\r\nFailed to create trace dump file\r\n\r\n" );
    }
}
/*-----------------------------------------------------------*/

static void prvInitialiseHeap( void )
{
/* The Windows demo could create one large heap region, in which case it would
 * be appropriate to use heap_4.  However, purely for demonstration purposes,
 * heap_5 is used instead, so start by defining some heap regions.  No
 * initialisation is required when any other heap implementation is used.  See
 * http://www.freertos.org/a00111.html for more information.
 *
 * The xHeapRegions structure requires the regions to be defined in start address
 * order, so this just creates one big array, then populates the structure with
 * offsets into the array - with gaps in between and messy alignment just for test
 * purposes. */
    static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
    volatile uint32_t ulAdditionalOffset = 19; /* Just to prevent 'condition is always true' warnings in configASSERT(). */
    const HeapRegion_t xHeapRegions[] =
    {
        /* Start address with dummy offsets						Size */
        { ucHeap + 1,                                          mainREGION_1_SIZE },
        { ucHeap + 15 + mainREGION_1_SIZE,                     mainREGION_2_SIZE },
        { ucHeap + 19 + mainREGION_1_SIZE + mainREGION_2_SIZE, mainREGION_3_SIZE },
        { NULL,                                                0                 }
    };

    /* Sanity check that the sizes and offsets defined actually fit into the
     * array. */
    configASSERT( ( ulAdditionalOffset + mainREGION_1_SIZE + mainREGION_2_SIZE + mainREGION_3_SIZE ) < configTOTAL_HEAP_SIZE );

    /* Prevent compiler warnings when configASSERT() is not defined. */
    ( void ) ulAdditionalOffset;

    vPortDefineHeapRegions( xHeapRegions );
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/

/*
 * Interrupt handler for when keyboard input is received.
 */
static uint32_t prvKeyboardInterruptHandler(void)
{
    /* Handle keyboard input. */
    switch (xKeyPressed)
    {
    case mainNO_KEY_PRESS_VALUE:
        break;
    case mainOUTPUT_TRACE_KEY:
        /* Saving the trace file requires Windows system calls, so enter a critical
           section to prevent deadlock or errors resulting from calling a Windows
           system call from within the FreeRTOS simulator. */
        portENTER_CRITICAL();
        {
            ( void ) xTraceDisable();
            prvSaveTraceFile();
            ( void ) xTraceEnable(TRC_START);
        }
        portEXIT_CRITICAL();
        break;
    default:
        #if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1 )
            {
                /* Call the keyboard interrupt handler for the blinky demo. */
                vBlinkyKeyboardInterruptHandler( xKeyPressed );
            }
        #endif
    break;
    }

    /* This interrupt does not require a context switch so return pdFALSE */
    return pdFALSE;
}

/*-----------------------------------------------------------*/
/*
 * Windows thread function to capture keyboard input from outside of the
 * FreeRTOS simulator. This thread passes data into the simulator using
 * an integer.
 */
static DWORD WINAPI prvWindowsKeyboardInputThread( void * pvParam )
{
    ( void ) pvParam;

    for ( ; ; )
    {
        /* Block on acquiring a key press. */
        xKeyPressed = _getch();
        
        /* Notify FreeRTOS simulator that there is a keyboard interrupt.
         * This will trigger prvKeyboardInterruptHandler.
         */
        vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER_KEYBOARD );
    }

    /* Should not get here so return negative exit status. */
    return -1;
}

/*-----------------------------------------------------------*/

/* The below code is used by the trace recorder for timing. */
static uint32_t ulEntryTime = 0;

void vTraceTimerReset( void )
{
    ulEntryTime = xTaskGetTickCount();
}

uint32_t uiTraceTimerGetFrequency( void )
{
    return configTICK_RATE_HZ;
}

uint32_t uiTraceTimerGetValue( void )
{
    return( xTaskGetTickCount() - ulEntryTime );
}
