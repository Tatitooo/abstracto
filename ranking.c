#include "ranking.h"
#include <string.h>

Ranking* crear_ranking() {
    Ranking* ranking = (Ranking*)malloc(sizeof(Ranking));
    ranking->jugadores = NULL;
    ranking->cantidad = 0;
    return ranking;
}

void agregar_jugador_ranking(Ranking* ranking, const char* nombre, int puntaje) {
    ranking->cantidad++;
    ranking->jugadores = (Jugador_Ranking*)realloc(ranking->jugadores, 
                                                   ranking->cantidad * sizeof(Jugador_Ranking));
    
    strcpy(ranking->jugadores[ranking->cantidad - 1].nombre, nombre);
    ranking->jugadores[ranking->cantidad - 1].puntaje = puntaje;
}

void mostrar_ranking(Ranking* ranking) {
    printf("=== RANKING ===\n");
    for (int i = 0; i < ranking->cantidad; i++) {
        printf("%d. %s - %d puntos\n", i + 1, 
               ranking->jugadores[i].nombre, 
               ranking->jugadores[i].puntaje);
    }
}

void liberar_ranking(Ranking* ranking) {
    if (ranking->jugadores) {
        free(ranking->jugadores);
    }
}