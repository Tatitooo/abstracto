# Estrategia de Implementación del DoCe

## Estructura del juego

El juego consta de un mazo de 40 cartas, distribuidas entre distintos tipos de efectos. Para representar este mazo se utilizó una pila semi-estática (también llamada híbrida). Esta elección se justifica en que el comportamiento del mazo sigue el principio LIFO (Last In, First Out), ya que las cartas se van tomando desde el tope y también se pueden reinsertar.

Se implementó como una pila de 40 elementos (tamaño fijo), donde cada posición contiene un puntero a una estructura `tDato`. Esta estructura posee un puntero genérico (`void*`) al dato propiamente dicho, junto con su tamaño en bytes (`unsigned tamDato`). Esto permite almacenar cualquier tipo de dato, brindando flexibilidad al diseño.

Además, la pila mantiene un `tope` que actúa como índice al último elemento insertado, facilitando el control de estado (llena/vacía) y las operaciones de inserción/extracción.

```c
// Estructura para datos con tamaño variable (para pila semi-estática)
typedef struct {
    void* dato;
    unsigned tamDato;
} tDato;

// Estructura de pila semi-estática/híbrida
typedef struct {
    tDato* pila[TAM_PILA];
    int tope;
} tPila;
```
Se optó por un diseño modular, con estructuras bien definidas y nombres significativos. Para facilitar la lectura del código y mantener una terminología coherente con el dominio del problema (juego de cartas), se redefinió el nombre de la estructura utilizada.

Esta decisión se alinea con los principios del Clean Code de Robert C. Martin, que promueve el uso de nombres significativos y relacionados con el dominio, facilitando así la comprensión del código por parte de otros desarrolladores (o por uno mismo en el futuro).

Al usar tMazo, el código refleja con claridad que esa estructura representa un mazo de cartas, sin necesidad de inferencias.

```c
// Mazo como alias de pila para mantener terminología del dominio
typedef tPila tMazo;
```
## Cartas

Los elementos que se colocan dentro del mazo son las **cartas**, que son una estructura que contiene un **tipo de carta** (definido previamente con un `enum`) y un **nombre** de la misma. La elección de `enum` permite **claridad y control** sobre los posibles valores de tipo de carta, **reduciendo errores** y **mejorando la legibilidad** del código.

```c
typedef enum {
    MAS2,
    MAS1,
    MENOS1,
    MENOS2,
    REPETIR,
    ESPEJO
} TipoCarta;

typedef struct {
    TipoCarta tipo;
    char nombre[MAX_NOMBRE_CARTA];
} Carta;
```
## Jugadores
Los jugadores se modelaron con una estructura que incluye su nombre, puntos acumulados, mano de hasta 3 cartas, el tipo de jugador (humano o IA en distintos niveles de dificultad) y un indicador booleano para saber si fue el vencedor.
La elección de representar la mano como un array de tres cartas permite un acceso simple y eficiente a las cartas en mano. La inclusión del campo TipoJugador permite modularizar el comportamiento de cada tipo de jugador e implementar diferentes niveles de IA sin complicar la lógica general del juego.

```c
define MAX_NOMBRE_JUGADOR 32

typedef enum {
    HUMANO,
    IA_FACIL,
    IA_MEDIO,
    IA_DIFICIL
} TipoJugador;

typedef struct {
    char nombre[MAX_NOMBRE_JUGADOR];
    int puntos;
    Carta mano[MAX_MANO];
    TipoJugador tipo;
    int esVencedor;
} Jugador;
```

## ¿Cómo se mezclan y roban las cartas?
Para garantizar que siempre haya cartas disponibles para los jugadores, se implementó una función que gestiona la reposición del mazo de juego a partir del mazo de descarte. El proceso se activa cuando el mazo de jugadas (desde el cual se roban las cartas para formar la mano de 3 cartas) queda vacío. En ese caso, no hay otra opción más que reutilizar las cartas del mazo de descarte.
El procedimiento es el siguiente:

1. **Intercambio de pilas**:  
   Si el mazo de jugadas está vacío, se realiza un intercambio entre las dos pilas. El mazo de descarte pasa a convertirse en el nuevo mazo de jugadas, mientras que el mazo original de jugadas (vacío) pasa a ocupar el lugar del descarte.

2. **Mezcla de cartas**:  
   Una vez hecho el intercambio, se procede a mezclar el nuevo mazo de jugadas (que antes era el de descarte). Para ello, primero se verifica que haya más de una carta disponible (de lo contrario, no tiene sentido mezclar). Luego, todas las cartas se extraen de la pila utilizando una función de desapilado, y se almacenan en un arreglo dinámico.

3. **Uso de memoria dinámica**:  
   Dado que en ese momento no se conoce cuántas cartas hay exactamente en el mazo, es necesario recurrir a memoria dinámica para crear el arreglo que las almacene temporalmente. Esto proporciona flexibilidad y eficiencia.

