#ifndef INTERFAZ_H
#define INTERFAZ_H

#include "api.h"
#include "juego.h"

#define SALIR_DEL_JUEGO -2

void iniciarInterfaz();
void cerrarInterfaz();
void mostrarMenuPrincipal();
int mostrarSeleccionDificultad();
void mostrarRanking(const JugadorRanking* ranking, int numJugadores);
void pedirNombreJugador(char* nombre);
int esperarMenuPrincipal();
void iniciarPantallaJuego(const Partida* partida);
void terminarPantallaJuego();
int elegirCartaGUI(const Jugador* jugador, const Partida* partida);
void mostrarTurnoJugador(const char* nombreJugador, int esIA);
void mostrarResultadoPartida(const Partida* partida);
int preguntarUsarEspejoGUI(const Jugador* jugador, TipoCarta cartaAtacante);
int interfazSigueCorriendo();

/* Audio functions */
void reproducirMusicaJuego();
void reproducirSonidoCarta();
void reproducirSonidoVictoria();
void reproducirSonidoDerrota();

#endif // INTERFAZ_H
