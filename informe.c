#include "informe.h"
#include <stdio.h>
#include <stdlib.h>

void obtenerFechaHoraActual(char* buffer, int tamMaximo)
{
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    strftime(buffer, tamMaximo, "%Y-%m-%d-%H-%M", tm_info);
}

void generarInformePartida(const Partida* partida)
{
    char fechaHora[32];
    char nombreArchivo[64];
    HistorialJugada* h;
    int turno = 1;
    FILE* f;

    /* Poner el nombre del archivo con la hora actual */
    obtenerFechaHoraActual(fechaHora, sizeof(fechaHora));
    snprintf(nombreArchivo, sizeof(nombreArchivo), "informes/informe-juego_%s.txt", fechaHora);
    f = fopen(nombreArchivo, "w");
    if (!f) return;

    fprintf(f, "Informe de partida DoCe\n\n");
    fprintf(f, "Jugador: %s\nIA: %s\nDificultad: %d\n\n", partida->jugador.nombre, partida->ia.nombre, partida->dificultad);
    fprintf(f, "Turnos:\n");
    h = partida->historial;
    while (h)
    {
        fprintf(f, "Turno %d: %s jugó '%s' | Puntos: %d - %d\n", turno, h->nombreJugador, h->nombreCarta, h->puntosHumano, h->puntosIA);
        h = h->siguiente;
        turno++;
    }

    fprintf(f, "\nResultado final:\n%s: %d puntos\n%s: %d puntos\nGanador: %s\n",
            partida->jugador.nombre, partida->jugador.puntos,
            partida->ia.nombre, partida->ia.puntos,
            partida->jugador.esVencedor ? partida->jugador.nombre : partida->ia.nombre);
    fclose(f);
}
