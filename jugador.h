#ifndef JUGADOR_H
#define JUGADOR_H

#include "cartas.h"

#define MAX_NOMBRE_JUGADOR 32

/* Asignar a cada tipo de jugador un n�mero de c�digo */
typedef enum
{
    HUMANO,
    IA_FACIL,
    IA_MEDIO,
    IA_DIFICIL
} TipoJugador;

/* Estructura de jugador para guardar la informaci�n de ambos participantes */
typedef struct
{
    char nombre[MAX_NOMBRE_JUGADOR];
    int puntos;
    Carta mano[MAX_MANO];
    TipoJugador tipo;
    int esVencedor;
} Jugador;

void inicializarJugador(Jugador* jugador, const char* nombre, TipoJugador tipo);
int elegirCartaIA(Jugador* jugador, Jugador* rival);
void robarCartaMano(Jugador* jugador, tMazo* mazo, tMazo* descarte);

#endif // JUGADOR_H
