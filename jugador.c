#include "jugador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void inicializarJugador(Jugador* jugador, const char* nombre, TipoJugador tipo)
{
    strncpy(jugador->nombre, nombre, MAX_NOMBRE_JUGADOR);
    jugador->puntos = 0;
    jugador->tipo = tipo;
    jugador->esVencedor = 0;
    int i = 0;

    for (i = 0; i < MAX_MANO; i++)
    {
        /* El vector mano es de tipo Carta, as� que le asignamos un valor default */
        jugador->mano[i].tipo = MAS1;
        strcpy(jugador->mano[i].nombre, "");
    }
}

void robarCartaMano(Jugador* jugador, tMazo* mazo, tMazo* descarte)
{   
    int i;
    for (i = 0; i < MAX_MANO; i++)
    {
        /* Si la carta est� seteada para ser eliminada, cambiar esa */
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

int elegirCartaIA(Jugador* jugador, Jugador* rival)
{
    int i, j, prioridad;
    int mejorCarta = -1, mayorPrioridad = -1, cartasBuenas = 0;
    TipoCarta tipo;

    /* Medio: Evita jugadas inefectivas */
    if (jugador->tipo == IA_MEDIO)
    {
        /* No usa una carta de "sacar puntos" si el oponente tiene 0 puntos. */
        /* Regla propia: Es menos probable que elija una carta de "-2" si el oponente tiene 1 punto */
        /* Si tiene 8 puntos o m�s, prioriza las cartas que suman puntos */

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
                prioridad = 2; // Normal
                break;
            }

            /* Guarda la carta con m�s prioridad. En caso de haber empate, va con la primera que tenga
            (que en la pr�ctica es lo mismo que hacerlo aleatorio) */
            if (prioridad > mayorPrioridad)
            {
                mayorPrioridad = prioridad;
                mejorCarta = i;
            }
        }

        return mejorCarta;
    }

    /* Dif�cil: Elige la mejor jugada */
    else if (jugador->tipo == IA_DIFICIL)
    {
        /* No usa una carta de "sacar puntos" si el oponente tiene 0 puntos. */
        /* Regla propia: Es menos probable que elija una carta de "-2" si el oponente tiene 1 punto */
        /* Si tiene 8 puntos o m�s, prioriza las cartas que suman puntos */
        /* Regla propia: en esa situaci�n prioriza los +2 sobre los +1 */
        /* Si el jugador est� cerca de ganar, prioriza "repetir turno" o "sacar punto" */
        /* Regla propia: dada esa situaci�n, prioriza el -2 sobre el -1 */
        /* Si recibe una carta negativa, usa la carta espejo. El resto de las veces prefiere no usarla. */
        /* Solo repite turno si las otras dos cartas son buenas (nuestra definici�n de "buena" va a ser +2, +1 o espejo)*/

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
                prioridad = -1; // �ltimo recurso
                break;
            }

            if (prioridad >= mayorPrioridad)
            {
                mayorPrioridad = prioridad;
                mejorCarta = i;
            }
        }

        return mejorCarta;
    }

    /* Si la IA est� en f�cil, o hay alg�n error de programaci�n tal que no se encuentra la dificultad,
    devuelve una carta random de su mano */
    return rand() % MAX_MANO;
}
