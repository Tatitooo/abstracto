# Pruebas del Proyecto

## Caso de Prueba 1: Ingreso de Nombre.
**Descripción:** Se quiere probar qué es lo que pasaría si un usuario intenta ingresar un nombre inválido, en este caso de menos de 3 carácteres.<br />
**Salida esperada:** Se espera que el sistema muestre un mensaje de error indicando que el nombre debe tener al menos 3 carácteres.<br />
**Salida obtenida:** La salida obtenida fue un mensaje de error que indicaba "El nombre debe tener al menos 3 carácteres".<br />
![image](https://github.com/user-attachments/assets/0182eb2e-7539-4e75-9a61-6546e7fc67b2)<br />

## Caso de Prueba 2: Generación de Informe de Partida.
**Descripción:** Se quiere probar qué es lo que pasaría si se intenta generar un informe de partida con datos de la partida.<br />
**Salida esperada:** Se espera que el sistema genere un archivo de texto con el informe de la partida, incluyendo detalles como el nombre del jugador, el nombre de la IA, la dificultad, los turnos jugados, y el resultado final.<br />
**Salida obtenida:** La salida obtenida fue un archivo de texto generado con el nombre informe-juego_YYYY-MM-DD_HH-MM-SS.txt.<br />
![image](https://github.com/user-attachments/assets/32caea49-3dda-4241-8da3-fd153b4b247f)<br />

## Caso de Prueba 3: Turnos de la Partida.
**Descripción:** Se quiere probar si tras finalizar una partida, se aprecian los turnos de la misma, qué cartas se usan y .<br />
**Salida esperada:** Se espera que el sistema genere un archivo de texto del informe de la partida, en el cual se muestren qué cartas utilizó el jugador o IA en sus determinados turnos.<br />
**Salida obtenida:** La salida obtenida fue el archivo de texto donde se puede apreciar los turnos de cada carta<br />
![image](https://github.com/user-attachments/assets/e836759d-b573-4dbb-a882-e35ac506d2a1)<br />

## Caso de Prueba 4: Datos Informe sin terminar la Partida.
**Descripción:** Se quiere probar qué es lo que pasaría con los datos del informe si la partida no fue finalizada al alcanzar 12 puntos.<br />
**Salida esperada:** Se espera que el sistema genere un archivo de texto con el informe de la partida, en el cual se muestre por defecto a la IA como ganador.<br />
**Salida obtenida:** La salida obtenida fue el informe donde se muestra a la IA como ganador.<br />
![image](https://github.com/user-attachments/assets/9c2971d4-7560-432c-b6aa-4f5b877af73a)<br />

## Caso de Prueba 5: Reinicio de Partida tras Finalizar.
**Descripción:** Se quiere probar si el sistema permite iniciar una nueva partida correctamente después de finalizar una anterior.<br />
**Salida esperada:** Se espera que el sistema reinicie todos los valores y permita comenzar una nueva partida sin errores.<br />
**Salida obtenida:** La salida obtenida fue el inicio de una nueva partida con valores reiniciados.<br />
![image](https://github.com/user-attachments/assets/ec6738db-f3f8-4518-8030-5c74cc42bdae)<br />
![image](https://github.com/user-attachments/assets/99ac7331-06ec-44ae-85f2-82bb752b443a)<br />

## Caso de Prueba 6: Visualización de Mensaje de Fin de Partida
**Descripción:** Se quiere probar si al finalizar la partida, el sistema muestra un mensaje claro indicando el final del juego y quién fue el ganador.<br />
**Salida esperada:** Se espera que el sistema muestre un mensaje de fin de partida y el nombre del ganador.<br />
**Salida obtenida:** La salida obtenida fue un mensaje indicando el fin de la partida y el ganador.<br />
![image](https://github.com/user-attachments/assets/05bf64e6-46dd-4e10-aa13-1ea0743a47c5)<br />

## Caso de Prueba 7: Visualización de Turno Actual
**Descripción:** Se quiere probar si durante la partida, el sistema muestra correctamente de quién es el turno (jugador o IA).<br />
**Salida esperada:** Se espera que el sistema indique en pantalla el turno actual.<br />
**Salida obtenida:** La salida obtenida fue la visualización del turno actual correctamente.<br />
![image](https://github.com/user-attachments/assets/168edb87-deb9-4e86-b31a-7f4715449fec)<br />
![image](https://github.com/user-attachments/assets/ce73505e-d470-42c8-a2bb-aff8b38064fa)<br />

## Caso de Prueba 8: Visualización de Puntuación Parcial Durante la Partida
**Descripción:** Se quiere probar si durante el desarrollo de la partida, el sistema muestra correctamente la puntuación parcial de ambos jugadores después de cada turno.<br />
**Salida esperada:** Se espera que el sistema actualice y muestre en pantalla la puntuación parcial del jugador y de la IA tras cada turno.<br />
**Salida obtenida:** La salida obtenida fue la visualización correcta de la puntuación parcial de ambos jugadores después de cada turno.<br />
![image](https://github.com/user-attachments/assets/a4958392-fd84-4af7-ad14-7fd4738f0722)<br />
