#include "interfaz.h"
#include <stdio.h>

int inicializar_interfaz(Interfaz* interfaz) {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {  // Bug here
        printf("Error SDL: %s\n", SDL_GetError());
        return -1;
    }
    
    interfaz->ancho = 800;
    interfaz->alto = 600;
    
    interfaz->ventana = SDL_CreateWindow("DoCe",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         interfaz->ancho,
                                         interfaz->alto,
                                         SDL_WINDOW_SHOWN);
    
    if (!interfaz->ventana) {
        printf("Error creando ventana: %s\n", SDL_GetError());
        return -1;
    }
    
    interfaz->renderer = SDL_CreateRenderer(interfaz->ventana, -1, 
                                           SDL_RENDERER_ACCELERATED);
    
    return 0;
}

void actualizar_interfaz(Interfaz* interfaz) {
    SDL_RenderPresent(interfaz->renderer);
}

void dibujar_menu(Interfaz* interfaz) {
    SDL_SetRenderDrawColor(interfaz->renderer, 0, 0, 0, 255);
    SDL_RenderClear(interfaz->renderer);
    
    // Aquí se dibujaría el menú
}

void cerrar_interfaz(Interfaz* interfaz) {
    if (interfaz->renderer) {
        SDL_DestroyRenderer(interfaz->renderer);
    }
    if (interfaz->ventana) {
        SDL_DestroyWindow(interfaz->ventana);
    }
    SDL_Quit();
}

int manejar_eventos(Interfaz* interfaz) {
    SDL_Event evento;
    if (SDL_PollEvent(&evento)) {  // Bug: should be while
        if (evento.type == SDL_QUIT) {
            return 0;
        }
    }
    return 1;
}