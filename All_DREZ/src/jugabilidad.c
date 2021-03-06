#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "tablero_grafico.h"
#include "movimientos_peon.h"
#include "movimientos_torre.h"
#include "movimientos_rey.h"
#include "movimientos_alfil.h"
#include "movimientos_reina.h"
#include "movimientos_caballo.h"
#include "lib_socket.h"

#define LADO 8
#define OSCURO 6
#define CLARO 90
#define SELECCION_CLARO 69
#define SELECCION_OSCURO 72
#define PARAR_CRONOMETRO 1000
#define TAMANIO_STREAM 17
#define COLOR_JAQUE 39

int f_rey_b = 7, c_rey_b = 4, f_rey_n = 0, c_rey_n = 4,
	f_origen_anterior = -1, c_origen_anterior = -1, f_destino_anterior = -1, c_destino_anterior = -1,
	minuto_b = 5, segundo_b = 0, milesima_segundo_b = 0, minuto_n = 5, segundo_n = 0, milesima_segundo_n = 0;

bool movi_torre_izq_b = false, movi_torre_der_b = false, movi_torre_izq_n = false, movi_torre_der_n = false, movi_rey_b = false,
	 movi_rey_n = false;


bool es_pieza_blanca(char pieza) {
	return pieza == 'w' || pieza == 'a' || pieza == 'r' || pieza == 't' || pieza == 'c' || pieza == 'p';
}

bool verificar_jaque_intermedio_blancas(char campo[LADO][LADO]) {
	int i, j;
	bool es_jaque = false;
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			if(campo[i][j] != ' ' && es_pieza_blanca(campo[i][j])) {
				switch(campo[i][j]) {
					case 'a': es_jaque = movimiento_permitido_alfil(i, j, f_rey_n , c_rey_n, campo); break;
					case 'p': es_jaque = movimiento_permitido_peon(i, j, f_rey_n, c_rey_n, campo); break;
					case 'c': es_jaque = movimiento_permitido_caballo(i, j, f_rey_n, c_rey_n, campo); break;
					case 'w': es_jaque = movimiento_permitido_reina(i, j, f_rey_n, c_rey_n, campo); break;
					case 't': es_jaque = movimiento_permitido_torre(i, j, f_rey_n, c_rey_n, campo); break;
					case 'r': es_jaque = movimiento_permitido_rey(i, j, f_rey_n, c_rey_n, campo); break;
				}
			}
			if(es_jaque) break;
		}
		if(es_jaque) break;
	}
	return es_jaque;
}

bool verificar_jaque_intermedio_negras(char campo[LADO][LADO]) {
	int i, j;
	bool es_jaque = false;
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			if(campo[i][j] != ' ' && !es_pieza_blanca(campo[i][j])) {
				switch(campo[i][j]) {
					case 'A': es_jaque = movimiento_permitido_alfil(i, j, f_rey_b , c_rey_b, campo); break;
					case 'P': es_jaque = movimiento_permitido_peon(i, j, f_rey_b , c_rey_b, campo); break;
					case 'C': es_jaque = movimiento_permitido_caballo(i, j, f_rey_b , c_rey_b, campo); break;
					case 'W': es_jaque = movimiento_permitido_reina(i, j, f_rey_b , c_rey_b, campo); break;
					case 'T': es_jaque = movimiento_permitido_torre(i, j, f_rey_b , c_rey_b, campo); break;
					case 'R': es_jaque = movimiento_permitido_rey(i, j, f_rey_b, c_rey_b, campo); break;
				}
			}
			if(es_jaque) break;
		}
		if(es_jaque) break;
	}
	return es_jaque;
}

bool condicion_enroque_izq_blanco(char campo[LADO][LADO]) {
	return !movi_torre_izq_b && !movi_rey_b && campo[7][1] == ' ' && campo[7][2] == ' ' && campo[7][3] == ' ' && campo[7][0] == 't';
}

bool condicion_enroque_der_blanco(char campo[LADO][LADO]) {
	return !movi_torre_der_b && !movi_rey_b && campo[7][5] == ' ' && campo[7][6] == ' ' && campo[7][7] == 't';
}

bool condicion_enroque_izq_negro(char campo[LADO][LADO]) {
	return !movi_torre_izq_n && !movi_rey_n && campo[0][1] == ' ' && campo[0][2] == ' ' && campo[0][3] == ' ' && campo[0][0] == 'T';
}

bool condicion_enroque_der_negro(char campo[LADO][LADO]) {
	return !movi_torre_der_n && !movi_rey_n && campo[0][5] == ' ' && campo[0][6] == ' ' && campo[0][7] == 'T';
}

void hacer_enroque_izq_blanco(char campo[LADO][LADO]) {
	campo[7][0] = ' ';
	campo[7][3] = 't';
}

void hacer_enroque_der_blanco(char campo[LADO][LADO]) {
	campo[7][7] = ' ';
	campo[7][5] = 't';
}

void hacer_enroque_izq_negro(char campo[LADO][LADO]) {
	campo[0][0] = ' ';
	campo[0][3] = 'T';
}

void hacer_enroque_der_negro(char campo[LADO][LADO]) {
	campo[0][7] = ' ';
	campo[0][5] = 'T';
}

void aplicar_movimiento(int fila_origen, int columna_origen, int fila_destino, int columna_destino, char campo[LADO][LADO]) {
	if(fila_origen == 0 && columna_origen == 0) movi_torre_izq_n = true;
	if(fila_origen == 0 && columna_origen == 7) movi_torre_der_n = true;
	if(fila_origen == 7 && columna_origen == 0) movi_torre_izq_b = true;
	if(fila_origen == 7 && columna_origen == 7) movi_torre_der_b = true;

	char pieza_origen = campo[fila_origen][columna_origen];

	if(condicion_enroque_der_blanco(campo) && pieza_origen == 'r' && fila_destino == 7 && columna_destino == 6) {
		hacer_enroque_der_blanco(campo);
		movi_rey_b = true;
		movi_torre_der_b = true;
	}
	if(condicion_enroque_izq_blanco(campo) && pieza_origen == 'r' && fila_destino == 7 && columna_destino == 2) {
		hacer_enroque_izq_blanco(campo);
		movi_rey_b = true;
		movi_torre_izq_b = true;
	}
	if(condicion_enroque_der_negro(campo) && pieza_origen == 'R' && fila_destino == 0 && columna_destino == 6) {
		hacer_enroque_der_negro(campo);
		movi_rey_n = true;
		movi_torre_der_n = true;
	}
	if(condicion_enroque_izq_negro(campo) && pieza_origen == 'R' && fila_destino == 0 && columna_destino == 2) {
		hacer_enroque_izq_negro(campo);
		movi_rey_n = true;
		movi_torre_izq_n = true;
	}

	if(fila_origen == 7 && columna_origen == 4) movi_rey_b = true;
	if(fila_origen == 0 && columna_origen == 4) movi_rey_n = true;

	campo[fila_destino][columna_destino] = pieza_origen;
	campo[fila_origen][columna_origen] = ' ';
}

