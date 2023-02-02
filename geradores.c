/*
 Geradores de dados do ambiente
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h> // Usada para a função rand()
#include <stddef.h>

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

void GeradorTemperatura() {

    int i = 0, MaxArray = 31, mediaTemperatura = 25, maxTemperatura = 26, minTemperatura = 24;
    FILE* arqDadosTemperatura;
    
    while (1) {
        arqDadosTemperatura = fopen("DadosTemperatura.txt", "w");

        if (arqDadosTemperatura == NULL) {
            printf("Erro na criação do arquivo DadosTemperatura");
            exit(1);
        }

        for (i = 0; i < MaxArray; i++) {
            if (i < MaxArray / 3) {
                fprintf(arqDadosTemperatura, "%d", mediaTemperatura);
            }
            else if (i >= MaxArray / 3 && i < 2 / 3 * MaxArray) {
                fprintf(arqDadosTemperatura, "%d", maxTemperatura);
            }
            else if (i >= 2 / 3 * MaxArray && i < MaxArray) {
                fprintf(arqDadosTemperatura, "%d", minTemperatura);
            }
        }

        i = 0;

        fclose(arqDadosTemperatura);

        printf(" \nOs dados de temperatura do ambiente foram gerados");
        vTaskDelay(1000000);
    }

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