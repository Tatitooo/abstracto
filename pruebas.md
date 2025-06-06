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

## Caso de Prueba 7: Visualización de Puntuación Parcial Durante la Partida
**Descripción:** Se quiere probar si durante el desarrollo de la partida, el sistema muestra correctamente la puntuación parcial de ambos jugadores después de cada turno.<br />
**Salida esperada:** Se espera que el sistema actualice y muestre en pantalla la puntuación parcial del jugador y de la IA tras cada turno.<br />
**Salida obtenida:** La salida obtenida fue la visualización correcta de la puntuación parcial de ambos jugadores después de cada turno.<br />
![image](https://github.com/user-attachments/assets/a4958392-fd84-4af7-ad14-7fd4738f0722)<br />

## Caso de Prueba 8: Usar Espejo Después de Rechazar
**Descripción:** Se quiere probar qué ocurre si el jugador decide que no quiere usar un espejo como respuesta a un ataque.<br />
**Salida esperada:** Se espera que el jugador no pueda jugar el espejo desde su mano después.<br />
**Salida obtenida:** El jugador no puede jugar el espejo por más que le haga clic.<br />
![image](https://github.com/user-attachments/assets/788502db-54d0-4955-bd08-99261482e0f9)<br />
*Por más que se resalte la carta, el usuario no puede jugarla ya que decidió rechazar el espejo.*


## Caso de Prueba 9: Tres Espejos
**Descripción:** Se quiere probar qué ocurre si en algún punto de la partida el jugador tiene 3 espejos y decide no usar espejo como respuesta a una reducción de puntos.<br />
**Salida esperada:** Se espera que el sistema prevenga este *softlock* prohibiendo que el jugador decida no usar espejo si sus tres cartas son del tipo Espejo.<br />
**Salida obtenida:** El espejo se utiliza automáticamente si el jugador tiene 3 Espejos.<br />

![image](https://github.com/user-attachments/assets/cd994b8d-884f-4204-81cf-63362dc2df38)<br />
![image](https://github.com/user-attachments/assets/9b74b00d-0027-4b44-85c5-be409b32d3ff)<br />

## Caso de Prueba 10: Restar con Pocos Puntos
**Descripción:** Se quiere probar qué ocurre si el jugador le intenta restar puntos a un rival con 0 puntos (o 1 punto en el caso de usar -2).<br />
**Salida esperada:** Se espera que el rival se quede con 0 puntos, en vez de ir a lo negativo. <br />
**Salida obtenida:** El rival se queda con 0 puntos.<br />
![image](https://github.com/user-attachments/assets/7dc29384-8da5-4608-9dd7-9c04a0177073)<br />
![image](https://github.com/user-attachments/assets/0a8653b5-b836-46a2-9787-5495e8446b24)<br />

## Caso de Prueba 11: Usar Espejo sin Carta Negativa
**Descripción:** Se quiere probar qué ocurre si un jugador utiliza la carta espejo después de que su rival juegue una carta que no sea de restar puntos.<br />
**Salida esperada:** La carta espejo no tiene efecto.<br />
**Salida obtenida:** La carta espejo no tiene efecto.<br />
![image](https://github.com/user-attachments/assets/1f3c9413-6aa2-4391-bf65-bb065ce86f64)<br />
![image](https://github.com/user-attachments/assets/920b5d4b-14c8-4e86-9bca-de8a711737c6)<br />
