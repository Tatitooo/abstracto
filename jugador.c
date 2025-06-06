#include "jugador.h"
#include "interfaz.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void inicializarJugador(Jugador* jugador, const char* nombre, TipoJugador tipo)
{
    int i;
    strcpy(jugador->nombre, nombre);
    jugador->puntos = 0;
    jugador->esVencedor = 0;
    jugador->rechazoEspejo = 0;
    jugador->tipo = tipo;
    for (i = 0; i < MAX_MANO; i++)
    {
        strcpy(jugador->mano[i].nombre, ELIMINAR_CARTA);
    }
}

void robarCartaMano(Jugador* jugador, tMazo* mazo, tMazo* descarte)
{
    int i;
    for (i = 0; i < MAX_MANO; i++)
    {
        /* Si la carta está seteada para ser eliminada, cambiar esa */
        if (strcmp(jugador->mano[i].nombre, ELIMINAR_CARTA) == 0)
        {
            asegurarCartasDisponibles(mazo, descarte);

            /* Agarrar solo si hay cartas */
            if (pilaVacia(mazo) != PILA_VACIA)
            {
                jugador->mano[i] = robarCarta(mazo);
            }
        }
    }
}

/* Preguntar si el jugador quiere usar una tarjeta espejo después de un ataque */
int preguntarUsarEspejo(Jugador* jugador, TipoCarta cartaAtacante, int puntosRival)
{
    int i;
    int cantidadEspejos = 0;
    int indiceCartaEspejo = -1;

    /* Contar cuántas cartas espejo tiene y encontrar la primera */
    for (i = 0; i < MAX_MANO; i++)
    {
        if (jugador->mano[i].tipo == ESPEJO)
        {
            if (indiceCartaEspejo == -1)
            {
                indiceCartaEspejo = i;
            }
            cantidadEspejos++;
        }
    }

    /* Si no hay carta espejo... */
    if (cantidadEspejos == 0)
    {
        return -1;
    }

    /* Si tiene tres cartas espejo, está forzado a usar una */
    if (cantidadEspejos == MAX_MANO)
    {
        return indiceCartaEspejo;
    }

    /* Si es IA difícil, siempre devuelve */
    if (jugador->tipo == IA_DIFICIL)
    {
        return indiceCartaEspejo;
    }
    /* Si es otra dificultad, hay una probabilidad de cantidadEspejos/MAX_MANO de que devuelva */
    else if (jugador->tipo != HUMANO)
    {
        return ((rand() % MAX_MANO) < cantidadEspejos) ? indiceCartaEspejo : -2;
    }
    /* Si es humano, pregunta por la GUI */
    else
    {
        return preguntarUsarEspejoGUI(jugador, cartaAtacante, puntosRival);
    }
}

