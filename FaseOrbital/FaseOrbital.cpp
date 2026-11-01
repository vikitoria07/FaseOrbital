// FaseOrbital.cpp 

// IMPORTACIONES

#include <iostream>
#include <string>
#include <cmath>
#include <allegro5/allegro.h>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <cstdint>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstring>

// HEADERS

#include "Generador.h"
#include "Jugador.h"
#include "Helpers.h"
#include "Enemigos.h"
#include "Balas.h"
#include "Pulso.h"
#include "Bitacora.h"
#include "Estadisticas&Ranking.h"
#include "Acciones.h"
#include "MotorJuego.h"
#include "Scripts.h"

// PRUEBAS

#include "Pruebas.h"

using namespace std;


// ============================================================
// VISTA (solo lee el estado del motor, no aplica reglas)
// ============================================================

namespace {
	constexpr float CENTRO_X = 400.0f;
	constexpr float CENTRO_Y = 300.0f;
	constexpr float RADIO_ORBITA = 100.0f;
	constexpr float RADIO_NUCLEO = 20.0f;
	constexpr float TAMANO_NAVE = 6.0f;
	constexpr float TAMANO_BALA = 3.0f;

	// Convierte (r, theta en grados) del motor a coordenadas de pantalla.
	void posicionEnPantalla(double r, double thetaGrados, float& x, float& y) {
		double rad = aRadianes(thetaGrados);
		x = CENTRO_X + static_cast<float>(r * cos(rad));
		y = CENTRO_Y + static_cast<float>(r * sin(rad));
	}

	// Los valores 0, 1 y 2 coinciden con CIAN, MAGENTA y AMARILLO.
	ALLEGRO_COLOR colorDeIndice(int indice) {
		switch (indice) {
		case CIAN:     return al_map_rgb(0, 255, 255);
		case MAGENTA:  return al_map_rgb(255, 0, 255);
		default:       return al_map_rgb(255, 255, 0);
		}
	}

	void dibujarNucleo() {
		al_draw_circle(CENTRO_X, CENTRO_Y, RADIO_ORBITA, al_map_rgb(70, 70, 70), 1.0f);
		al_draw_filled_circle(CENTRO_X, CENTRO_Y, RADIO_NUCLEO, al_map_rgb(255, 255, 255));
	}

	void dibujarPulso(const Pulso& pulso) {
		if (pulso.activo) {
			al_draw_circle(CENTRO_X, CENTRO_Y,
				static_cast<float>(pulso.radio_actual),
				al_map_rgb(255, 255, 255), 2.0f);
		}
	}

	void dibujarEnemigos(const ListaEnemigos& enemigos) {
		for (const NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Enemigo& e = actual->dato;
			float x, y;
			posicionEnPantalla(e.r, e.theta, x, y);
			al_draw_filled_circle(x, y, static_cast<float>(e.radio), colorDeIndice(e.color));
		}
	}

	void dibujarBalas(const ListaBalas& balas) {
		for (const NodoBala* actual = balas.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Bala& b = actual->dato;
			al_draw_filled_circle(CENTRO_X + static_cast<float>(b.x),
				CENTRO_Y + static_cast<float>(b.y),
				TAMANO_BALA, colorDeIndice(b.color));
		}
	}

	void dibujarNave(const Jugador& jugador) {
		float x, y;
		posicionEnPantalla(RADIO_ORBITA, jugador.obtenerPosicionAngular(), x, y);
		al_draw_filled_circle(x, y, TAMANO_NAVE, colorDeIndice(jugador.obtenerColorActual()));
	}
}

void dibujarMundo(const Motor& motor) {
	dibujarNucleo();
	dibujarPulso(motor.obtenerPulso());
	dibujarEnemigos(motor.obtenerEnemigos());
	dibujarBalas(motor.obtenerBalas());
	dibujarNave(motor.obtenerJugador());
}



// ============================================================
// ENTRADA (traduce teclado y mouse a Accion, sin aplicar reglas)
// ============================================================

namespace {
	constexpr float VELOCIDAD_GIRO = 4.0f; // grados por tick
}

