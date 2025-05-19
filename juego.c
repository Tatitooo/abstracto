#include "juego.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void agregarHistorial(Partida* partida, const char* nombreJugador, const char* nombreCarta, int puntosActual, int puntosSig, TipoJugador tipo)
{
    HistorialJugada* nuevo = (HistorialJugada*)malloc(sizeof(HistorialJugada));
    strncpy(nuevo->nombreJugador, nombreJugador, MAX_NOMBRE_JUGADOR);
    strncpy(nuevo->nombreCarta, nombreCarta, MAX_NOMBRE_CARTA);
    nuevo->siguiente = NULL;
    nuevo->puntosHumano = tipo == HUMANO ? puntosActual : puntosSig;
    nuevo->puntosIA = tipo == HUMANO ? puntosSig : puntosActual;

    if (!partida->historial)
    {
        partida->historial = nuevo;
    }
    else
    {
        HistorialJugada* actual = partida->historial;
        while (actual->siguiente) actual = actual->siguiente;
        actual->siguiente = nuevo;
    }
}

void liberarPartida(Partida* partida)
{
    vaciarMazo(&partida->mazo);
    vaciarMazo(&partida->descarte);
    HistorialJugada* h = partida->historial;
    while (h)
    {
        HistorialJugada* temp = h;
        h = h->siguiente;
        free(temp);
    }
    partida->historial = NULL;
}

void inicializarPartida(Partida* partida, const char* nombreJugador, int dificultad)
{
    partida->dificultad = dificultad;
    partida->historial = NULL;

    partida->ultimaCartaJugador.tipo = MAS1;
    strcpy(partida->ultimaCartaJugador.nombre, "");
    partida->ultimaCartaIA.tipo = MAS1;
    strcpy(partida->ultimaCartaIA.nombre, "");

    crearMazoInicial(&partida->mazo);
    inicializarMazo(&partida->descarte);
    inicializarJugador(&partida->jugador, nombreJugador, HUMANO);
    inicializarJugador(&partida->ia, "IA", dificultad == 0 ? IA_FACIL : (dificultad == 1 ? IA_MEDIO : IA_DIFICIL));
    for (int i = 0; i < MAX_MANO; i++)
    {
        partida->jugador.mano[i] = robarCarta(&partida->mazo);
        partida->ia.mano[i] = robarCarta(&partida->mazo);
    }
    partida->turnoActual = rand() % 2;
}

int preguntarUsarEspejo(Jugador* jugador, TipoCarta cartaAtacante)
{
    int i;
    for (i = 0; i < MAX_MANO; i++)
    {
        if (jugador->mano[i].tipo == ESPEJO)
        {
            if (jugador->tipo == IA_DIFICIL)
            {
                return i;
            }
            else if (jugador->tipo != HUMANO)
            {
                return rand() % MAX_MANO == 0 ? i : -1;
            }
            else
            {
                return -1;
            }
        }
    }
    return -1;
}

void aplicarEfectoCarta(Partida* partida, Jugador* actual, Jugador* rival, int idxCarta, int* repetirTurno, int* efectoEspejo)
{
    Carta carta = actual->mano[idxCarta];
    int espejoUsado = -1;

    if (carta.tipo == MENOS1 || carta.tipo == MENOS2)
    {
        espejoUsado = preguntarUsarEspejo(rival, carta.tipo);
        if (espejoUsado >= 0)
        {
            *efectoEspejo = 1;
            if (carta.tipo == MENOS1)
            {
                actual->puntos = actual->puntos > 0 ? actual->puntos - 1 : 0;
            }
            else if (carta.tipo == MENOS2)
            {
                actual->puntos = actual->puntos > 1 ? actual->puntos - 2 : 0;
            }
            descartarCarta(&partida->descarte, rival->mano[espejoUsado]);
            strcpy(rival->mano[espejoUsado].nombre, ELIMINAR_CARTA);
            robarCartaMano(rival, &partida->mazo, &partida->descarte);
        }
    }

    if (espejoUsado < 0)
    {
        switch (carta.tipo)
        {
        case MAS2:
            actual->puntos += 2;
            break;
        case MAS1:
            actual->puntos += 1;
            break;
        case MENOS1:
            rival->puntos = rival->puntos > 0 ? rival->puntos - 1 : 0;
            break;
        case MENOS2:
            rival->puntos = rival->puntos > 1 ? rival->puntos - 2 : 0;
            break;
        case REPETIR:
            *repetirTurno = 1;
            break;
        case ESPEJO:
            break;
        default:
            break;
        }
    }
}

void jugarPartida(Partida* partida)
{
    int fin = 0;
    int repetirTurno, efectoEspejo, idxCarta;

    while (!fin) {
        Jugador* actual = partida->turnoActual == 0 ? &partida->jugador : &partida->ia;
        Jugador* rival = partida->turnoActual == 0 ? &partida->ia : &partida->jugador;

        idxCarta = -1;
        if (actual->tipo == HUMANO)
        {
            idxCarta = 0;
        }
        else
        {
            idxCarta = elegirCartaIA(actual, rival);
        }

        repetirTurno = 0;
        efectoEspejo = 0;

        if (actual->tipo == HUMANO) {
            partida->ultimaCartaJugador = actual->mano[idxCarta];
        } else {
            partida->ultimaCartaIA = actual->mano[idxCarta];
        }

        aplicarEfectoCarta(partida, actual, rival, idxCarta, &repetirTurno, &efectoEspejo);
        agregarHistorial(partida, actual->nombre, nombreCarta(actual->mano[idxCarta].tipo), actual->puntos, rival->puntos, actual->tipo);
        descartarCarta(&partida->descarte, actual->mano[idxCarta]);
        strcpy(actual->mano[idxCarta].nombre, ELIMINAR_CARTA);
        robarCartaMano(actual, &partida->mazo, &partida->descarte);

        if (efectoEspejo)
        {
            agregarHistorial(partida, rival->nombre, nombreCarta(ESPEJO), actual->puntos, rival->puntos, actual->tipo);
            partida->turnoActual = 1 - partida->turnoActual;
        }

        if (actual->puntos >= PUNTOS_GANAR)
        {
            actual->esVencedor = 1;
            fin = 1;
        }
        else if (!repetirTurno)
        {
            partida->turnoActual = 1 - partida->turnoActual;
        }
        // hacer: agregar la interfaz
    }
    // hacer: agregar la interfaz
}
