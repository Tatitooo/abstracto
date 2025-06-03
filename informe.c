#include "informe.h"
#include <stdio.h>
#include <time.h>

void generar_informe(const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        printf("Error al crear informe\n");
        return;
    }
    
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    
    fprintf(archivo, "Informe generado: %02d/%02d/%04d %02d:%02d\n",
            tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
            tm->tm_hour, tm->tm_min);
    
    fprintf(archivo, "Estado del juego: OK\n");
    fclose(archivo);
}

void agregar_linea_informe(const char* linea) {
    FILE* archivo = fopen("informe.txt", "a");
    if (!archivo) return;
    
    fprintf(archivo, "%s\n", linea);
    fclose(archivo);
}