#include <stdbool.h>
#include "movimientos_peon.h"
#include "movimientos_reina.h"
#include "movimientos_rey.h"
#include "movimientos_torre.h"
#include "movimientos_alfil.h"

#define LADO 8

bool es_amigo_de_caballo(char rey, char otro) {
	return (rey == 'c')? otro == 'w' || otro == 'a' || otro == 'r' || otro == 't' || otro == 'c' || otro == 'p':
			otro == 'W' || otro == 'A' || otro == 'R' || otro == 'T' || otro == 'C' || otro == 'P';
}

bool movimiento_permitido_caballo(int fila_origen, int columna_origen, int fila_destino, int columna_destino, char campo[LADO][LADO]) {
	return ((fila_origen + 2 == fila_destino && columna_origen - 1 == columna_destino) ||
		    (fila_origen - 2 == fila_destino && columna_origen + 1 == columna_destino) ||
		    (columna_origen + 2 == columna_destino && fila_origen - 1 == fila_destino) ||
		    (columna_origen - 2 == columna_destino && fila_origen + 1 == fila_destino) ||
		    (fila_origen + 2 == fila_destino && columna_origen + 1 == columna_destino) ||
		    (fila_origen - 2 == fila_destino && columna_origen - 1 == columna_destino) ||
			(columna_origen + 2 == columna_destino && fila_origen + 1 == fila_destino) ||
			(columna_origen - 2 == columna_destino && fila_origen - 1 == fila_destino)) &&
			!es_amigo_de_caballo(campo[fila_origen][columna_origen], campo[fila_destino][columna_destino]);
}


bool es_jaque_caballo(char pieza, int fila_destino, int columna_destino, int f_rey_b, int c_rey_b, int f_rey_n, int c_rey_n, char campo[LADO][LADO]) {
	bool es_jaque = false;
		if(pieza == 'c' && movimiento_permitido_caballo(fila_destino, columna_destino, f_rey_n, c_rey_n, campo)) {
			es_jaque = true;
		}

		if(pieza == 'C' && movimiento_permitido_caballo(fila_destino, columna_destino, f_rey_b, c_rey_b, campo)) {
			es_jaque = true;
		}
	return es_jaque;
}

bool si_se_mueve_es_jaque_caballo(int fila, int columna, char campo[LADO][LADO], int f_rey_b, int c_rey_b, int f_rey_n, int c_rey_n) {
	int i,j;
	char pieza = campo[fila][columna];
	bool es_jaque = false;
	// hago que desaparezca
	campo[fila][columna] = ' ';
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			if(!es_amigo_de_caballo(pieza, campo[i][j])) {
				switch(campo[i][j]) {
					case 'p': if(es_jaque_peon('p', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'P': if(es_jaque_peon('P', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'a': if(es_jaque_alfil('a', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n,campo)) es_jaque = true;
					break;
					case 'A': if(es_jaque_alfil('A', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n,campo)) es_jaque = true;
					break;
					case 't': if(es_jaque_torre('t', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'T': if(es_jaque_torre('T', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'c': if(es_jaque_caballo('c', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'C': if(es_jaque_caballo('C', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'w': if(es_jaque_reina('w', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
					case 'W': if(es_jaque_reina('W', i, j, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo)) es_jaque = true;
					break;
				}
				if (es_jaque) break;
			}
		}
	}
	// hago que aparezca
	campo[fila][columna] = pieza;
	return es_jaque;
}
