
# 🎮 Bienvenidos al juego **DoCe** 🎲

**DoCe** es un juego simple y adictivo desarrollado por el equipo de *CodeInc*, que combina estrategia y azar. Simula una batalla entre un jugador humano y la computadora. El objetivo es alcanzar **12 puntos** antes que tu oponente, usando cartas con distintos efectos.

---

## 🕹️ ¿Cómo jugar al DoCe? ✍️


1. Al comenzar, tenés que ingresar tu nombre y seleccionar una dificultad:

   🔹 Fácil  
   🔹 Media  
   🔹 Difícil

3. Cada jugador comienza con **3 cartas en mano**. El inicio de la partida es aleatorio.

---

## 🧩 Cartas de acción

El mazo contiene **40 cartas**, distribuidas de la siguiente manera:
|  **Carta**        | **Cantidad**   | **Efecto**                                 |
|-------------------|----------------|--------------------------------------------|
|  +1 punto         | 10             | Suma 1 punto al jugador que la usa         |
|  +2 puntos        | 6              | Suma 2 puntos al jugador que la usa        |
|  -1 punto         | 8              | Resta 1 punto al oponente                  |
|  -2 puntos        | 6              | Resta 2 puntos al oponente                 |
|  Repetir turno    | 6              | Permite jugar un turno adicional           |
|  Espejo           | 4              | Refleja un efecto negativo recibido        |

---

## 🧠 ¡A jugar!

- En cada turno, el jugador elige una carta de su mano y la juega. Luego, **roba una carta del mazo** para mantener siempre 3 en mano.  
- Si el mazo se termina y nadie llegó a 12 puntos todavía, se mezclan las cartas usadas y se continúa con ese mazo nuevo.  
- ❗ **Ningún jugador puede tener puntaje negativo**.  
- 🏆 **Gana el primero en llegar a 12 puntos.**

---

## 🗂️ Registros de partida

Al finalizar una partida, se genera automáticamente un archivo 📄 con:

- **El detalle de cada turno** (quién jugó qué carta).  
- **La evolución de los puntajes**.  
- **El ganador**.  

🕒 El nombre del archivo incluye la fecha y hora del juego.

---

## ⚙️ ¿Cómo cambiar las configuraciones del juego?

Para modificar configuraciones como el nombre del jugador, la dificultad o el comportamiento de las cartas:

1. Abrí los archivos de configuración o variables en el código fuente.
2. Realizá los cambios necesarios 🛠️. Hay comentarios para ayudarte a entender el código.
3. Guardá, compilá y ejecutá nuevamente el juego. ▶️

---

## 🧾 Configuración de la API
Las configuraciones de la API se leen del archivo `config.txt` con el siguiente formato:
```nginx
UrlApi | Código identificador del grupo
```
En este caso:
```nginx
https://algoritmos-api.azurewebsites.net/api/doce | interfaz
```

---

## 🚀 ¡Que comience el duelo y que gane el mejor!