struct EstadoEntrada {
	bool izquierda = false;
	bool derecha = false;
	bool pausado = false;
	bool pulsoPedido = false;
	bool colorPedido = false;
	Color colorSeleccionado = MAGENTA;
	bool mouseMovido = false;
	float anguloMouse = 0.0f;
};

void procesarEvento(const ALLEGRO_EVENT& ev, EstadoEntrada& e) {
	switch (ev.type) {
	case ALLEGRO_EVENT_KEY_DOWN:
		switch (ev.keyboard.keycode) {
		case ALLEGRO_KEY_LEFT:  e.izquierda = true; break;
		case ALLEGRO_KEY_RIGHT: e.derecha = true; break;
		case ALLEGRO_KEY_1:
		case ALLEGRO_KEY_PAD_1:
			e.colorSeleccionado = CIAN; e.colorPedido = true; break;
		case ALLEGRO_KEY_2:
		case ALLEGRO_KEY_PAD_2:
			e.colorSeleccionado = MAGENTA; e.colorPedido = true; break;
		case ALLEGRO_KEY_3:
		case ALLEGRO_KEY_PAD_3:
			e.colorSeleccionado = AMARILLO; e.colorPedido = true; break;
		case ALLEGRO_KEY_SPACE: e.pulsoPedido = true; break;
		case ALLEGRO_KEY_P:
		case ALLEGRO_KEY_ESCAPE: e.pausado = !e.pausado; break;
		}
		break;

	case ALLEGRO_EVENT_KEY_UP:
		if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)  e.izquierda = false;
		if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) e.derecha = false;
		break;

	case ALLEGRO_EVENT_MOUSE_AXES: {
		float dx = static_cast<float>(ev.mouse.x) - CENTRO_X;
		float dy = static_cast<float>(ev.mouse.y) - CENTRO_Y;
		if (dx != 0.0f || dy != 0.0f) {
			e.anguloMouse = normalizarAngulo(
				static_cast<float>(atan2(dy, dx) * 180.0 / PI));
			e.mouseMovido = true;
		}
		break;
	}

	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		if (ev.mouse.button == 1) {
			e.pulsoPedido = true;
		}
		break;
	}
}

// Las flechas tienen prioridad sobre el mouse. Los eventos de un solo uso se consumen aquí.
Accion construirAccion(EstadoEntrada& e, const Jugador& jugador) {
	Accion accion;

	if (e.izquierda || e.derecha) {
		float delta = e.derecha ? VELOCIDAD_GIRO : -VELOCIDAD_GIRO;
		accion.tieneGiro = true;
		accion.anguloObjetivo =
			normalizarAngulo(jugador.obtenerPosicionAngular() + delta);
	}
	else if (e.mouseMovido) {
		accion.tieneGiro = true;
		accion.anguloObjetivo = e.anguloMouse;
		e.mouseMovido = false;
	}

	if (e.colorPedido) {
		accion.tieneColor = true;
		accion.color = e.colorSeleccionado;
		e.colorPedido = false;
	}

	accion.activarPulso = e.pulsoPedido;
	e.pulsoPedido = false;

	return accion;
}

void dibujarPausa() {
	al_draw_filled_rectangle(380.0f, 280.0f, 392.0f, 320.0f, al_map_rgb(255, 255, 255));
	al_draw_filled_rectangle(408.0f, 280.0f, 420.0f, 320.0f, al_map_rgb(255, 255, 255));
}

void dibujarHUD(const Motor& motor, const ALLEGRO_FONT* fuente, const char* nombreJugador) {
	const Jugador& jugador = motor.obtenerJugador();
	const ALLEGRO_COLOR blanco = al_map_rgb(255, 255, 255);

	al_draw_textf(fuente, blanco, 10.0f, 10.0f, ALLEGRO_ALIGN_LEFT,
		"Jugador: %s", nombreJugador);
	al_draw_textf(fuente, blanco, 10.0f, 28.0f, ALLEGRO_ALIGN_LEFT,
		"Puntaje: %.0f", jugador.obtenerPuntaje());
	al_draw_textf(fuente, blanco, 10.0f, 46.0f, ALLEGRO_ALIGN_LEFT,
		"Multiplicador: x%d", jugador.obtenerComboActual());
	al_draw_textf(fuente, blanco, 10.0f, 64.0f, ALLEGRO_ALIGN_LEFT,
		"Oleada: %d", motor.obtenerOleada());
	al_draw_textf(fuente, blanco, 10.0f, 82.0f, ALLEGRO_ALIGN_LEFT,
		"Vida nucleo: %d", jugador.obtenerVidaNucleo());
	al_draw_textf(fuente, blanco, 10.0f, 100.0f, ALLEGRO_ALIGN_LEFT,
		"Carga pulso: %.0f%%", jugador.obtenerCargaPulse());

	const float carga = jugador.obtenerCargaPulse();
	al_draw_rectangle(10.0f, 120.0f, 110.0f, 130.0f, blanco, 1.0f);
	al_draw_filled_rectangle(10.0f, 120.0f, 10.0f + carga, 130.0f,
		al_map_rgb(255, 255, 0));
}

