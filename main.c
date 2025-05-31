#include "interfaz.h"
#include "juego.h"
#include "informe.h"
#include "ranking.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    srand((unsigned int)time(NULL));
    int opcion, dificultad, salir = 0;
    char nombre[MAX_NOMBRE_JUGADOR];
    Partida partida;
    ConfigApi config;

    iniciarInterfaz();

    while (!salir && interfazSigueCorriendo())
    {
        mostrarMenuPrincipal();
        opcion = esperarMenuPrincipal();

        if (!interfazSigueCorriendo())
        {
            salir = 1;
            break;
        }

        if (opcion == 0)   // Jugar
        {
            pedirNombreJugador(nombre);

            if (!interfazSigueCorriendo())
            {
                salir = 1;
                break;
            }

            dificultad = mostrarSeleccionDificultad();

            if (!interfazSigueCorriendo())
            {
                salir = 1;
                break;
            }

            inicializarPartida(&partida, nombre, dificultad);
            jugarPartida(&partida);
            generarInformePartida(&partida);
            liberarPartida(&partida);
        }
        else if (opcion == 1)     // Ranking
        {
            if (leerConfigApi(&config))
            {
                mostrarRankingApi(&config);
            }
        }
        else if (opcion == 2)     // Salir
        {
            salir = 1;
        }
    }
    cerrarInterfaz();
    return 0;
}
