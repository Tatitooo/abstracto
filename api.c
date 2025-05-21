#include "api.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ApiResponse* response = (ApiResponse*)userp;
    
    char* ptr = realloc(response->data, response->size + realsize + 1);
    if (!ptr) return 0;
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0;
    
    return realsize;
}

ApiClient* crear_api_client(const char* base_url) {
    ApiClient* client = (ApiClient*)malloc(sizeof(ApiClient));
    client->base_url = strdup(base_url);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    client->curl = curl_easy_init();
    
    return client;
}

ApiResponse* hacer_peticion(ApiClient* client, const char* endpoint) {
    if (!client->curl) return NULL;
    
    ApiResponse* response = (ApiResponse*)malloc(sizeof(ApiResponse));
    response->data = malloc(1);
    response->size = 0;
    
    char url[256];
    snprintf(url, sizeof(url), "%s%s", client->base_url, endpoint);
    
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, (void*)response);
    
    CURLcode res = curl_easy_perform(client->curl);
    
    if (res != CURLE_OK) {
        free(response->data);
        free(response);
        return NULL;
    }
    
    return response;
}

void liberar_api_response(ApiResponse* response) {
    if (response) {
        free(response->data);
        free(response);
    }
}

void liberar_api_client(ApiClient* client) {
    if (client) {
        if (client->curl) {
            curl_easy_cleanup(client->curl);
        }
        free(client->base_url);
        free(client);
        curl_global_cleanup();
    }
}