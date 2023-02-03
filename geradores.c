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

}

void GeradorTemperatura() {
   
    int maxArray = 30, mediaTemperatura = 25, maxTemperatura = 26, minTemperatura = 24, atualTemperatura[30];
    FILE* arqDadosTemperatura;

        arqDadosTemperatura = fopen("DadosTemperatura.txt", "w");

        for (int i = 0; i < maxArray; i++) {
            if (i >= 0 && i < maxArray / 3)
                atualTemperatura[i] = maxTemperatura;
            else
                if (i >=
                    
                    maxArray / 3 && i < 2 * maxArray / 3)
                    atualTemperatura[i] = mediaTemperatura;
                else
                    if (i >= 2 * maxArray / 3 && i < maxArray)
                        atualTemperatura[i] = minTemperatura;

            fprintf(arqDadosTemperatura, "%d ", atualTemperatura[i]);
        }

        fclose(arqDadosTemperatura);
}

void GeradorTensao() {

}

void GeradorParticulas() {
  
    int maxArray = 30, quantidadeParticula[30];
    FILE* arqDadosParticula;
    time_t t;

    /* Intializes random number generator */
    srand((unsigned)time(&t));

    arqDadosParticula = fopen("DadosParticula.txt", "w");

    // Geração de valores aleatórios de partículas
    for (int i = 0; i < maxArray; i++) {
        quantidadeParticula[i] = 1 + (rand() % 14);
        fprintf(arqDadosParticula, "%d ", quantidadeParticula[i]);
    }

    fclose(arqDadosParticula);
}

void GeradorPresencaGas() {
  
}