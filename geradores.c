/*
 Geradores de dados do ambiente
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h> // Usada para a função rand()


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

int GeradorTemperatura() {

    int i = 0, temperatura_media[31];
    FILE* arqDadosTemperatura;

    arqDadosTemperatura = fopen("DadosTemperatura.txt", "wt");

    for (int i = 0; i < 31; i++) {
        if (i < 10) {
            temperatura_media[i] = 25;
        }
        else if (i >= 10 && i < 20) {
            temperatura_media[i] = 26;
        }
        else if (i < 31) {
            temperatura_media[i] = 24;
        }

        fprintf(arqDadosTemperatura, "%d\n", temperatura_media[i]);
        printf("%d ", temperatura_media[i]);
    }

    //fclose(arqDadosTemperatura);
    i = 0;
    
    printf(" \nOs dados de temperaura do ambiente foram gerados");
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
    
    //GeradorPresenca();
    GeradorTemperatura();
    //GeradorTensao();
    //GeradorParticulas();
    //GeradorPresencaGas();

    return 0;
}
/*-----------------------------------------------------------*/
