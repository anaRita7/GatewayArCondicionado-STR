/*
 Geradores de dados do ambiente
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

void GeradorPresenca() {
    return 0;
}

int GeradorTemperatura(int temperatura_min, int temperatura_max) {
    return 0;
}

void GeradorTensao() {
    return 0;
}

void GeradorParticulas() {
    return 0;
}

void GeradorPresencaGas() {
    return 0;
}

int main(void) {
    
    GeradorPresenca();
    GeradorTemperatura(temperatura_min, temperatura_max);
    GeradorTensao();
    GeradorParticulas();
    GeradorPresencaGas();

    return 0;
}
/*-----------------------------------------------------------*/
