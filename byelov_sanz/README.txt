/*_________________________________________________________________________________________________
PRACTICA 2

SQUAD: Ctrl Unit

AUTORES:
         Nicolás Sanz Tuñón
         Constantino Byelov Serdiuk
___________________________________________________________________________________________________
*/

Mejoras relizadas:
	- Hemos realizado el cd avanzado, el cual deja redireccionar a carpetas o directorios con espacios en ellos, 
	  la redireccion se puede hacer con los caracteres especiales "" (dobles comillas), ''(comillas simples) y \ (barra invertida).
	
	- Prompt personalizado, en la parte de arriba tenemos un define con el prompt personal, si se pone a 1 saldra el USER enviroment 
	  con la extension fija de MINISHELL, en cambio si se pone a 0, el prompt saldrá con el USER enviroment además de la ruta en la 
	  cual este en ese momento, nos ha parecido util al momento de hacer el comando cd, para siempre saber en donde estamos y no tener 
	  que usar el comandp pwd.

	- Hemos cambiado el código del GRIS_T a "\x1b[90m" debido a que antes al usarlo el mensaje salia azul en vez de gris.


Restricciones del programa:
	- Las intrucciones en background solo permiten comantarios tras el carácter &.


