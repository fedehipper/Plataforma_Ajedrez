#include <stdbool.h>

#define LADO 8

bool es_amigo_de_caballo(char rey, char otro) {
	if(rey == 'c') {
		return otro == 'w' || otro == 'a' || otro == 'r' || otro == 't' || otro == 'c' || otro == 'p';
	} else {
		return otro == 'W' || otro == 'A' || otro == 'R' || otro == 'T' || otro == 'C' || otro == 'P';
	}
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
