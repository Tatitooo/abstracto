#include "juego.h"
#include "interfaz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"

void agregarHistorial(Partida* partida, const char* nombreJugador, const char* nombreCarta, int puntosActual, int puntosSig, TipoJugador tipo)
{
    HistorialJugada* nuevo = (HistorialJugada*)malloc(sizeof(HistorialJugada));
    strncpy(nuevo->nombreJugador, nombreJugador, MAX_NOMBRE_JUGADOR);
    strncpy(nuevo->nombreCarta, nombreCarta, MAX_NOMBRE_CARTA);
    nuevo->siguiente = NULL;
    nuevo->puntosHumano = tipo == HUMANO ? puntosActual : puntosSig;
    nuevo->puntosIA = tipo == HUMANO ? puntosSig : puntosActual;

    /* Verificar si es el primer turno registrado */
    if (!partida->historial)
    {
        partida->historial = nuevo;
    }
    /* Agregar al final de la lista de turnos de la partida*/
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
    /* Vaciar historial */
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
    partida->dificultad = dificultad;  // entre 0 y 2
    partida->historial = NULL;

    /* Inicializamos las últimas cartas con un valor default */
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
    /* Hacer que empiece un jugador aleatorio */
    partida->turnoActual = rand() % 2;
}

void aplicarEfectoCarta(Partida* partida, Jugador* actual, Jugador* rival, int idxCarta, int* repetirTurno, int* efectoEspejo)
{
    Carta carta = actual->mano[idxCarta];
    int espejoUsado = -1;
    int puntosRivalAntes = rival->puntos;

    /* Nos fijamos si el rival quiere usar la carta espejo */
    if (carta.tipo == MENOS1 || carta.tipo == MENOS2)
    {
        espejoUsado = preguntarUsarEspejo(rival, carta.tipo, actual->puntos);
        if (espejoUsado >= 0)
        {
            /* Primero aplicamos el efecto original para el historial */
            if (carta.tipo == MENOS1)
            {
                rival->puntos = rival->puntos > 0 ? rival->puntos - 1 : 0;
            }
            else if (carta.tipo == MENOS2)
            {
                rival->puntos = rival->puntos > 1 ? rival->puntos - 2 : 0;
            }
            /* Registramos la jugada original con los puntos que habrían quedado */
            agregarHistorial(partida, actual->nombre, nombreCarta(carta.tipo), actual->puntos, rival->puntos, actual->tipo);

            /* Ahora revertimos el efecto y aplicamos el espejo */
            rival->puntos = puntosRivalAntes; // Restauramos los puntos originales
            if (carta.tipo == MENOS1)
            {
                actual->puntos = actual->puntos > 0 ? actual->puntos - 1 : 0;
            }
            else if (carta.tipo == MENOS2)
            {
                actual->puntos = actual->puntos > 1 ? actual->puntos - 2 : 0;
            }

            /* Se usó la carta espejo */
            *efectoEspejo = 1;

            if (rival->tipo == HUMANO) {
                partida->ultimaCartaJugador = rival->mano[espejoUsado];
            } else {
                partida->ultimaCartaIA = rival->mano[espejoUsado];
            }
            descartarCarta(&partida->descarte, rival->mano[espejoUsado]);
            strcpy(rival->mano[espejoUsado].nombre, ELIMINAR_CARTA);
            robarCartaMano(rival, &partida->mazo, &partida->descarte);

            /* Registramos la jugada del espejo con los puntos finales */
            agregarHistorial(partida, rival->nombre, nombreCarta(ESPEJO), rival->puntos, actual->puntos, rival->tipo);
            return; // Salimos porque ya registramos todo
        }
        else if (espejoUsado == -2) // El jugador decidió no usar la carta espejo
        {
            rival->rechazoEspejo = 1; // Marcamos que rechazó usar el espejo
        }
    }

    /* Si el espejo no fue usado, aplicar el efecto de la carta */
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
            /* Se rechazó la oferta de usar el espejo así que ahora no hace nada */
            break;
        default:
            break;
        }
        /* Registramos la jugada normal */
        agregarHistorial(partida, actual->nombre, nombreCarta(carta.tipo), actual->puntos, rival->puntos, actual->tipo);
    }
}