4. **Algoritmo de mezcla (Fisher-Yates)**:  
   Con todas las cartas en un arreglo, se aplica el algoritmo de Fisher-Yates, que permite barajar de forma eficiente.  
   Este algoritmo recorre el arreglo desde el principio hasta el penúltimo elemento. En cada iteración i, se elige un índice aleatorio j entre 0 e i inclusive, y se intercambian las posiciones i y j.  
   Este método simula el proceso físico de “sacar cartas al azar” pero lo hace en tiempo lineal, sin necesidad de usar memoria adicional.

5. **Reconstrucción del mazo**:  
   Una vez mezcladas, las cartas se reinsertan una a una en el mazo utilizando la función de apilado. Finalmente, se libera la memoria utilizada por el arreglo dinámico.

Esta estrategia es esencial para la dinámica del juego, ya que asegura que siempre se pueda continuar robando cartas sin interrupciones, y la aleatorización garantiza que el juego sea justo e impredecible en cada ronda.

## Estrategia del juego
La estrategia general consiste en que cada jugador utiliza las cartas de su mano para afectar su puntaje o el de su oponente, según el efecto de la carta.  
Las cartas pueden:

- Sumar o restar puntos,
- Repetir el turno,
- Anular efectos negativos y devolverlos.

Por lo tanto, las decisiones deben basarse tanto en el estado actual como en la predicción de los movimientos del rival.

El jugador humano compite contra una IA cuya dificultad define el nivel de estrategia aplicada:

- **IA_FACIL**: realiza elecciones aleatorias simples.
- **IA_DIFICIL**: evalúa el estado del juego para tomar decisiones más complejas, como priorizar cartas defensivas o maximizar daño al oponente.
Cada partida está compuesta por los dos jugadores (humano e IA), un mazo principal, un mazo de descarte, un historial de jugadas, una bandera que indica el turno actual (0 para el jugador humano y 1 para la IA), una bandera de dificultad, y una carta para registrar la última jugada de cada participante. Todo esto se encapsula en la siguiente estructura:

```c
typedef struct {
    Jugador jugador;
    Jugador ia;
    tMazo mazo;
    tMazo descarte;
    HistorialJugada* historial;
    int turnoActual; // 0: jugador, 1: IA
    int dificultad;
    Carta ultimaCartaJugador;
    Carta ultimaCartaIA;
} Partida;
```
Cuando se crea una nueva partida, se inicializan todos los campos. En particular, el puntero al historial de jugadas se establece en NULL, lo cual es fundamental para reservar espacio y preparar el entorno correctamente antes de comenzar el juego. Esta inicialización explícita sigue buenas prácticas de diseño (Clean Code) al mantener el estado del sistema predecible desde el inicio.

### Desarrollo de la partida 

Al iniciar la partida, se despliega la interfaz gráfica, se reproduce música de fondo y comienza el bucle principal del juego. Este bucle representa una mecánica por turnos entre el jugador humano y la IA.

En cada iteración del ciclo:

1. **Determinación del turno**:  
   Se verifica a quién le corresponde jugar (`turnoActual`). Esta información también se muestra visualmente para mejorar la experiencia del usuario.

2. **Selección de carta**:  
   - Si el turno corresponde al jugador humano, se espera que seleccione una carta desde la interfaz gráfica. Una función especializada devuelve el índice de la carta seleccionada, o bien detecta si el usuario eligió salir del juego.  
   - Si el turno corresponde a la IA, se invoca una función automatizada que selecciona una carta según la dificultad actual.

3. **Registro de jugadas**:  
   Se guarda la última carta jugada por cada jugador. Si el turno se repite (por efecto de una carta *Repetir turno*), se considera ese estado para no alternar el turno prematuramente y reflejar correctamente la jugada en la interfaz.

4. **Aplicación del efecto**:  
   Una vez elegida la carta, se ejecuta su efecto. Este puede alterar el flujo del juego (como permitir repetir el turno o activar un efecto espejo). Los efectos se gestionan mediante punteros a variables que indican si se debe repetir o revertir el turno.

5. **Actualización del historial**:  
   Se agrega la jugada al historial, incluyendo efectos especiales como el espejo, que puede anular el efecto anterior y devolver el turno al jugador que la activó.

6. **Descartar y reponer**:  
   La carta jugada se descarta (es decir, se añade al mazo de descarte mediante una función que encapsula esta acción). Luego, se repone la mano robando una nueva carta del mazo principal.

### Control de finalización 

Después de cada turno, se comprueba si el jugador de ese turno alcanzó los 12 puntos, condición necesaria para finalizar la partida. En caso contrario, se alterna el turno usando una lógica simple basada en los valores 0 y 1.

El ciclo continúa hasta que uno de los jugadores obtiene la victoria. Al final de la partida:

- Se determina si el jugador resultó vencedor.
- Se activa la interfaz final correspondiente, junto con efectos de sonido.
- Se muestran los resultados.
- Se envían los datos de la partida a una API para su registro.

## Efectos de las cartas y uso del efecto espejo
Cada carta jugada puede tener un efecto sobre el puntaje del jugador propio o del oponente. Estos efectos están implementados dentro de una función que, aunque no devuelve un valor explícito, modifica directamente los datos de los jugadores a través de punteros. Esto permite actualizar el estado del juego en tiempo real, sin necesidad de retornar estructuras completas.

### Aplicación de efectos

