#ifndef CARTAS_H
#define CARTAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Cantidad de cada tipo de carta */
#define CANT_MAS2 6
#define CANT_MAS1 10
#define CANT_MENOS1 8
#define CANT_MENOS2 6
#define CANT_REPETIR 6
#define CANT_ESPEJO 4

#define MAX_MANO 3
#define MAX_NOMBRE_CARTA 20
#define ELIMINAR_CARTA "-"
#define MIN(X,Y) ((X)<=(Y) ? (X) : (Y))

/* Las cartas totales van a estar definidas por la cantidad de cartas que definimos arriba */
#define TAM_PILA CANT_MAS2 + CANT_MAS1 + CANT_MENOS1 + CANT_MENOS2 + CANT_REPETIR + CANT_ESPEJO
#define TODO_OK 1
#define PILA_LLENA -1
#define PILA_VACIA -2
#define ERROR_MEMORIA -3

/* A cada tipo de carta le asignamos un número */
typedef enum
{
    MAS2,
    MAS1,
    MENOS1,
    MENOS2,
    REPETIR,
    ESPEJO
} TipoCarta;

typedef struct
{
    TipoCarta tipo;
    char nombre[MAX_NOMBRE_CARTA];
} Carta;

/* Estructura para los datos de tamaño variable en la pila semiestática*/
typedef struct
{
    void* dato;
    unsigned tamDato;
} tDato;

/* Estructura de pila semiestática */
typedef struct
{
    tDato* pila[TAM_PILA];
    int tope;
} tPila;

/* Si bien un mazo es una pila, por temas de legibilidad definimos el nombre tMazo */
typedef tPila tMazo;

/* Primitivas de la pila semiestática */
void crearPila(tPila* p);
int pilaLlena(const tPila* p);
int pilaVacia(const tPila* p);
int ponerEnPila(tPila* p, const void* elem, unsigned tamElem);
int sacarDePila(tPila* p, void* elem, unsigned tamElem);
int verTope(const tPila* p, void* elem, unsigned tamElem);
void vaciarPila(tPila* p);

/* Funciones específicas del mazo usando las primitivas de pila */
void inicializarMazo(tMazo* mazo);
void mezclarMazo(tMazo* mazo);
void agregarCartaMazo(tMazo* mazo, Carta carta);
Carta robarCarta(tMazo* mazo);
void descartarCarta(tMazo* descarte, Carta carta);
void vaciarMazo(tMazo* mazo);
void crearMazoInicial(tMazo* mazo);
void asegurarCartasDisponibles(tMazo* mazo, tMazo* descarte);

const char* nombreCarta(TipoCarta tipo);

#endif // CARTAS_H