void jugarPartida(Partida* partida)
{
    int fin = 0;
    int repetirTurno, efectoEspejo, idxCarta;

    iniciarPantallaJuego(partida);
    reproducirMusicaJuego();

    while (!fin) {
        Jugador* actual = partida->turnoActual == 0 ? &partida->jugador : &partida->ia;
        Jugador* rival = partida->turnoActual == 0 ? &partida->ia : &partida->jugador;

        /* Resaltar de quién es el turno */
        mostrarTurnoJugador(actual->nombre, actual->tipo != HUMANO);

        /* Nos fijamos si la interfaz todavía está ejecutándose */
        if (!interfazSigueCorriendo()) {
            terminarPantallaJuego();
            return;
        }

        idxCarta = -1;
        if (actual->tipo == HUMANO)
        {
            idxCarta = elegirCartaGUI(actual, partida);

            if (idxCarta == SALIR_DEL_JUEGO) {
                terminarPantallaJuego();
                return;
            }

            /* Si el jugador rechazó usar el espejo, no puede usarlo en este turno */
            if (actual->rechazoEspejo && idxCarta >= 0 && actual->mano[idxCarta].tipo == ESPEJO)
            {
                idxCarta = -1; // Forzar a elegir otra carta
                continue;
            }
        }
        else
        {
            idxCarta = elegirCartaIA(actual, rival);
            /* Verificar si el juego fue terminado durante el turno de la IA */
            if (!interfazSigueCorriendo()) {
                terminarPantallaJuego();
                return;
            }
        }

        repetirTurno = 0;
        efectoEspejo = 0;

        /* Antes de aplicar los efectos, guardamos la última carta jugada */
        if (actual->tipo == HUMANO) {
            partida->ultimaCartaJugador = actual->mano[idxCarta];
            reproducirSonidoCarta();
        } else {
            partida->ultimaCartaIA = actual->mano[idxCarta];
            reproducirSonidoCarta();
            /* Si la IA juega repetir, mostramos la nueva carta */
            if (actual->mano[idxCarta].tipo == REPETIR) {
                mostrarTurnoJugador(actual->nombre, 1);
                if (!interfazSigueCorriendo()) {
                    terminarPantallaJuego();
                    return;
                }
            }
        }

        aplicarEfectoCarta(partida, actual, rival, idxCarta, &repetirTurno, &efectoEspejo);
        descartarCarta(&partida->descarte, actual->mano[idxCarta]);
        strcpy(actual->mano[idxCarta].nombre, ELIMINAR_CARTA);
        robarCartaMano(actual, &partida->mazo, &partida->descarte);

        if (efectoEspejo)
        {
            partida->turnoActual = 1 - partida->turnoActual;
        }
        /* Nos fijamos si ya ganó alguno */
        if (actual->puntos >= PUNTOS_GANAR)
        {
            actual->esVencedor = 1;
            fin = 1;
        }
        else if (!repetirTurno)
        {
            /* Resetear el flag de rechazo de espejo al cambiar de turno */
            actual->rechazoEspejo = 0;
            partida->turnoActual = 1 - partida->turnoActual;
            /* Delay entre que se cambian jugadores */
            mostrarTurnoJugador(NULL, -1);
        }
    }

    if (partida->jugador.esVencedor) {
        reproducirSonidoVictoria();
    } else if (partida->ia.esVencedor) {
        reproducirSonidoDerrota();
    }

    mostrarResultadoPartida(partida);

    /* Enviar resultado a la API */
    if (partida->jugador.esVencedor || partida->ia.esVencedor) {
        enviarResultado(partida->jugador.nombre, partida->jugador.esVencedor);
    }

    if (!interfazSigueCorriendo()) {
        terminarPantallaJuego();
        return;
    }

    terminarPantallaJuego();
}
