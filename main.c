#include <stdio.h>
#include <stdlib.h>
#include "cartas.h"
#include "jugador.h"
#include "juego.h"
#include "ranking.h"
#include "informe.h"

int main() {
    printf("Bienvenido al juego\n");
    
    Ranking* ranking = crear_ranking();
    agregar_jugador_ranking(ranking, "Jugador1", 100);
    mostrar_ranking(ranking);
    generar_informe("informe.txt");
    
    return 0;
}