// ============================================================
// PANTALLAS (estetica Game Boy)
// ============================================================

constexpr int MAX_JUGADORES = 4;
constexpr int LARGO_NOMBRE = 10;
constexpr int OPCIONES_MENU = 5;

enum class Pantalla { Titulo, Menu, RegistroJugadores, Turno, Juego, Pausa, Resumen, Fin, Estadisticas, Ranking };

enum IndiceGB { GB_OSCURO, GB_MEDIO_OSCURO, GB_MEDIO_CLARO, GB_CLARO };

// Paleta pixel art: azul noche, morado, magenta y amarillo.
ALLEGRO_COLOR colorGB(int indice) {
	switch (indice) {
	case 0: return al_map_rgb(16, 12, 40);     // fondo
	case 1: return al_map_rgb(70, 40, 120);    // sombra / marco
	case 2: return al_map_rgb(255, 60, 200);   // magenta
	default: return al_map_rgb(255, 230, 80);  // amarillo
	}
}

const int TAM_PIXEL = 8;

// Glifo 5x5 por carácter: '1' es píxel encendido.
const char* glifo(char c) {
	switch (c) {
	case 'A': return "01110" "10001" "11111" "10001" "10001";
	case 'B': return "11110" "10001" "11110" "10001" "11110";
	case 'E': return "11111" "10000" "11110" "10000" "11111";
	case 'F': return "11111" "10000" "11110" "10000" "10000";
	case 'I': return "11111" "00100" "00100" "00100" "11111";
	case 'L': return "10000" "10000" "10000" "10000" "11111";
	case 'O': return "01110" "10001" "10001" "10001" "01110";
	case 'R': return "11110" "10001" "11110" "10100" "10010";
	case 'S': return "01111" "10000" "01110" "00001" "11110";
	case 'T': return "11111" "00100" "00100" "00100" "00100";
	default:  return "00000" "00000" "00000" "00000" "00000";
	}
}

void dibujarTextoPixel(const char* texto, float x, float y, ALLEGRO_COLOR color) {
	for (const char* p = texto; *p != '\0'; ++p) {
		const char* g = glifo(*p);
		for (int fila = 0; fila < 5; ++fila) {
			for (int col = 0; col < 5; ++col) {
				if (g[fila * 5 + col] == '1') {
					float px = x + col * TAM_PIXEL;
					float py = y + fila * TAM_PIXEL;
					al_draw_filled_rectangle(px, py, px + TAM_PIXEL, py + TAM_PIXEL, color);
				}
			}
		}
		x += 6 * TAM_PIXEL;
	}
}

// Título centrado con sombra de un píxel de desplazamiento.
void dibujarTituloPixel(const char* texto, float y) {
	float ancho = static_cast<float>(std::strlen(texto) * 6 * TAM_PIXEL - TAM_PIXEL);
	float x = (800.0f - ancho) / 2.0f;
	dibujarTextoPixel(texto, x + TAM_PIXEL, y + TAM_PIXEL, colorGB(1));
	dibujarTextoPixel(texto, x, y, colorGB(3));
}

