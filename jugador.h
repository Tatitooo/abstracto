#ifndef JUGADOR_H
#define JUGADOR_H

#include "cartas.h"

#define MAX_NOMBRE_JUGADOR 20
#define PUNTOS_GANAR 12
#define SALIR_DEL_JUEGO -2

/* Asignar a cada tipo de jugador un número de código */
typedef enum
{
    HUMANO,
    IA_FACIL,
    IA_MEDIO,
    IA_DIFICIL
} TipoJugador;

/* Estructura de jugador para guardar la información de ambos participantes */
typedef struct
{
    char nombre[MAX_NOMBRE_JUGADOR];
    int puntos;
    int esVencedor;
    int rechazoEspejo;
    TipoJugador tipo;
    Carta mano[MAX_MANO];
} Jugador;

void inicializarJugador(Jugador* jugador, const char* nombre, TipoJugador tipo);
int elegirCartaIA(Jugador* jugador, Jugador* rival);
void robarCartaMano(Jugador* jugador, tMazo* mazo, tMazo* descarte);
int preguntarUsarEspejo(Jugador* jugador, TipoCarta cartaAtacante, int puntosRival);

#endif // JUGADOR_H