int elegirCartaIA(Jugador* jugador, Jugador* rival)
{
    int i, j, prioridad;
    int mejorCarta = -1, mayorPrioridad = -1, cartasBuenas = 0;
    TipoCarta tipo;

    /* Fácil: Elige una carta al azar */
    if (jugador->tipo == IA_FACIL)
    {
        do {
            mejorCarta = rand() % MAX_MANO;
            /* Si rechazó usar la carta espejo, no puede usarla */
        } while (jugador->mano[mejorCarta].tipo == ESPEJO && jugador->rechazoEspejo);
        /* Nunca va a haber while infinito porque siempre va a haber una carta que no sea espejo
           (si no, no la habría rechazado) */
        return mejorCarta;
    }

    /* Medio: Evita jugadas inefectivas */
    else if (jugador->tipo == IA_MEDIO)
    {
        /* No usa una carta de "sacar puntos" si el oponente tiene 0 puntos. */
        /* Regla propia: Es menos probable que elija una carta de "-2" si el oponente tiene 1 punto */
        /* Si tiene 8 puntos o más, prioriza las cartas que suman puntos */

        for (i = 0; i < MAX_MANO; i++)
        {
            tipo = jugador->mano[i].tipo;

            switch(tipo)
            {
            case MAS2:
                if (jugador->puntos >= PUNTOS_GANAR - 4) prioridad = 3; // Alta
                else prioridad = 2; // Normal
                break;
            case MAS1:
                if (jugador->puntos >= PUNTOS_GANAR - 4) prioridad = 3; // Alta
                else prioridad = 2; // Normal
                break;
            case MENOS2:
                if (rival->puntos == 0) prioridad = 0; // Muy baja
                else if (rival->puntos == 1) prioridad = 1; // Baja
                else prioridad = 2; // Normal
                break;
            case MENOS1:
                if (rival->puntos == 0) prioridad = 0; // Muy baja
                else prioridad = 2; // Normal
                break;
            case REPETIR:
                prioridad = 2; // Normal
                break;
            case ESPEJO:
                if (jugador->rechazoEspejo) prioridad = -1; // Nunca va a ser elegida
                else prioridad = 2; // Normal
                break;
            }

            /* Guarda la carta con más prioridad. En caso de haber empate, va con la primera que tenga
            (que en la práctica es lo mismo que hacerlo aleatorio) */
            if (prioridad > mayorPrioridad)
            {
                mayorPrioridad = prioridad;
                mejorCarta = i;
            }
        }

        return mejorCarta;
    }

    /* Difícil: Elige la mejor jugada */
    else if (jugador->tipo == IA_DIFICIL)
    {
        /* No usa una carta de "sacar puntos" si el oponente tiene 0 puntos. */
        /* Regla propia: Es menos probable que elija una carta de "-2" si el oponente tiene 1 punto */
        /* Si tiene 8 puntos o más, prioriza las cartas que suman puntos */
        /* Regla propia: en esa situación prioriza los +2 sobre los +1 */
        /* Si el jugador está cerca de ganar, prioriza "repetir turno" o "sacar punto" */
        /* Regla propia: dada esa situación, prioriza el -2 sobre el -1 */
        /* Si recibe una carta negativa, usa la carta espejo. El resto de las veces prefiere no usarla. */
        /* Solo repite turno si las otras dos cartas son buenas (nuestra definición de "buena" va a ser +2, +1 o espejo)*/

        for (i = 0; i < MAX_MANO; i++)
        {
            tipo = jugador->mano[i].tipo;

            switch(tipo)
            {
            case MAS2:
                if (jugador->puntos >= PUNTOS_GANAR - 3) prioridad = 8; // Muy alta
                else prioridad = 3; // Normal
                break;
            case MAS1:
                if (jugador->puntos >= PUNTOS_GANAR - 3) prioridad = 7; // Muy alta
                else prioridad = 3; // Normal
                break;
            case MENOS2:
                if (rival->puntos >= PUNTOS_GANAR - 3) prioridad = 6; // Alta
                else if (rival->puntos == 1) prioridad = 2; // Baja
                else if (rival->puntos == 0) prioridad = 1; // Muy baja
                else prioridad = 3; // Normal
                break;
            case MENOS1:
                if (rival->puntos >= PUNTOS_GANAR - 3) prioridad = 5; // Alta
                else if (rival->puntos == 0) prioridad = 1; // Muy baja
                else prioridad = 3; // Normal
                break;
            case REPETIR:
                if (rival->puntos >= PUNTOS_GANAR - 3) prioridad = 6; // Alta
                else prioridad = 1; // Muy baja
                /* Recorremos la mano a ver si al menos dos cartas son buenas */
                for (j = 0; j < MAX_MANO; j++)
                {
                    if (j != i)
                    {
                        if (jugador->mano[j].tipo == MAS2) cartasBuenas++;
                        else if (jugador->mano[j].tipo == MAS1) cartasBuenas++;
                        else if (jugador->mano[j].tipo == ESPEJO) cartasBuenas++;
                    }
                }
                if (cartasBuenas >= 2) prioridad = 4; // Normal (pero con preferencia)
                break;
            case ESPEJO:
                prioridad = 0; // Último recurso
                break;
            }

            /* Guardar la carta con más prioridad */
            if (prioridad > mayorPrioridad)
            {
                mayorPrioridad = prioridad;
                mejorCarta = i;
            }
        }

        return mejorCarta;
    }
    /* Nunca debería llegar acá, pero si lo hace, devuelve una carta al azar */
    return rand() % MAX_MANO;
}