// Marco de bloques alternados alrededor de la pantalla.
void dibujarMarco() {
	for (int i = 0; i < 800; i += TAM_PIXEL * 2) {
		ALLEGRO_COLOR c = (i / TAM_PIXEL) % 2 == 0 ? colorGB(1) : colorGB(2);
		al_draw_filled_rectangle(i, 0, i + TAM_PIXEL, TAM_PIXEL, c);
		al_draw_filled_rectangle(i, 600 - TAM_PIXEL, i + TAM_PIXEL, 600, c);
	}
	for (int j = 0; j < 600; j += TAM_PIXEL * 2) {
		ALLEGRO_COLOR c = (j / TAM_PIXEL) % 2 == 0 ? colorGB(1) : colorGB(2);
		al_draw_filled_rectangle(0, j, TAM_PIXEL, j + TAM_PIXEL, c);
		al_draw_filled_rectangle(800 - TAM_PIXEL, j, 800, j + TAM_PIXEL, c);
	}
}

void texto(const ALLEGRO_FONT* fuente, float y, ALLEGRO_COLOR color, const char* t) {
	al_draw_text(fuente, color, 400.0f, y, ALLEGRO_ALIGN_CENTRE, t);
}

// Pantalla de inicio: fondo, marco, título y aviso.
void dibujarPantallaTitulo(const ALLEGRO_FONT* fuente) {
	al_clear_to_color(colorGB(0));
	dibujarMarco();
	dibujarTituloPixel("FASE ORBITAL", 220.0f);
	texto(fuente, 400.0f, colorGB(3), "PRESIONA ENTER");
}



struct Sesion {
	Pantalla actual = Pantalla::Titulo;
	int opcionMenu = 0;
	int cantidadJugadores = 1;
	int jugadorRegistrando = 0;
	int jugadorActivo = 0;
	char nombres[MAX_JUGADORES][LARGO_NOMBRE + 1] = {};
	Motor* juego = nullptr;
};

void iniciarJuego(Sesion& s, EstadoEntrada& e) {
	delete s.juego;
	s.juego = new Motor();
	e = EstadoEntrada();
	s.actual = Pantalla::Juego;
}

void procesarPantalla(const ALLEGRO_EVENT& ev, Sesion& s, EstadoEntrada& e, bool& salir) {
	const bool teclaAbajo = ev.type == ALLEGRO_EVENT_KEY_DOWN;
	const int tecla = ev.keyboard.keycode;
	const bool enter = teclaAbajo && (tecla == ALLEGRO_KEY_ENTER || tecla == ALLEGRO_KEY_PAD_ENTER);
	const bool pausa = teclaAbajo && (tecla == ALLEGRO_KEY_P || tecla == ALLEGRO_KEY_ESCAPE);
	const bool arriba = teclaAbajo && tecla == ALLEGRO_KEY_UP;
	const bool abajo = teclaAbajo && tecla == ALLEGRO_KEY_DOWN;
	const bool izquierda = teclaAbajo && tecla == ALLEGRO_KEY_LEFT;
	const bool derecha = teclaAbajo && tecla == ALLEGRO_KEY_RIGHT;

	switch (s.actual) {
	case Pantalla::Titulo:
		if (enter) s.actual = Pantalla::Menu;
		break;
	case Pantalla::Menu:
		if (arriba) s.opcionMenu = (s.opcionMenu + OPCIONES_MENU - 1) % OPCIONES_MENU;
		if (abajo) s.opcionMenu = (s.opcionMenu + 1) % OPCIONES_MENU;
		if (s.opcionMenu == 0 && izquierda && s.cantidadJugadores > 1) --s.cantidadJugadores;
		if (s.opcionMenu == 0 && derecha && s.cantidadJugadores < MAX_JUGADORES) ++s.cantidadJugadores;
		if (enter) {
			if (s.opcionMenu == 1) {
				for (int i = 0; i < MAX_JUGADORES; ++i) s.nombres[i][0] = '\0';
				s.jugadorRegistrando = 0;
				s.actual = Pantalla::RegistroJugadores;
			}
			else if (s.opcionMenu == 2) {
				s.actual = Pantalla::Estadisticas;
			}
			else if (s.opcionMenu == 3) {
				s.actual = Pantalla::Ranking;
			} 
			else if (s.opcionMenu == 4) {
				salir = true;
			}
		}
		break;

	case Pantalla::RegistroJugadores: {
		char* nombre = s.nombres[s.jugadorRegistrando];
		const int largo = static_cast<int>(strlen(nombre));

		if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
			const int c = ev.keyboard.unichar;
			if (c >= 32 && c <= 126 && largo < LARGO_NOMBRE) {
				nombre[largo] = static_cast<char>(c);
				nombre[largo + 1] = '\0';
			}
		}
		else if (teclaAbajo && tecla == ALLEGRO_KEY_BACKSPACE && largo > 0) {
			nombre[largo - 1] = '\0';
		}
		else if (enter && largo > 0) {
			++s.jugadorRegistrando;
			if (s.jugadorRegistrando == s.cantidadJugadores) {
				s.jugadorActivo = 0;
				s.actual = Pantalla::Turno;
			}
		}
		break;
	}

	case Pantalla::Turno:
		if (enter) iniciarJuego(s, e);
		break;

	case Pantalla::Juego:
		if (pausa) s.actual = Pantalla::Pausa;
		break;

	case Pantalla::Pausa:
		if (pausa) s.actual = Pantalla::Juego;
		break;

	case Pantalla::Resumen:
		if (enter) s.actual = Pantalla::Fin;
		break;

	case Pantalla::Fin:
		if (enter) {
			if (s.jugadorActivo + 1 < s.cantidadJugadores) {
				++s.jugadorActivo;
				s.actual = Pantalla::Turno;
			}
			else {
				s.actual = Pantalla::Estadisticas;
			}
		}
		break;

	case Pantalla::Estadisticas:
		if (enter) s.actual = Pantalla::Ranking;
		break;

	case Pantalla::Ranking:
		if (enter) s.actual = Pantalla::Menu;
		break;
	}
}