Al ejecutarse esta función, lo primero que se hace es obtener la carta que fue jugada, accediendo a la posición correspondiente en la mano del jugador actual.

La lógica para aplicar los efectos está estructurada con un `switch` que evalúa el tipo de carta jugada, el cual está definido como un `enum`. Según el caso, se suman o restan puntos a los jugadores. También puede activarse un efecto especial, como repetir el turno, lo que se indica mediante una flag (bandera) que se actualiza a 1 y se propaga hacia la función principal del ciclo de juego.

### Efecto espejo

Una carta especial dentro del juego es la carta espejo, cuyo propósito es anular el efecto negativo de una carta jugada por el oponente. Esta carta solo tiene sentido si la carta rival es negativa (por ejemplo, resta 1 o 2 puntos).

El sistema está diseñado para que:

- Si la carta jugada por el rival tiene un efecto negativo (-1 o -2), se verifica si el jugador actual (humano o IA) posee una carta espejo en su mano.
- Si es así, se consulta si desea usarla (en el caso del jugador humano mediante la interfaz, y en el caso de la IA mediante una función automatizada).

Una vez tomada la decisión:

- Se modifica una variable por referencia para indicar que se usó el efecto espejo, de modo que esta información se pueda registrar en el historial de jugadas.
- Luego, el efecto negativo se invierte, haciendo que el daño sea devuelto al oponente, y no aplicado al jugador que recibió el ataque.
- Se registra también cuál carta espejo fue utilizada y por quién, para mantener el estado del juego correctamente actualizado.

### Finalización del efecto

Una vez aplicado el efecto (tanto normal como espejo):

- La carta jugada (ya sea una carta ofensiva o una carta espejo) se descarta, es decir, se coloca en el mazo de descarte.
- Luego, el jugador repone su mano robando una carta del mazo principal.

Este diseño permite manejar con claridad tanto los efectos simples como los complejos, como el caso del espejo, manteniendo el estado del juego actualizado y registrando cada evento relevante en el historial.

## Dificultades del juego y comportamiento de la IA

La inteligencia artificial (IA) del juego está diseñada para adaptarse a distintos niveles de dificultad, afectando directamente la forma en que elige sus jugadas. Esto se implementa a través de la función `elegirCartaIA`, que analiza la mano de la IA y retorna el índice de la carta más adecuada, en función del estado del juego y del nivel de dificultad elegido.

El algoritmo recorre la mano, calcula la prioridad de cada carta, y selecciona aquella con el valor más alto, devolviendo su índice dentro del arreglo de cartas, y no el tipo de carta directamente.

### Dificultad FÁCIL

En este nivel, la IA no aplica ninguna lógica estratégica. Simplemente selecciona una carta de manera aleatoria. Esto simula un comportamiento impredecible, ideal para jugadores principiantes.

### Dificultad MEDIA

En esta dificultad, la IA toma decisiones más razonadas, evaluando el estado del juego y asignando prioridades a las cartas según su contexto. Las reglas principales son:

- Si el jugador humano tiene 0 puntos, evita usar cartas que resten puntos (porque el puntaje no puede ser negativo).
- De la misma manera, si el jugador humano tiene 1 punto, preferirá no usar la carta de -2 puntos.
- Si la IA tiene 8 o más puntos, prioriza cartas que sumen puntos (+1, +2).

Para cada carta de la mano, se calcula una prioridad de uso:

- Prioridad 3: muy alta (por ejemplo, usar un +2 si la IA tiene 9 puntos).
- Prioridad 2: normal (cartas neutrales o sin contexto crítico).
- Prioridad 1: baja (por ejemplo, restar 2 puntos si el jugador tiene solo 1).
- Prioridad 0: muy baja o sin utilidad (como restar puntos a un jugador con 0).

Una vez evaluadas todas las cartas, la IA selecciona la carta con mayor prioridad. En caso de empate, elige la primera de su mano que cumpla con ese criterio.

### Dificultad DIFÍCIL

Este nivel aplica todas las reglas de dificultad media, pero además añade un análisis más profundo y reactivo:

- Evalúa la mano completa antes de decidir si repetir el turno: solo repite si tiene más de una carta "buena" (definidas como +1, +2 o espejo).
- Usa el efecto espejo automáticamente cuando recibe una carta negativa, sin azar.
- Prioriza:
  - Cartas +2 o +1 cuando la IA está cerca de ganar.
  - Cartas que restan puntos y de repetir turno si el jugador humano está cerca de los 12 puntos.
  - Repetir el turno si el contexto lo justifica (presencia de buenas cartas).

El sistema de prioridad en dificultad difícil va de 0 a 8, permitiendo una mayor granularidad:

- Prioridades 7-8: muy alta (+1 o +2 cuando la IA está cerca de ganar).
- 5-6: alta (-1, -2 o repetir cuando el humano está cerca de ganar).
- 4: preferida (repetir cuando hay más de una carta buena en mano).
- 3: normal
- 2: baja (como tirar -2 cuando el jugador tiene 1 punto).
- 1 o menos: muy baja o último recurso (como restar puntos cuando el jugador tiene 0, o jugar espejo).
