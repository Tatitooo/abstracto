#ifndef RANKING_H
#define RANKING_H

#include "jugador.h"
#include "api.h"

#define MAX_URL 128
#define MAX_CODIGO_GRUPO 16

/* Estructura para leer la configuración de la API*/
typedef struct
{
    char url[MAX_URL];
    char codigoGrupo[MAX_CODIGO_GRUPO];
} ConfigApi;

int leerConfigApi(ConfigApi* config);
void mostrarRankingApi(const ConfigApi* config);

#endif // RANKING_H
