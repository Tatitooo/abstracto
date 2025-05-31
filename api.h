#ifndef API_H
#define API_H

#include <curl/curl.h>

#define LINK_API "https://algoritmos-api.azurewebsites.net/api/doce"

/* Estructura para la configuración de la API */
typedef struct
{
    char url[256];
    char codigo[64];
} ConfigAPI;

/* Estructura para almacenar la respuesta de la API */
typedef struct
{
    char* datos;
    size_t tam;
} RespuestaAPI;

/* Estructura para el ranking */
typedef struct
{
    char nombre[32];
    int victorias;
} JugadorRanking;

/* Funciones principales */
int leerConfiguracionAPI(ConfigAPI* config);
int enviarResultado(const char* nombreJugador, int esVencedor);
int obtenerRankingAPI(JugadorRanking** ranking, int* numJugadores);

#endif /* API_H */