bool movimiento_enroque_izq_blanco(int fila_origen, int columna_origen, int fila_destino, int columna_destino) {
	return fila_origen == 7 && columna_origen == 4 && fila_destino == 7 && columna_destino == 2;
}

bool movimiento_enroque_der_blanco(int fila_origen, int columna_origen, int fila_destino, int columna_destino) {
	return fila_origen == 7 && columna_origen == 4 && fila_destino == 7 && columna_destino == 6;
}

bool movimiento_enroque_izq_negro(int fila_origen, int columna_origen, int fila_destino, int columna_destino) {
	return fila_origen == 0 && columna_origen == 4 && fila_destino == 0 && columna_destino == 2;
}

bool movimiento_enroque_der_negro(int fila_origen, int columna_origen, int fila_destino, int columna_destino) {
	return fila_origen == 0 && columna_origen == 4 && fila_destino == 0 && columna_destino == 6;
}

bool mover_pieza_a_destino(int fila_origen, int fila_destino, int columna_origen, int columna_destino, char campo[LADO][LADO], bool *pieza_esta_en_jaque) {
	bool mover = false, enroque_der_blanca = false, enroque_izq_blanca = false, enroque_der_negra = false, enroque_izq_negra = false;

	if(campo[fila_origen][columna_origen] != ' ' && columna_destino != 8 && fila_destino != 8 && columna_origen != 8 && fila_origen != 8) {
		switch(campo[fila_origen][columna_origen]) {
			case 'p':
			case 'P':
				if(movimiento_permitido_peon(fila_origen, columna_origen, fila_destino, columna_destino, campo))
					mover = !si_se_mueve_es_jaque_peon(fila_origen, columna_origen, fila_destino, columna_destino, campo, f_rey_b, c_rey_b, f_rey_n, c_rey_n);
			break;
			case 'T':
			case 't':
				if(movimiento_permitido_torre(fila_origen, columna_origen, fila_destino, columna_destino, campo))
					mover = !si_se_mueve_es_jaque_torre(fila_origen, columna_origen, fila_destino, columna_destino,  campo, f_rey_b, c_rey_b, f_rey_n, c_rey_n);
			break;
			case 'r':
				enroque_der_blanca = movimiento_enroque_der_blanco(fila_origen, columna_origen, fila_destino, columna_destino);
				enroque_izq_blanca = movimiento_enroque_izq_blanco(fila_origen, columna_origen, fila_destino, columna_destino);
				if(enroque_der_blanca) mover = condicion_enroque_der_blanco(campo);
				if(enroque_izq_blanca) mover = condicion_enroque_izq_blanco(campo);
				if(!enroque_der_blanca && !enroque_izq_blanca) mover = movimiento_permitido_rey(fila_origen, columna_origen, fila_destino, columna_destino, campo);
			break;
			case 'R':
				enroque_der_negra = movimiento_enroque_der_negro(fila_origen, columna_origen, fila_destino, columna_destino);
				enroque_izq_negra = movimiento_enroque_izq_negro(fila_origen, columna_origen, fila_destino, columna_destino);
				if(enroque_der_negra) mover = condicion_enroque_der_negro(campo);
				if(enroque_izq_negra) mover = condicion_enroque_izq_negro(campo);
				if(!enroque_der_negra && !enroque_izq_negra) mover = movimiento_permitido_rey(fila_origen, columna_origen, fila_destino, columna_destino, campo);
			break;
			case 'a':
			case 'A':
				if(movimiento_permitido_alfil(fila_origen, columna_origen, fila_destino, columna_destino, campo))
					mover = !si_se_mueve_es_jaque_alfil(fila_origen, columna_origen, fila_destino, columna_destino,  campo, f_rey_b, c_rey_b, f_rey_n, c_rey_n);
			break;
			case 'w':
			case 'W':
				if(movimiento_permitido_reina(fila_origen, columna_origen, fila_destino, columna_destino, campo))
					mover = !si_se_mueve_es_jaque_reina(fila_origen, columna_origen, fila_destino, columna_destino,  campo, f_rey_b, c_rey_b, f_rey_n, c_rey_n);
			break;
			case 'c':
			case 'C':
				if(movimiento_permitido_caballo(fila_origen, columna_origen, fila_destino, columna_destino, campo))
					mover = !si_se_mueve_es_jaque_caballo(fila_origen, columna_origen, fila_destino, columna_destino,  campo, f_rey_b, c_rey_b, f_rey_n, c_rey_n);
			break;
		}
		if(mover && pieza_esta_en_jaque != NULL && *pieza_esta_en_jaque) *pieza_esta_en_jaque = false;
	}
	return mover;
}

bool misma_coordenada(int fila, int columna, int nueva_fila, int nueva_columna) {
	return fila == nueva_fila && columna == nueva_columna;
}

char obtener_pieza(int fila, int columna, char campo[LADO][LADO]) {
	return (fila != 8 && columna != 8) ? campo[fila][columna] : ' ';
}

bool hay_pieza(int fila, int columna, char campo[LADO][LADO]) {
	return fila != 8 && columna != 8 && campo[fila][columna] != ' ';
}


