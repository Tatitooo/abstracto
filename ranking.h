#ifndef RANKING_H
#define RANKING_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char nombre[50];
    int puntaje;
} Jugador_Ranking;

typedef struct {
    Jugador_Ranking* jugadores;
    int cantidad;
} Ranking;

Ranking* crear_ranking();
void agregar_jugador_ranking(Ranking* ranking, const char* nombre, int puntaje);
void mostrar_ranking(Ranking* ranking);
void liberar_ranking(Ranking* ranking);

#endif