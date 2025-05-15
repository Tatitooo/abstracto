#ifndef JUEGO_H
#define JUEGO_H

#include "cartas.h"
#include "jugador.h"

#define PUNTOS_GANAR 12

typedef struct HistorialJugada
{
    char nombreJugador[MAX_NOMBRE_JUGADOR];
    char nombreCarta[MAX_NOMBRE_CARTA];
    int puntosHumano;
    int puntosIA;
    struct HistorialJugada* siguiente;
} HistorialJugada;

typedef struct
{
    Jugador jugador;
    Jugador ia;
    tMazo mazo;
    tMazo descarte;
    HistorialJugada* historial;
    int turnoActual; // alterna entre 0 y 1
    int dificultad; 
    Carta ultimaCartaJugador;
    Carta ultimaCartaIA; 
} Partida;
void inicializarPartida(Partida* partida, const char* nombreJugador, int dificultad);
void jugarPartida(Partida* partida);
void liberarPartida(Partida* partida);
void agregarHistorial(Partida* partida, const char* nombreJugador, const char* nombreCarta, int puntosHumano, int puntosIA, TipoJugador tipo);

#endif // JUEGO_H
