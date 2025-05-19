#include "cartas.h"
#include <time.h>

const char* nombreCarta(TipoCarta tipo)
{
    switch (tipo)
    {
    case MAS2:
        return "+2 puntos";
    case MAS1:
        return "+1 punto";
    case MENOS1:
        return "-1 punto rival";
    case MENOS2:
        return "-2 puntos rival";
    case REPETIR:
        return "Repetir turno";
    case ESPEJO:
        return "Espejo";
    default:
        return "Desconocida";
    }
}

/* Primitivas de la pila semiestática */
void crearPila(tPila* p)
{
    p->tope = TAM_PILA;
}

int pilaLlena(const tPila* p)
{
    return p->tope == 0 ? PILA_LLENA : 0;
}

int pilaVacia(const tPila* p)
{
    return p->tope == TAM_PILA ? PILA_VACIA : 0;
}

int ponerEnPila(tPila* p, const void* elem, unsigned tamElem)
{
    if (p->tope == 0)
        return PILA_LLENA;

    tDato* datoAIngresar = malloc(sizeof(tDato));
    if (!datoAIngresar)
        return ERROR_MEMORIA;

    datoAIngresar->dato = malloc(tamElem);
    if (!datoAIngresar->dato) {
        free(datoAIngresar);
        return ERROR_MEMORIA;
    }

    memcpy(datoAIngresar->dato, elem, tamElem);
    datoAIngresar->tamDato = tamElem;

    p->tope--;
    p->pila[p->tope] = datoAIngresar;

    return TODO_OK;
}

int sacarDePila(tPila* p, void* elem, unsigned tamElem)
{
    if(p->tope == TAM_PILA)
        return PILA_VACIA;

    tDato* datoExtraido = p->pila[p->tope];
    memcpy(elem, datoExtraido->dato, MIN(datoExtraido->tamDato, tamElem));

    free(datoExtraido->dato);
    free(datoExtraido);

    p->tope++;
    return TODO_OK;
}

int verTope(const tPila* p, void* elem, unsigned tamElem)
{
    if(p->tope == TAM_PILA)
        return PILA_VACIA;

    tDato* datoExtraido = p->pila[p->tope];
    memcpy(elem, datoExtraido->dato, MIN(datoExtraido->tamDato, tamElem));

    return TODO_OK;
}

void vaciarPila(tPila* p)
{
    for (int i = p->tope; i < TAM_PILA; i++) {
        tDato* dato = p->pila[i];
        if (dato) {
            free(dato->dato);
            free(dato);
        }
    }
    p->tope = TAM_PILA;
}

/* Funciones específicas del mazo usando las primitivas de pila */
void inicializarMazo(tMazo* mazo)
{
    crearPila(mazo);
}

void agregarCartaMazo(tMazo* mazo, Carta carta)
{
    ponerEnPila(mazo, &carta, sizeof(Carta));
}

Carta robarCarta(tMazo* mazo)
{
    Carta carta;
    Carta cartaVacia = {MAS1, ""}; // el MAS1 lo tomamos como valor default (un placeholder)

    if(sacarDePila(mazo, &carta, sizeof(Carta)) == TODO_OK)
    {
        return carta;
    }

    /* Ponemos un default por las dudas pero nunca debería fallar el sacarDePila porque mezclamos antes de que ocurra */
    return cartaVacia;
}

void descartarCarta(tMazo* descarte, Carta carta)
{
    agregarCartaMazo(descarte, carta);
}

void vaciarMazo(tMazo* mazo)
{
    vaciarPila(mazo);
}

void mezclarMazo(tMazo* mazo)
{
    int cantCartas = TAM_PILA - mazo->tope;

    /* Si hay una sola carta no tiene sentido mezclar */
    if(cantCartas < 2) return;

    /* Conseguir todas las cartas */
    Carta* cartas = (Carta*)malloc(sizeof(Carta) * cantCartas);
    for(int i = 0; i < cantCartas; i++)
    {
        sacarDePila(mazo, &cartas[i], sizeof(Carta));
    }

    /* Mezclar con el algoritmo Fisher-Yates */
    srand((unsigned int)time(NULL));
    for(int i = cantCartas - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Carta temp = cartas[i];
        cartas[i] = cartas[j];
        cartas[j] = temp;
    }

    /* Poner las cartas en el mazo de vuelta */
    for(int i = 0; i < cantCartas; i++)
    {
        agregarCartaMazo(mazo, cartas[i]);
    }

    free(cartas);
}

void crearMazoInicial(tMazo* mazo)
{
    inicializarMazo(mazo);
    Carta carta;
    int i;

    /* Cartas +2 */
    for (i = 0; i < CANT_MAS2; i++)
    {
        carta.tipo = MAS2;
        strcpy(carta.nombre, nombreCarta(MAS2));
        agregarCartaMazo(mazo, carta);
    }
    /* Cartas +1 */
    for (i = 0; i < CANT_MAS1; i++)
    {
        carta.tipo = MAS1;
        strcpy(carta.nombre, nombreCarta(MAS1));
        agregarCartaMazo(mazo, carta);
    }
    /* Cartas -1 */
    for (i = 0; i < CANT_MENOS1; i++)
    {
        carta.tipo = MENOS1;
        strcpy(carta.nombre, nombreCarta(MENOS1));
        agregarCartaMazo(mazo, carta);
    }
    /* Cartas -2 */
    for (i = 0; i < CANT_MENOS2; i++)
    {
        carta.tipo = MENOS2;
        strcpy(carta.nombre, nombreCarta(MENOS2));
        agregarCartaMazo(mazo, carta);
    }
    /* Cartas repetir */
    for (i = 0; i < CANT_REPETIR; i++)
    {
        carta.tipo = REPETIR;
        strcpy(carta.nombre, nombreCarta(REPETIR));
        agregarCartaMazo(mazo, carta);
    }
    /* Cartas espejo */
    for (i = 0; i < CANT_ESPEJO; i++)
    {
        carta.tipo = ESPEJO;
        strcpy(carta.nombre, nombreCarta(ESPEJO));
        agregarCartaMazo(mazo, carta);
    }

    mezclarMazo(mazo);
}

void asegurarCartasDisponibles(tMazo* mazo, tMazo* descarte)
{
    /* Si el mazo está vacío, mezclamos las cartas de la pila de descarte y las ponemos en el mazo */
    if (pilaVacia(mazo) == PILA_VACIA)
    {
        /* Cambiar las pilas; la de descarte se convierte en el mazo principal
           y el mazo (vacío) se convierte en la de descarte */
        tMazo temp = *mazo;
        *mazo = *descarte;
        *descarte = temp;
        mezclarMazo(mazo);
    }
}