void dibujarPantalla(const Sesion& s, const ALLEGRO_FONT* fuente) {
	const ALLEGRO_COLOR claro = colorGB(GB_CLARO);
	const ALLEGRO_COLOR medio = colorGB(GB_MEDIO_CLARO);
	const ALLEGRO_COLOR medioOscuro = colorGB(GB_MEDIO_OSCURO);
	const ALLEGRO_COLOR oscuro = colorGB(GB_OSCURO);
	const bool enJuego = s.actual == Pantalla::Juego || s.actual == Pantalla::Pausa;

	if (enJuego) {
		al_clear_to_color(al_map_rgb(0, 0, 0));
		if (s.juego != nullptr) {
			dibujarMundo(*s.juego);
			dibujarHUD(*s.juego, fuente, s.nombres[s.jugadorActivo]);
		}
	}
	else {
		al_clear_to_color(claro);
		al_draw_rectangle(8.0f, 8.0f, 792.0f, 592.0f, oscuro, 6.0f);
	}

	switch (s.actual) {
	case Pantalla::Titulo: 
		dibujarPantallaTitulo(fuente);
		break;
	case Pantalla::Menu: {
		const char* etiquetas[OPCIONES_MENU] = { "JUGADORES", "JUGAR", "ESTADISTICAS", "RANKING", "SALIR" };
		texto(fuente, 60.0f, oscuro, "FASE ORBITAL");
		for (int i = 0; i < OPCIONES_MENU; ++i) {
			const char* marca = (i == s.opcionMenu) ? "> " : "  ";
			char linea[48];
			if (i == 0) {
				snprintf(linea, sizeof linea, "%s%s < %d >", marca, etiquetas[i], s.cantidadJugadores);
			}
			else {
				snprintf(linea, sizeof linea, "%s%s", marca, etiquetas[i]);
			}
			texto(fuente, 150.0f + i * 40.0f, i == s.opcionMenu ? oscuro : medioOscuro, linea);
		}
		break;
	}

	case Pantalla::RegistroJugadores: {
		char titulo[32];
		snprintf(titulo, sizeof titulo, "JUGADOR %d", s.jugadorRegistrando + 1);
		char linea[LARGO_NOMBRE + 2];
		snprintf(linea, sizeof linea, "%s_", s.nombres[s.jugadorRegistrando]);
		texto(fuente, 120.0f, oscuro, titulo);
		texto(fuente, 180.0f, oscuro, linea);
		texto(fuente, 300.0f, medioOscuro, "ESCRIBE TU NOMBRE");
		texto(fuente, 330.0f, medioOscuro, "ENTER para confirmar");
		break;
	}

	case Pantalla::Turno:
		texto(fuente, 150.0f, medioOscuro, "TURNO DE");
		texto(fuente, 190.0f, oscuro, s.nombres[s.jugadorActivo]);
		texto(fuente, 300.0f, medioOscuro, "ENTER para empezar");
		break;

	case Pantalla::Juego:
		break;

	case Pantalla::Pausa:
		al_draw_filled_rectangle(0.0f, 0.0f, 800.0f, 600.0f, al_map_rgba(0, 0, 0, 160));
		texto(fuente, 280.0f, claro, "PAUSA");
		texto(fuente, 310.0f, claro, "P / ESC para continuar");
		break;

	case Pantalla::Resumen:
		texto(fuente, 100.0f, oscuro, "RESUMEN");
		if (s.juego != nullptr) {
			const Jugador& jugador = s.juego->obtenerJugador();
			al_draw_textf(fuente, oscuro, 400.0f, 160.0f, ALLEGRO_ALIGN_CENTRE,
				"Puntaje: %.0f", jugador.obtenerPuntaje());
			al_draw_textf(fuente, oscuro, 400.0f, 180.0f, ALLEGRO_ALIGN_CENTRE,
				"Combo maximo: %d", jugador.obtenerComboMaximo());
			al_draw_textf(fuente, oscuro, 400.0f, 200.0f, ALLEGRO_ALIGN_CENTRE,
				"Oleada: %d", s.juego->obtenerOleada());
		}
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Fin:
		texto(fuente, 200.0f, oscuro, "FIN DE PARTIDA");
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Estadisticas:
		texto(fuente, 100.0f, oscuro, "ESTADISTICAS");
		texto(fuente, 200.0f, medioOscuro, "Sin datos: falta conectar historial.csv");
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Ranking:
		texto(fuente, 100.0f, oscuro, "RANKING");
		texto(fuente, 200.0f, medioOscuro, "Sin datos: falta conectar historial.csv");
		texto(fuente, 300.0f, medioOscuro, "ENTER para volver al menu");
		break;
	}
}

