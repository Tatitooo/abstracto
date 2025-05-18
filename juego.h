#ifndef JUEGO_H
#define JUEGO_H

#include "cartas.h"
#include "jugador.h"

#define PUNTOS_GANAR 12

/* Estructura para el historial de la jugada; es como una lista simplemente enlazada donde se guarda la información de cada turno */
typedef struct HistorialJugada
{
    char nombreJugador[MAX_NOMBRE_JUGADOR];
    char nombreCarta[MAX_NOMBRE_CARTA];
    int puntosHumano;
    int puntosIA;
    struct HistorialJugada* siguiente;
} HistorialJugada;

/* Estructura para guardar toda la información de la partida actual */
typedef struct
{
    Jugador jugador;
    Jugador ia;
    tMazo mazo;
    tMazo descarte;
    HistorialJugada* historial;
    int turnoActual; // Alterna entre 0 y 1
    int dificultad; // Podría calcularse como (partida.ia).tipo, pero por legibilidad lo agregamos a la estructura
    Carta ultimaCartaJugador; // Última carta jugada por el humano
    Carta ultimaCartaIA;     // Última carta jugada por la IA
} Partida;

void inicializarPartida(Partida* partida, const char* nombreJugador, int dificultad);
void jugarPartida(Partida* partida);
void liberarPartida(Partida* partida);
void agregarHistorial(Partida* partida, const char* nombreJugador, const char* nombreCarta, int puntosHumano, int puntosIA, TipoJugador tipo);

#endif // JUEGO_H