bool verificar_jaque(char pieza, int fila_destino, int columna_destino, char campo[LADO][LADO]) {
	bool es_jaque = es_jaque_rey(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo) ||
					es_jaque_alfil(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo) ||
					es_jaque_peon(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo) ||
					es_jaque_torre(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo) ||
					es_jaque_caballo(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo) ||
					es_jaque_reina(pieza, fila_destino, columna_destino, f_rey_b, c_rey_b, f_rey_n, c_rey_n, campo);
	return es_jaque;
}

volatile int close_button_pressed = false;

void close_button_handler(void) {
	close_button_pressed = true;
}

void seleccionar_origen(int fila, int columna, bool turno_blanca, int *clic_blanca, int *clic_negra, int *fila_origen, int *columna_origen) {
	if(turno_blanca) {
		if(*clic_blanca == 0) {
			*fila_origen = fila;
			*columna_origen = columna;
			*clic_blanca = 1;
		}
	} else {
		if(*clic_negra == 0) {
			*fila_origen = fila;
			*columna_origen = columna;
			*clic_negra = 1;
		}
	}
}



bool hay_movimiento_permitido_blancas(char campo[LADO][LADO]) {
	int i, j, k, l;
	bool aux = false, se_puede_mover = false;
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			if(campo[i][j] != ' ' && !es_pieza_blanca(campo[i][j])) {
				for(k = 0 ; k < LADO ; k++) {
					for(l = 0 ; l < LADO ; l++) {
						se_puede_mover = mover_pieza_a_destino(i, k, j, l, campo, &aux);
						if(se_puede_mover) break;
					}
					if(se_puede_mover) break;
				}
				if(se_puede_mover) break;
			}
			if(se_puede_mover) break;
		}
		if(se_puede_mover) break;
	}
	return se_puede_mover;
}

bool hay_movimiento_permitido_negras(char campo[LADO][LADO]) {
	int i, j, k, l;
	bool aux = false, se_puede_mover = false;
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			if(campo[i][j] != ' ' && es_pieza_blanca(campo[i][j])) {
				for(k = 0 ; k < LADO ; k++) {
					for(l = 0 ; l < LADO ; l++) {
						se_puede_mover = mover_pieza_a_destino(i, k, j, l, campo, &aux);
						if(se_puede_mover) break;
					}
					if(se_puede_mover) break;
				}
				if(se_puede_mover) break;
			}
			if(se_puede_mover) break;
		}
		if(se_puede_mover) break;
	}
	return se_puede_mover;
}

void obtener_fila_y_columna(int * fila, int * columna) {
	*fila = (mouse_y - 11) / 80;
	*columna = (mouse_x - 11) / 80;
}

void verificar_estado_de_rey(bool turno_blanca, bool * mensaje_jaque_mate, bool * mensaje_jaque, bool * jaque_mate, bool negra_esta_en_jaque, bool blanca_esta_en_jaque, char campo[LADO][LADO]) {
	if(*mensaje_jaque_mate && (!hay_movimiento_permitido_negras(campo) || !hay_movimiento_permitido_blancas(campo)) && (negra_esta_en_jaque || blanca_esta_en_jaque)) {
		*jaque_mate = true;
		allegro_message("\n          JAQUE MATE          \n\n");
		*mensaje_jaque_mate = false;
		minuto_b = PARAR_CRONOMETRO;
		minuto_n = PARAR_CRONOMETRO;
	}

	if(turno_blanca) {
		if(*mensaje_jaque_mate && !hay_movimiento_permitido_negras(campo) && !negra_esta_en_jaque && !blanca_esta_en_jaque) {
			*jaque_mate = true;
			allegro_message("\n          EMPATE POR AHOGADO          \n\n");
			*mensaje_jaque_mate = false;
			minuto_b = PARAR_CRONOMETRO;
			minuto_n = PARAR_CRONOMETRO;
		}
	} else {
		if(*mensaje_jaque_mate && !hay_movimiento_permitido_blancas(campo) && !negra_esta_en_jaque && !blanca_esta_en_jaque) {
			*jaque_mate = true;
			allegro_message("\n          EMPATE POR AHOGADO          \n\n");
			*mensaje_jaque_mate = false;
			minuto_b = PARAR_CRONOMETRO;
			minuto_n = PARAR_CRONOMETRO;
		}
	}

	if(*mensaje_jaque && (verificar_jaque_intermedio_negras(campo) || verificar_jaque_intermedio_blancas(campo)) && !(!hay_movimiento_permitido_negras(campo) || !hay_movimiento_permitido_blancas(campo))) {
		allegro_message("\n          JAQUE          \n\n");
		*mensaje_jaque = false;
	}
}