int main() {
	if (!al_init()) {
		fprintf(stderr, "No se pudo iniciar Allegro.\n");
		return -1;
	}

	al_install_keyboard();
	al_install_mouse();
	al_init_primitives_addon();
	al_init_font_addon();

	ALLEGRO_DISPLAY* display = al_create_display(800, 600);
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
	ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();
	ALLEGRO_FONT* fuente = al_create_builtin_font();

	if (!display || !timer || !cola || !fuente) {
		fprintf(stderr, "No se pudo crear display, timer, cola de eventos o fuente.\n");
		if (fuente) al_destroy_font(fuente);
		if (cola) al_destroy_event_queue(cola);
		if (timer) al_destroy_timer(timer);
		if (display) al_destroy_display(display);
		return -1;
	}

	al_set_window_title(display, "Fase Orbital");

	al_register_event_source(cola, al_get_keyboard_event_source());
	al_register_event_source(cola, al_get_mouse_event_source());
	al_register_event_source(cola, al_get_display_event_source(display));
	al_register_event_source(cola, al_get_timer_event_source(timer));

	al_start_timer(timer);

	Sesion sesion;
	EstadoEntrada entrada;
	bool salir = false;

	while (!salir) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(cola, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			salir = true;
			continue;
		}

		procesarEvento(ev, entrada);
		procesarPantalla(ev, sesion, entrada, salir);

		if (ev.type == ALLEGRO_EVENT_TIMER) {
			if (sesion.actual == Pantalla::Juego && sesion.juego != nullptr) {
				sesion.juego->tick(construirAccion(entrada, sesion.juego->obtenerJugador()));
				if (sesion.juego->terminada()) {
					sesion.actual = Pantalla::Resumen;
				}
			}

			dibujarPantalla(sesion, fuente);
			al_flip_display();
		}
	}

	delete sesion.juego;
	al_stop_timer(timer);
	al_destroy_font(fuente);
	al_destroy_event_queue(cola);
	al_destroy_timer(timer);
	al_destroy_display(display);

	return 0;
}