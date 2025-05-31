#include "ranking.h"
#include "interfaz.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int leerConfigApi(ConfigApi* config)
{
    FILE* archivo = fopen("config.txt", "r");
    if (!archivo) return 0;
    fscanf(archivo, "%127s | %15s", config->url, config->codigoGrupo);
    fclose(archivo);
    return 1;
}

/* Función de comparación para ordenarlos con qsort */
static int compararJugadores(const void* a, const void* b)
{
    const JugadorRanking* jugadorA = (const JugadorRanking*)a;
    const JugadorRanking* jugadorB = (const JugadorRanking*)b;

    /* Primero comparar por victorias (orden descendente) */
    if (jugadorB->victorias != jugadorA->victorias)
    {
        return jugadorB->victorias - jugadorA->victorias;
    }

    /* En caso de empate, comparar por nombre (orden alfabético ascendente) */
    return strcmp(jugadorA->nombre, jugadorB->nombre);
}

void mostrarRankingApi(const ConfigApi* config)
{
    JugadorRanking* ranking = NULL;
    int numJugadores = 0;

    if (obtenerRankingAPI(&ranking, &numJugadores))
    {
        /* Ordenar el ranking si hay jugadores */
        if (ranking != NULL && numJugadores > 0)
        {
            qsort(ranking, numJugadores, sizeof(JugadorRanking), compararJugadores);
        }
        mostrarRanking(ranking, numJugadores);
        free(ranking);  // Liberar la memoria que asignamos para mostrar el ranking
    }
    else
    {
        mostrarRanking(NULL, 0);  // Mostramos el ranking vacío en caso de que falle la llamada a la API
    }
}
