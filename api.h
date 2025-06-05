#ifndef API_H
#define API_H

typedef struct {
    char* data;
    size_t size;
} ApiResponse;

typedef struct {
    void* curl;
    char* base_url;
} ApiClient;

ApiClient* crear_api_client(const char* base_url);
ApiResponse* hacer_peticion(ApiClient* client, const char* endpoint);
void liberar_api_response(ApiResponse* response);
void liberar_api_client(ApiClient* client);

#endif