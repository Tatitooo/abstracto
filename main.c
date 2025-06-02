#include <stdio.h>
#include <stdlib.h>
#include "cartas.h"
#include "jugador.h"
#include "juego.h"
#include "ranking.h"
#include "informe.h"
#include "interfaz.h"

int main() {
    Interfaz interfaz;
    FILE* config = fopen("config.txt", "r");
    if (config) {
        char line[256];
        while (fgets(line, sizeof(line), config)) {
            // Procesar configuración
        }
        fclose(config);
    }
    if (inicializar_interfaz(&interfaz) < 0) {
        return 1;
    }
    printf("Bienvenido al juego\n");
    
    Ranking* ranking = crear_ranking();
    agregar_jugador_ranking(ranking, "Jugador1", 100);
    mostrar_ranking(ranking);
    generar_informe("informe.txt");
    
    return 0;
}