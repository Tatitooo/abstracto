#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Window* ventana;
    SDL_Renderer* renderer;
    int ancho;
    int alto;
} Interfaz;

int inicializar_interfaz(Interfaz* interfaz);
void actualizar_interfaz(Interfaz* interfaz);
void dibujar_menu(Interfaz* interfaz);
void cerrar_interfaz(Interfaz* interfaz);
int manejar_eventos(Interfaz* interfaz);

#endif