void seleccionar_pieza_negra_a_mover(BITMAP * pantalla, char pieza, char campo[LADO][LADO]) {
	switch(pieza) {
		case 'P': dibujar_peon_en_movimiento(pantalla,'P', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'A': dibujar_alfil_en_movimiento(pantalla,'A', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'T': dibujar_torre_en_movimiento(pantalla,'T', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'W': dibujar_reina_en_movimiento(pantalla,'W', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'R': dibujar_rey_en_movimiento(pantalla,'R', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'C': dibujar_caballo_en_movimiento(pantalla,'C', mouse_x - 40 , mouse_y - 40, campo); break;
	}
}

void seleccionar_pieza_blanca_a_mover(BITMAP * pantalla, char pieza, char campo[LADO][LADO]) {
	switch(pieza) {
		case 'p': dibujar_peon_en_movimiento(pantalla, 'p', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'a': dibujar_alfil_en_movimiento(pantalla,'a', mouse_x - 40 , mouse_y - 40, campo); break;
		case 't': dibujar_torre_en_movimiento(pantalla,'t', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'w': dibujar_reina_en_movimiento(pantalla,'w', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'r': dibujar_rey_en_movimiento(pantalla,'r', mouse_x - 40 , mouse_y - 40, campo); break;
		case 'c': dibujar_caballo_en_movimiento(pantalla,'c', mouse_x - 40 , mouse_y - 40, campo); break;
	}
}

void seleccionar_origen_blanca(bool condicion_blanca_seleccionar, bool blanca_en_jaque, int fila, int columna, char *pieza, bool turno_blanca, int *clic_blanca, int *clic_negra, int *fila_origen, int *columna_origen, char campo[LADO][LADO]) {
	if(condicion_blanca_seleccionar && !blanca_en_jaque) {
		seleccionar_origen(fila, columna, turno_blanca, clic_blanca, clic_negra, fila_origen, columna_origen);
		*pieza = campo[*fila_origen][*columna_origen];
	}
	if(condicion_blanca_seleccionar && blanca_en_jaque) {
		seleccionar_origen(fila, columna, turno_blanca, clic_blanca, clic_negra, fila_origen, columna_origen);
		*pieza = campo[*fila_origen][*columna_origen];
	}
}

void seleccionar_origen_negra(bool condicion_negra_seleccionar, bool negra_en_jaque, int fila, int columna, char *pieza, bool turno_blanca, int *clic_blanca, int *clic_negra, int *fila_origen, int *columna_origen, char campo[LADO][LADO]) {
	if(condicion_negra_seleccionar && !negra_en_jaque) {
		seleccionar_origen(fila, columna, turno_blanca, clic_blanca, clic_negra, fila_origen, columna_origen);
		*pieza = campo[*fila_origen][*columna_origen];
	}
	if(condicion_negra_seleccionar && negra_en_jaque) {
		seleccionar_origen(fila, columna, turno_blanca, clic_blanca, clic_negra, fila_origen, columna_origen);
		*pieza = campo[*fila_origen][*columna_origen];
	}
}

void dibujar_cuadros_seleccion_anterior(BITMAP *pantalla, char campo[LADO][LADO]) {
	if(f_origen_anterior != -1 && c_origen_anterior != -1 && f_destino_anterior != -1 && c_destino_anterior != -1) {
		draw_cuadrado_relleno(f_origen_anterior, c_origen_anterior, campo, SELECCION_OSCURO, pantalla);
		draw_cuadrado_relleno(f_destino_anterior, c_destino_anterior, campo, SELECCION_OSCURO, pantalla);
	}
}

void asignacion_variables_auxiliares(bool *turno_blanca, bool valor_turno_blanca, bool * mensaje_jaque, bool * mensaje_jaque_mate, int fila_origen, int columna_origen, int fila_destino, int columna_destino) {
	*turno_blanca = valor_turno_blanca;
	*mensaje_jaque = true;
	*mensaje_jaque_mate = true;
	f_origen_anterior = fila_origen;
	c_origen_anterior = columna_origen;
	f_destino_anterior = fila_destino;
	c_destino_anterior = columna_destino;
}

bool es_fuera_de_menu(int *arr, int mouse_x, int mouse_y) {
	return !(mouse_x > arr[4] && mouse_x < arr[6] && mouse_y > arr[5] && mouse_y < arr[7]);
}

bool mouse_dentro_tablero(int mouse_x, int mouse_y) {
	obtener_fila_y_columna(&mouse_x, &mouse_y);
	return mouse_x < 8 && mouse_y < 8;
}

void promocionar_peon_blanco(char pieza_promocion_blanca, char campo[LADO][LADO]) {
	int i;
	for(i = 0 ; i < 8 ; i++) if(campo[0][i] == 'p') campo[0][i] = pieza_promocion_blanca;
}

void promocionar_peon_negro(char pieza_promocion_negra, char campo[LADO][LADO]) {
	int i;
	for(i = 0 ; i < 8 ; i++) if(campo[7][i] == 'P') campo[7][i] = pieza_promocion_negra;
}

void tiempo_jugador_blanco(BITMAP *pantalla, int minuto, int segundo, bool *tiempo_limite) {
	if(!*tiempo_limite && minuto >= 0 && minuto != PARAR_CRONOMETRO)
		(segundo < 10) ? textprintf_centre_ex(pantalla, font, 767, 505, 16, 28, "0%d : 0%d", minuto, segundo) : textprintf_centre_ex(pantalla, font, 767, 505, 16, 28, "0%d : %d", minuto, segundo);
	if(minuto < 0) *tiempo_limite = true;
}

void tiempo_jugador_negro(BITMAP * pantalla, int minuto, int segundo, bool *tiempo_limite) {
	if(!*tiempo_limite && minuto >= 0 && minuto != PARAR_CRONOMETRO)
		(segundo < 10) ? textprintf_centre_ex(pantalla, font, 767, 185, 16, 28, "0%d : 0%d", minuto, segundo) : textprintf_centre_ex(pantalla, font, 767, 185, 16, 28, "0%d : %d", minuto, segundo);
	if(minuto < 0) *tiempo_limite = true;
}

void cronometro_jugador_blanco(void) {
	while(minuto_b != PARAR_CRONOMETRO) {
		if(milesima_segundo_b < 1) {
			segundo_b--;
			milesima_segundo_b = 10;
			if(segundo_b < 0) {
				minuto_b--;
				segundo_b = 59;
			}
		}
		milesima_segundo_b--;
		rest(100);
	}
}

void cronometro_jugador_negro(void) {
	while(minuto_n != PARAR_CRONOMETRO) {
		if(milesima_segundo_n < 1) {
			segundo_n--;
			milesima_segundo_n = 10;
			if(segundo_n < 0) {
				minuto_n--;
				segundo_n = 59;
			}
		}
		milesima_segundo_n--;
		rest(100);
	}
}

void timer(BITMAP *pantalla, bool turno_blanca, int minuto_n_detenido, int segundo_n_detenido, int minuto_b_detenido, int segundo_b_detenido, bool *t_limite_b, bool *t_limite_n) {
	if(turno_blanca) {
		tiempo_jugador_negro(pantalla, minuto_n_detenido, segundo_n_detenido, t_limite_n);
		tiempo_jugador_blanco(pantalla, minuto_b, segundo_b, t_limite_b);
	} else {
		tiempo_jugador_blanco(pantalla, minuto_b_detenido, segundo_b_detenido, t_limite_b);
		tiempo_jugador_negro(pantalla, minuto_n, segundo_n, t_limite_n);
	}
}

void actualizacion_timer_negro(int *minuto_n_detenido, int *segundo_n_detenido, int *minuto_b_detenido, int *segundo_b_detenido) {
	*minuto_n_detenido = minuto_n;
	*segundo_n_detenido = segundo_n;
	minuto_b = *minuto_b_detenido;
	segundo_b = *segundo_b_detenido;
}

void actualizacion_timer_blanco(int *minuto_n_detenido, int *segundo_n_detenido, int *minuto_b_detenido, int *segundo_b_detenido) {
	*minuto_b_detenido = minuto_b;
	*segundo_b_detenido = segundo_b;
	minuto_n = *minuto_n_detenido;
	segundo_n = *segundo_n_detenido;
}

void verificar_tiempo_limite_message(bool tiempo_limite_blanco, bool tiempo_limite_negro, bool *mensaje_tiempo_limite) {
	if(!*mensaje_tiempo_limite) {
		if(tiempo_limite_blanco) {
			allegro_message("TIEMPO LIMITE, GANA JUGADOR NEGRO");
			*mensaje_tiempo_limite = true;
		}
		if(tiempo_limite_negro) {
			allegro_message("TIEMPO LIMITE, GANA JUGADOR BLANCO");
			*mensaje_tiempo_limite = true;
		}
	}
}

void marcar_movimiento_permitido(BITMAP * pantalla, char campo[LADO][LADO], int fila_origen, int columna_origen) {
	int i, j;
	bool condicion_dibujo = false, soy_blanco = false;
	for(i = 0 ; i < LADO ; i++) {
		for(j = 0 ; j < LADO ; j++) {
			condicion_dibujo = mover_pieza_a_destino(fila_origen, i, columna_origen, j, campo, NULL);
			soy_blanco = es_pieza_blanca(campo[fila_origen][columna_origen]);

			if(condicion_dibujo && campo[i][j] == ' ') circlefill(pantalla, j * 80 + 50 , 50 + i * 80 , 12, 26);

			if(condicion_dibujo && campo[i][j] != ' ' && ((soy_blanco && !es_pieza_blanca(campo[i][j])) || (!soy_blanco && es_pieza_blanca(campo[i][j])))) {
				draw_cuadrado(i, j, campo, 16, 26, pantalla);
			}
		}
	}
}

void re_dibujar(BITMAP *pantalla, int fila_origen, int columna_origen, int fila_destino, int columna_destino, char campo[LADO][LADO], bool se_movio) {
	draw_cuadrado(fila_origen, columna_origen, campo, CLARO, OSCURO, pantalla);
	re_draw(pantalla, campo);
	if(se_movio) {
		draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
		draw_cuadrado(fila_destino, columna_destino, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
	}
}

// modo test, se mueven las piezas para detectar errores graficos o logicos
void seleccionar_en_solitario(char campo[LADO][LADO], SAMPLE * sonido_mover, BITMAP * pantalla) {
	int fila = 0, columna = 0, fila_origen = 0, fila_destino = -1, columna_origen = 0, columna_destino = -1,
		clic_blanca = 0, clic_negra = 0, minuto_b_detenido = 5, segundo_b_detenido = 0, minuto_n_detenido = 5, segundo_n_detenido = 0;

	bool turno_blanca = true, blanca_en_jaque = false, negra_en_jaque = false, condicion_blanca_seleccionar = false,
		 condicion_negra_seleccionar = false, movio_blanca = false, movio_negra = false, jaque_mate = false,
		 mensaje_jaque = true, mensaje_jaque_mate = true, tiempo_limite_negro = false,
		 tiempo_limite_blanco = false, mensaje_tiempo_limite = false, presiono_blanca = false, presiono_negra = false;

	char pieza = ' ', pieza_promocion_blanca = 'w', pieza_promocion_negra = 'W';

	pthread_t hilo_timer_blanco, hilo_timer_negro;

	pthread_create(&hilo_timer_blanco, NULL, (void*)cronometro_jugador_blanco, NULL);
	pthread_create(&hilo_timer_negro, NULL, (void*)cronometro_jugador_negro, NULL);

	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	while(!close_button_pressed) {
		blit(pantalla, screen, 0, 0, 0, 0, 870, 667);
		verificar_estado_de_rey(turno_blanca, &mensaje_jaque_mate, &mensaje_jaque, &jaque_mate, negra_en_jaque, blanca_en_jaque, campo);

		if(negra_en_jaque) draw_cuadrado(f_rey_n, c_rey_n, campo, 15, COLOR_JAQUE, pantalla);
		if(blanca_en_jaque) draw_cuadrado(f_rey_b, c_rey_b, campo, 15, COLOR_JAQUE, pantalla);

		rest(30);

		if(!jaque_mate && (mouse_b & 1) && mouse_dentro_tablero(mouse_x, mouse_y) && !tiempo_limite_blanco && !tiempo_limite_negro) {

			obtener_fila_y_columna(&fila, &columna);

			if(turno_blanca) {
				condicion_blanca_seleccionar = hay_pieza(fila, columna, campo) && es_pieza_blanca(campo[fila][columna]);
				seleccionar_origen_blanca(condicion_blanca_seleccionar, blanca_en_jaque, fila, columna, &pieza, turno_blanca, &clic_blanca, &clic_negra, &fila_origen, &columna_origen, campo);

				if(clic_blanca > 0) {
					presiono_blanca = true;
					presiono_negra = false;
					while(mouse_b & 1) {
						ocultar_pieza(fila_origen, columna_origen, campo, 114, 66, pantalla);
						marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
						(blanca_en_jaque && fila_origen == f_rey_b && columna_origen == c_rey_b) ? draw_cuadrado(f_rey_b, c_rey_b, campo, 15, COLOR_JAQUE, pantalla) :
								draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
						obtener_fila_y_columna(&fila_destino, &columna_destino);
						seleccionar_pieza_blanca_a_mover(pantalla, pieza, campo);
						re_draw(pantalla, campo);
						clic_blanca += 1;
						dibujar_cuadros_seleccion_anterior(pantalla, campo);
						dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
						tiempo_jugador_blanco(pantalla, minuto_b, segundo_b, &tiempo_limite_blanco);
						tiempo_jugador_negro(pantalla, minuto_n_detenido, segundo_n_detenido, &tiempo_limite_negro);
					}
					campo[fila_origen][columna_origen] = pieza;
				}

				if(fila_destino != -1 && columna_destino != -1) {
					movio_blanca = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &blanca_en_jaque);
					if(movio_blanca) {
						presiono_blanca = false;
						play_sample(sonido_mover, 200, 150, 1000, 0);
						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_blanco(pieza_promocion_blanca, campo);
						actualizacion_timer_blanco(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'r') {
							f_rey_b = fila_destino;
							c_rey_b = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_blanca);
						asignacion_variables_auxiliares(&turno_blanca, false, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);
					} else {
						turno_blanca = true;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_blanca && verificar_jaque(pieza, fila_destino, columna_destino, campo)) negra_en_jaque = true;
					clic_blanca = 0;
				}

			} else {

				condicion_negra_seleccionar = hay_pieza(fila, columna, campo) && !es_pieza_blanca(campo[fila][columna]);
				seleccionar_origen_negra(condicion_negra_seleccionar, negra_en_jaque, fila, columna, &pieza, turno_blanca, &clic_blanca, &clic_negra, &fila_origen, &columna_origen, campo);

				if(clic_negra > 0) {
					presiono_negra = true;
					presiono_blanca = false;
					while(mouse_b & 1) {
						ocultar_pieza(fila_origen, columna_origen, campo, 114, 66, pantalla);
						marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
						(negra_en_jaque && fila_origen == f_rey_n && columna_origen == c_rey_n) ? draw_cuadrado(f_rey_n, c_rey_n, campo, 15, COLOR_JAQUE, pantalla) :
							draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
						obtener_fila_y_columna(&fila_destino, &columna_destino);
						seleccionar_pieza_negra_a_mover(pantalla, pieza, campo);
						re_draw(pantalla, campo);
						clic_negra += 1;
						dibujar_cuadros_seleccion_anterior(pantalla, campo);
						dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
						tiempo_jugador_negro(pantalla, minuto_n, segundo_n, &tiempo_limite_negro);
						tiempo_jugador_blanco(pantalla, minuto_b_detenido, segundo_b_detenido, &tiempo_limite_blanco);
					}
					campo[fila_origen][columna_origen] = pieza;
				}

				if(fila_destino != -1 && columna_destino != -1) {
					movio_negra = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &negra_en_jaque);
					if(movio_negra) {
						presiono_negra = false;
						play_sample(sonido_mover, 200, 150, 1000, 0);
						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_negro(pieza_promocion_negra, campo);
						actualizacion_timer_negro(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'R') {
							f_rey_n = fila_destino;
							c_rey_n = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_negra);
						asignacion_variables_auxiliares(&turno_blanca, true, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);
					} else {
						turno_blanca = false;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_negra && verificar_jaque(pieza, fila_destino, columna_destino, campo)) blanca_en_jaque = true;
					clic_negra = 0;
				}
			}
		}

		if(!jaque_mate && !mouse_dentro_tablero(mouse_x, mouse_y) && (mouse_b & 1) && !(tiempo_limite_blanco || tiempo_limite_negro)) {
			(turno_blanca) ? seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_blanca, turno_blanca) : seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_negra, turno_blanca);
			re_draw(pantalla, campo);
		}

		verificar_tiempo_limite_message(tiempo_limite_blanco, tiempo_limite_negro, &mensaje_tiempo_limite);
		dibujar_cuadros_seleccion_anterior(pantalla, campo);
		if(!presiono_blanca && presiono_negra && !negra_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
		if(!presiono_negra && presiono_blanca && !blanca_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
		marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
		dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
		timer(pantalla, turno_blanca, minuto_n_detenido, segundo_n_detenido, minuto_b_detenido, segundo_b_detenido, &tiempo_limite_blanco, &tiempo_limite_negro);
	}

}

typedef struct {
	char pieza;
	int fila_origen;
	int fila_destino;
	int columna_origen;
	int columna_destino;
} __attribute__((packed)) protocolo;

void asignar_desde_el_paquete(char *pieza, int *fila_origen, int *columna_origen, int *fila_destino, int *columna_destino, protocolo *package) {
	*pieza = package->pieza;
	*fila_origen = package->fila_origen;
	*columna_origen = package->columna_origen;
	*fila_destino = package->fila_destino;
	*columna_destino = package->columna_destino;
}

void asignar_hacia_el_paquete(int fila_origen, int columna_origen, int fila_destino, int columna_destino, protocolo *package, char campo[LADO][LADO]) {
	package->pieza = campo[fila_destino][columna_destino];
	package->fila_origen = fila_origen;
	package->fila_destino = fila_destino;
	package->columna_origen = columna_origen;
	package->columna_destino = columna_destino;
}

void *serializar(protocolo *paquete) {
	void * stream = malloc(TAMANIO_STREAM);
	memcpy(stream, &(paquete->pieza), 1);
	memcpy(stream + 1, &(paquete->fila_origen), 4);
	memcpy(stream + 5, &(paquete->fila_destino), 4);
	memcpy(stream + 9, &(paquete->columna_origen), 4);
	memcpy(stream + 13, &(paquete->columna_destino), 4);
	return stream;
}

int recibir(int socket, protocolo *paquete) {
	void *buffer = malloc(TAMANIO_STREAM);
	int recibio_stream = recv(socket, buffer, TAMANIO_STREAM, 0);
	memcpy(&(paquete->pieza), buffer, 1);
	memcpy(&(paquete->fila_origen), buffer + 1, 4);
	memcpy(&(paquete->fila_destino), buffer + 5, 4);
	memcpy(&(paquete->columna_origen), buffer + 9, 4);
	memcpy(&(paquete->columna_destino), buffer + 13, 4);
	free(buffer);
	return recibio_stream;
}

void seleccionar_en_red(char campo[LADO][LADO], SAMPLE * sonido_mover, BITMAP * pantalla, char modo_cliente_o_servidor, int *socket) {
	int fila = 0, columna = 0, fila_origen = 0, fila_destino = -1, columna_origen = 0, columna_destino = -1,
		clic_blanca = 0, clic_negra = 0, minuto_b_detenido = 5, segundo_b_detenido = 0, minuto_n_detenido = 5, segundo_n_detenido = 0;

	bool turno_blanca = true, blanca_en_jaque = false, negra_en_jaque = false, condicion_blanca_seleccionar = false,
			 condicion_negra_seleccionar = false, movio_blanca = false, movio_negra = false, jaque_mate = false,
			 mensaje_jaque = true, mensaje_jaque_mate = true, tiempo_limite_negro = false,
			 tiempo_limite_blanco = false, mensaje_tiempo_limite = false, presiono_blanca = false, presiono_negra = false;

	char pieza = ' ', pieza_promocion_blanca = 'w', pieza_promocion_negra = 'W';

	pthread_t hilo_timer_blanco, hilo_timer_negro;
	protocolo package;

	pthread_create(&hilo_timer_blanco, NULL, (void*)cronometro_jugador_blanco, NULL);
	pthread_create(&hilo_timer_negro, NULL, (void*)cronometro_jugador_negro, NULL);

	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	// servidor --> jugador blanco
	if(modo_cliente_o_servidor == 's') {

		while(!close_button_pressed) {
			blit(pantalla, screen, 0, 0, 0, 0, 870, 667);
			verificar_estado_de_rey(turno_blanca, &mensaje_jaque_mate, &mensaje_jaque, &jaque_mate, negra_en_jaque, blanca_en_jaque, campo);

			if(negra_en_jaque) draw_cuadrado(f_rey_n, c_rey_n, campo, 15, COLOR_JAQUE, pantalla);
			if(blanca_en_jaque) draw_cuadrado(f_rey_b, c_rey_b, campo, 15, COLOR_JAQUE, pantalla);

			rest(30);

			if(!jaque_mate && !turno_blanca && !tiempo_limite_blanco && !tiempo_limite_negro && (recibir(*socket, &package) != -1)) {
				asignar_desde_el_paquete(&pieza, &fila_origen, &columna_origen, &fila_destino, &columna_destino, &package);

				if(fila_destino != -1 && columna_destino != -1) {
					movio_negra = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &negra_en_jaque);
					if(movio_negra) {
						presiono_negra = true;
						presiono_blanca = true;
						play_sample(sonido_mover, 200, 150, 1000, 0);
						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_negro(pieza_promocion_negra, campo);
						actualizacion_timer_negro(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'R') {
							f_rey_n = fila_destino;
							c_rey_n = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_negra);
						asignacion_variables_auxiliares(&turno_blanca, true, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);
					} else {
						turno_blanca = false;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_negra && verificar_jaque(pieza, fila_destino, columna_destino, campo)) blanca_en_jaque = true;
					clic_negra = 0;
				}
			}

			if(turno_blanca && mouse_dentro_tablero(mouse_x, mouse_y) && !jaque_mate && (mouse_b & 1) && !tiempo_limite_blanco && !tiempo_limite_negro) {
				obtener_fila_y_columna(&fila, &columna);

				condicion_blanca_seleccionar = hay_pieza(fila, columna, campo) && es_pieza_blanca(campo[fila][columna]);
				seleccionar_origen_blanca(condicion_blanca_seleccionar, blanca_en_jaque, fila, columna, &pieza, turno_blanca, &clic_blanca, &clic_negra, &fila_origen, &columna_origen, campo);

				if(clic_blanca > 0) {
					presiono_blanca = true;
					while(mouse_b & 1) {
						ocultar_pieza(fila_origen, columna_origen, campo, 114, 66, pantalla);
						marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
						(blanca_en_jaque && fila_origen == f_rey_b && columna_origen == c_rey_b) ? draw_cuadrado(f_rey_b, c_rey_b, campo, 15, COLOR_JAQUE, pantalla) :
							draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
						obtener_fila_y_columna(&fila_destino, &columna_destino);
						seleccionar_pieza_blanca_a_mover(pantalla, pieza, campo);
						re_draw(pantalla, campo);
						clic_blanca += 1;
						dibujar_cuadros_seleccion_anterior(pantalla, campo);
						dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
						tiempo_jugador_blanco(pantalla, minuto_b, segundo_b, &tiempo_limite_blanco);
						tiempo_jugador_negro(pantalla, minuto_n_detenido, segundo_n_detenido, &tiempo_limite_negro);
					}
					campo[fila_origen][columna_origen] = pieza;
				}

				if(fila_destino != -1 && columna_destino != -1) {
					movio_blanca = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &blanca_en_jaque);
					if(movio_blanca) {
						presiono_blanca = true;
						presiono_negra = true;
						play_sample(sonido_mover, 200, 150, 1000, 0);

						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_blanco(pieza_promocion_blanca, campo);
						actualizacion_timer_blanco(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'r') {
							f_rey_b = fila_destino;
							c_rey_b = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_blanca);
						asignacion_variables_auxiliares(&turno_blanca, false, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);

						asignar_hacia_el_paquete(fila_origen, columna_origen, fila_destino, columna_destino, &package, campo);
						void* buffer = serializar(&package);
						send(*socket, buffer, TAMANIO_STREAM, 0);

					} else {
						turno_blanca = true;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_blanca && verificar_jaque(pieza, fila_destino, columna_destino, campo)) negra_en_jaque = true;
					clic_blanca = 0;
				}

			}
			if(!jaque_mate && !mouse_dentro_tablero(mouse_x, mouse_y) && (mouse_b & 1) && !(tiempo_limite_blanco || tiempo_limite_negro)) {
				(turno_blanca) ? seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_blanca, turno_blanca) : seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_negra, turno_blanca);
				re_draw(pantalla, campo);
			}

			verificar_tiempo_limite_message(tiempo_limite_blanco, tiempo_limite_negro, &mensaje_tiempo_limite);
			dibujar_cuadros_seleccion_anterior(pantalla, campo);
			if(!presiono_blanca && presiono_negra && !negra_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
			if(!presiono_negra && presiono_blanca && !blanca_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
			marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
			dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
			timer(pantalla, turno_blanca, minuto_n_detenido, segundo_n_detenido, minuto_b_detenido, segundo_b_detenido, &tiempo_limite_blanco, &tiempo_limite_negro);
		}

	}

	// cliente --> jugador negro
	if(modo_cliente_o_servidor == 'c') {

		while(!close_button_pressed) {
			blit(pantalla, screen, 0, 0, 0, 0, 870, 667);
			verificar_estado_de_rey(turno_blanca, &mensaje_jaque_mate, &mensaje_jaque, &jaque_mate, negra_en_jaque, blanca_en_jaque, campo);

			if(negra_en_jaque) draw_cuadrado(f_rey_n, c_rey_n, campo, 15, COLOR_JAQUE, pantalla);
			if(blanca_en_jaque) draw_cuadrado(f_rey_b, c_rey_b, campo, 15, COLOR_JAQUE, pantalla);

			rest(30);

			if(!jaque_mate && turno_blanca && !tiempo_limite_blanco && !tiempo_limite_negro && (recibir(*socket, &package) != -1)) {
				asignar_desde_el_paquete(&pieza, &fila_origen, &columna_origen, &fila_destino, &columna_destino, &package);

				if(fila_destino != -1 && columna_destino != -1) {
					movio_blanca = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &blanca_en_jaque);
					if(movio_blanca) {
						presiono_blanca = true;
						presiono_negra = true;
						play_sample(sonido_mover, 200, 150, 1000, 0);
						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_blanco(pieza_promocion_blanca, campo);
						actualizacion_timer_blanco(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'r') {
							f_rey_b = fila_destino;
							c_rey_b = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_blanca);
						asignacion_variables_auxiliares(&turno_blanca, false, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);
					} else {
						turno_blanca = true;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_blanca && verificar_jaque(pieza, fila_destino, columna_destino, campo)) negra_en_jaque = true;
					clic_blanca = 0;
				}

			}
			if(!turno_blanca && mouse_dentro_tablero(mouse_x, mouse_y) && !jaque_mate && (mouse_b & 1) && !tiempo_limite_blanco && !tiempo_limite_negro) {
				obtener_fila_y_columna(&fila, &columna);
				condicion_negra_seleccionar = hay_pieza(fila, columna, campo) && !es_pieza_blanca(campo[fila][columna]);
				seleccionar_origen_negra(condicion_negra_seleccionar, negra_en_jaque, fila, columna, &pieza, turno_blanca, &clic_blanca, &clic_negra, &fila_origen, &columna_origen, campo);

				if(clic_negra > 0) {
					presiono_negra = true;
					while(mouse_b & 1) {
						ocultar_pieza(fila_origen, columna_origen, campo, 114, 66, pantalla);
						marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
						(negra_en_jaque && fila_origen == f_rey_n && columna_origen == c_rey_n) ? draw_cuadrado(f_rey_n, c_rey_n, campo, 15, COLOR_JAQUE, pantalla) :
							draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
						obtener_fila_y_columna(&fila_destino, &columna_destino);
						seleccionar_pieza_negra_a_mover(pantalla, pieza, campo);
						re_draw(pantalla, campo);
						clic_negra += 1;
						dibujar_cuadros_seleccion_anterior(pantalla, campo);
						dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
						tiempo_jugador_negro(pantalla, minuto_n, segundo_n, &tiempo_limite_negro);
						tiempo_jugador_blanco(pantalla, minuto_b_detenido, segundo_b_detenido, &tiempo_limite_blanco);
					}
					campo[fila_origen][columna_origen] = pieza;
				}

				if(fila_destino != -1 && columna_destino != -1) {
					movio_negra = mover_pieza_a_destino(fila_origen, fila_destino, columna_origen, columna_destino, campo, &negra_en_jaque);
					if(movio_negra) {
						presiono_negra = true;
						presiono_blanca = true;
						play_sample(sonido_mover, 200, 150, 1000, 0);

						aplicar_movimiento(fila_origen, columna_origen, fila_destino, columna_destino, campo);
						promocionar_peon_negro(pieza_promocion_negra, campo);
						actualizacion_timer_negro(&minuto_n_detenido, &segundo_n_detenido, &minuto_b_detenido, &segundo_b_detenido);

						if(campo[fila_destino][columna_destino] == 'R') {
							f_rey_n = fila_destino;
							c_rey_n = columna_destino;
						}
						re_dibujar(pantalla, fila_origen, columna_origen, fila_destino, columna_destino, campo, movio_negra);
						asignacion_variables_auxiliares(&turno_blanca, true, &mensaje_jaque, &mensaje_jaque_mate, fila_origen, columna_origen, fila_destino, columna_destino);

						asignar_hacia_el_paquete(fila_origen, columna_origen, fila_destino, columna_destino, &package, campo);
						void* buffer = serializar(&package);
						send(*socket, buffer, TAMANIO_STREAM, 0);

					} else {
						turno_blanca = false;
						switcheo_draw_piezas(pantalla, pieza, campo);
					}
					if(movio_negra && verificar_jaque(pieza, fila_destino, columna_destino, campo)) blanca_en_jaque = true;
					clic_negra = 0;
				}
			}

			if(!jaque_mate && !mouse_dentro_tablero(mouse_x, mouse_y) && (mouse_b & 1) && !(tiempo_limite_blanco || tiempo_limite_negro)) {
				(turno_blanca) ? seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_blanca, turno_blanca) : seleccionar_promocion(pantalla, mouse_x, mouse_y, &pieza_promocion_negra, turno_blanca);
				re_draw(pantalla, campo);
			}

			verificar_tiempo_limite_message(tiempo_limite_blanco, tiempo_limite_negro, &mensaje_tiempo_limite);
			dibujar_cuadros_seleccion_anterior(pantalla, campo);
			if(!presiono_blanca && presiono_negra && !negra_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
			if(!presiono_negra && presiono_blanca && !blanca_en_jaque) draw_cuadrado(fila_origen, columna_origen, campo, SELECCION_CLARO, SELECCION_OSCURO, pantalla);
			marcar_movimiento_permitido(pantalla, campo, fila_origen, columna_origen);
			dibujar_seleccion_promocion(pantalla, pieza_promocion_blanca, pieza_promocion_negra);
			timer(pantalla, turno_blanca, minuto_n_detenido, segundo_n_detenido, minuto_b_detenido, segundo_b_detenido, &tiempo_limite_blanco, &tiempo_limite_negro);

		}
	}

}
