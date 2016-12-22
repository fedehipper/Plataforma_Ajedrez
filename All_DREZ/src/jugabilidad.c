#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include <stdbool.h>
#include "tablero_grafico.h"

#define LADO 8

void mover_pieza_a_destino(char pieza,int fila_origen, int fila_destino, int columna_origen, int columna_destino, char campo[LADO][LADO]) {
	campo[fila_destino][columna_destino] = campo[fila_origen][columna_origen];
	campo[fila_origen][columna_origen] = ' ';
}

bool misma_coordenada(int fila, int columna, int nueva_fila, int nueva_columna) {
	return fila == nueva_fila && columna == nueva_columna;
}

void seleccionar(char campo[LADO][LADO]) {
	int fila = 0, columna = 0, fila_origen = 0, fila_destino = 0, columna_origen = 0, columna_destino = 0, clic_origen = 0, tecla = 0;
	char pieza = ' ';

	while(tecla != KEY_A) {
		rest(75);

		if(clic_origen == 1) {
			fila_origen = fila;
			columna_origen = columna;
		}

		if(clic_origen > 1 && !misma_coordenada(fila_origen, columna_origen, fila, columna)) {
			fila_destino = fila;
			columna_destino = columna;
			mover_pieza_a_destino(pieza, fila_origen, fila_destino, columna_origen, columna_destino, campo);
			draw_selector_cuadrado(fila_destino, columna_destino, campo);
			draw_cuadrado(fila_origen, columna_origen, campo);
			clic_origen = 0;
			re_draw(campo);
			rest(300);
			re_draw(campo);
		}

		if (mouse_b & 1) {
			fila = (mouse_y - 11) / 80;
			columna = (mouse_x - 11) / 80;
			printf("%d %d\n" ,fila,columna);
			draw_selector_cuadrado(fila, columna, campo);
			pieza = campo[fila][columna];
			clic_origen += 1;
		}

		 // fila --> y
		 // columna --> x
		if(keypressed()) tecla = readkey() >> 8;
	}

}
