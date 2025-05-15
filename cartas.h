#ifndef CARTAS_H
#define CARTAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MANO 3
#define MAX_NOMBRE_CARTA 100
#define ELIMINAR_CARTA "-"
#define MIN(X,Y) ((X)<=(Y) ? (X) : (Y))

#define TAM_PILA 40
#define TODO_OK 1
#define PILA_LLENA -1
#define PILA_VACIA -2
#define ERROR_MEMORIA -3
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

typedef struct
{
    void* dato;
    unsigned tamDato;

} tDato;

typedef struct
{
    tDato* pila[TAM_PILA];
    int tope;
} tPila;

typedef tPila tMazo;

void crearPila(tPila* p);
int pilaLlena(const tPila* p);
int pilaVacia(const tPila* p);
int ponerEnPila(tPila* p, const void* elem, unsigned tamElem);
int sacarDePila(tPila* p, void* elem, unsigned tamElem);
int verTope(const tPila* p, void* elem, unsigned tamElem);
void vaciarPila(tPila* p);
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
