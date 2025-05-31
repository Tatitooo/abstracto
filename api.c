#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/* Función de callback para recibir la respuesta de la API */
static size_t escribirCallback(void* contenido, size_t tamElemento, size_t numElementos, void* datosUsuario)
{
    size_t tamReal = tamElemento * numElementos;
    RespuestaAPI* resp = (RespuestaAPI*)datosUsuario;

    char* ptr = realloc(resp->datos, resp->tam + tamReal + 1);
    /* Si no se pudo realocar, falla la funcion */
    if (!ptr)
    {
        return 0;
    }

    resp->datos = ptr;
    memcpy(&(resp->datos[resp->tam]), contenido, tamReal);
    resp->tam += tamReal;
    resp->datos[resp->tam] = 0;

    return tamReal;
}

/* Lee la configuración del archivo config.txt */
int leerConfiguracionAPI(ConfigAPI* config)
{
    FILE* archivo = fopen("config.txt", "r");
    if (!archivo)
    {
        return 0;
    }

    /* Si el fscanf no asignó dos valores significa que leyó algo mal */
    if (fscanf(archivo, "%255[^|]|%63s", config->url, config->codigo) != 2)
    {
        fclose(archivo);
        return 0;
    }

    fclose(archivo);
    return 1;
}

/* Envía el resultado de la partida a la API */
int enviarResultado(const char* nombreJugador, int esVencedor)
{
    CURL* curl;
    CURLcode resultado;
    ConfigAPI config;
    char datosJson[512];
    char url[512];
    struct curl_slist* cabeceras = NULL;
    int exito = 0;
    long codigoHttp = 0;
    RespuestaAPI respuesta = {0};

    /* Si falla la lectrura de la API, evidentemente no envía resultado */
    if (!leerConfiguracionAPI(&config))
    {
        return 0;
    }

    /* Preparar el JSON */
    snprintf(datosJson, sizeof(datosJson),
             "{"
             "\"codigoGrupo\": \"%s\","
             "\"jugador\": {"
             "\"nombre\": \"%s\","
             "\"vencedor\": %d"
             "}"
             "}", config.codigo, nombreJugador, esVencedor);

    /* Construir URL para POST */
    snprintf(url, sizeof(url), LINK_API);

    curl = curl_easy_init();
    if (curl)
    {
        cabeceras = curl_slist_append(cabeceras, "Content-Type: application/json");

        /* Desactivar verificación SSL; si no lo hacemos no deja conectarnos */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        /* Configurar para recibir respuesta */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&respuesta);

        /* Usar la URL específica para POST */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, datosJson);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, cabeceras);

        resultado = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigoHttp);

        /* Si el código de HTTP es 200 y pico es que nos pudimos conectar */
        if (resultado == CURLE_OK)
        {
            exito = (codigoHttp >= 200 && codigoHttp < 300);
        }

        curl_slist_free_all(cabeceras);
        curl_easy_cleanup(curl);
    }

    /* Liberamos la memoria asignada en curl_easy_setopt() para que no haya fuga */
    if (respuesta.datos)
    {
        free(respuesta.datos);
    }

    return exito;
}

/* Obtiene el ranking de la API */
int obtenerRankingAPI(JugadorRanking** ranking, int* numJugadores)
{
    CURL* curl;
    CURLcode resultado;
    ConfigAPI config;
    RespuestaAPI respuesta = {0};
    int exito = 0;
    long codigoHttp = 0;
    char url[512];

    *ranking = NULL;
    *numJugadores = 0;

    /* Si falla la lectrura de la API, evidentemente no envía resultado */
    if (!leerConfiguracionAPI(&config))
    {
        return 0;
    }

    /* Construir URL para GET */
    snprintf(url, sizeof(url), LINK_API "/%s", config.codigo);

    curl = curl_easy_init();
    if (curl)
    {
        /* Desactivar verificación SSL */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&respuesta);

        resultado = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigoHttp);

        /* Si conseguimos los jugadores del ranking, los ordenamos para mostrarlos por orden de victorias y luego alfabético*/
        if (resultado == CURLE_OK && codigoHttp >= 200 && codigoHttp < 300 && respuesta.datos)
        {
            const char* datosTemp = respuesta.datos;
            while ((datosTemp = strstr(datosTemp, "\"nombreJugador\":")) != NULL)
            {
                (*numJugadores)++;
                datosTemp++;
            }

            if (*numJugadores > 0)
            {
                /* Asignar memoria para el array de ranking */
                *ranking = (JugadorRanking*)malloc(sizeof(JugadorRanking) * (*numJugadores));
                if (*ranking)
                {
                    /* Parsear el JSON manualmente */
                    char* pos = respuesta.datos;
                    int idx = 0;

                    while ((pos = strstr(pos, "\"nombreJugador\":")) != NULL && idx < *numJugadores)
                    {
                        pos = strchr(pos, ':') + 1;
                        while (*pos == ' ' || *pos == '"') pos++;

                        /* Copiar el nombre */
                        int i = 0;
                        while (*pos != '"' && i < 63)
                        {
                            (*ranking)[idx].nombre[i++] = *pos++;
                        }
                        (*ranking)[idx].nombre[i] = '\0';

                        /* Buscar victorias */
                        pos = strstr(pos, "\"cantidadPartidasGanadas\":");
                        if (pos)
                        {
                            pos = strchr(pos, ':') + 1;
                            (*ranking)[idx].victorias = atoi(pos);
                            idx++;
                        }
                    }
                }
            }
            exito = 1;
        }

        curl_easy_cleanup(curl);
    }

    /* Liberar la memoria asignada en las operaciones de cURL */
    if (respuesta.datos)
    {
        free(respuesta.datos);
    }

    return exito;
}
