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
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
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

	// Los valores 0, 1 y 2 coinciden con CIAN, MAGENTA y AMBAR.
	ALLEGRO_COLOR colorDeIndice(int indice) {
		switch (indice) {
		case CIAN:     return al_map_rgb(0, 255, 255);
		case MAGENTA:  return al_map_rgb(255, 0, 255);
		default:       return al_map_rgb(255, 160, 40);
		}
	}

	constexpr double TAU = 6.283185307179586;
	constexpr int MAX_SEGUIMIENTO = 256;
	constexpr int MAX_PARTICULAS = 300;
	constexpr int TICKS_PARTICULA = 15;
	constexpr int PARTICULAS_POR_BAJA = 10;
	constexpr int TIPOS_ENEMIGO = 6;

	// Fuentes TTF por escala (tamano = 8 * escala); se buscan en orden de prioridad.
	ALLEGRO_FONT* fuentes[6] = {};
	const char* RUTAS_FUENTE[] = {
		"fuente.ttf",
		"/System/Library/Fonts/Supplemental/Arial.ttf",
		"/System/Library/Fonts/Helvetica.ttc",
		"C:\\Windows\\Fonts\\arial.ttf"
	};

	void liberarFuentes() {
		for (int escala = 1; escala <= 5; ++escala) {
			al_destroy_font(fuentes[escala]);
			fuentes[escala] = nullptr;
		}
	}

	bool cargarFuentes() {
		for (const char* ruta : RUTAS_FUENTE) {
			bool ok = true;
			for (int escala = 1; escala <= 5 && ok; ++escala) {
				fuentes[escala] = al_load_ttf_font(ruta, 8 * escala, 0);
				ok = fuentes[escala] != nullptr;
			}
			if (ok) return true;
			liberarFuentes();
		}
		return false;
	}

	// Sonidos sintetizados al iniciar: el juego no necesita archivos de audio.
	struct Sonidos {
		ALLEGRO_SAMPLE* disparo = nullptr;
		ALLEGRO_SAMPLE* baja = nullptr;
		ALLEGRO_SAMPLE* error = nullptr;
		ALLEGRO_SAMPLE* dano = nullptr;
		ALLEGRO_SAMPLE* pulso = nullptr;
	};

	Sonidos sonidos;
	bool audioActivo = false;

	// Barrido de frecuencia con envolvente lineal; ruido mezcla una parte aleatoria.
	ALLEGRO_SAMPLE* crearTono(double frecIni, double frecFin, double duracion,
		bool cuadrada, double ruido, float volumen) {
		const unsigned int n = static_cast<unsigned int>(44100.0 * duracion);
		int16_t* buffer = static_cast<int16_t*>(al_malloc(n * sizeof(int16_t)));
		if (buffer == nullptr) return nullptr;
		double fase = 0.0;
		uint32_t lcg = 12345u;
		for (unsigned int i = 0; i < n; ++i) {
			const double t = static_cast<double>(i) / n;
			fase += TAU * (frecIni + (frecFin - frecIni) * t) / 44100.0;
			const double onda = cuadrada ? (sin(fase) >= 0.0 ? 1.0 : -1.0) : sin(fase);
			lcg = lcg * 1664525u + 1013904223u;
			const double azar = static_cast<double>(lcg >> 8) / 8388608.0 - 1.0;
			const double mezcla = onda * (1.0 - ruido) + azar * ruido;
			const double envolvente = (1.0 - t) * (1.0 - t);
			buffer[i] = static_cast<int16_t>(mezcla * envolvente * volumen * 32767.0);
		}
		return al_create_sample(buffer, n, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1, true);
	}

	void cargarSonidos() {
		audioActivo = al_install_audio() && al_reserve_samples(8);
		if (!audioActivo) {
			fprintf(stderr, "Aviso: sin dispositivo de audio, el juego corre sin sonido.\n");
			return;
		}
		sonidos.disparo = crearTono(900.0, 400.0, 0.08, false, 0.0, 0.5f);
		sonidos.baja = crearTono(300.0, 60.0, 0.25, false, 0.6, 0.8f);
		sonidos.error = crearTono(180.0, 160.0, 0.15, true, 0.0, 0.4f);
		sonidos.dano = crearTono(140.0, 50.0, 0.4, true, 0.3, 0.9f);
		sonidos.pulso = crearTono(200.0, 1200.0, 0.4, false, 0.0, 0.6f);
	}

	void reproducir(ALLEGRO_SAMPLE* s) {
		if (audioActivo && s != nullptr) {
			al_play_sample(s, 0.6f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, nullptr);
		}
	}

	void liberarSonidos() {
		al_destroy_sample(sonidos.disparo);
		al_destroy_sample(sonidos.baja);
		al_destroy_sample(sonidos.error);
		al_destroy_sample(sonidos.dano);
		al_destroy_sample(sonidos.pulso);
		if (audioActivo) al_uninstall_audio();
		audioActivo = false;
	}

	struct Instantanea {
		int id;
		int tipo;
		int color;
		float x;
		float y;
	};

	struct Particula {
		float x;
		float y;
		float vx;
		float vy;
		int vida;
		ALLEGRO_COLOR color;
	};

	// Estado de la vista: detecta bajas, disparos y danos comparando ticks sucesivos.
	struct VistaEfectos {
		Instantanea anteriores[MAX_SEGUIMIENTO];
		int cantAnteriores = 0;
		int bajasAnt[TIPOS_ENEMIGO] = {};
		int disparosAnt = 0;
		int erroneosAnt = 0;
		int pulsosAnt = 0;
		int vidaAnt = 0;
		bool inicializado = false;
		Particula particulas[MAX_PARTICULAS];
		int cantParticulas = 0;
		uint32_t semilla = 2463534242u;
	};

	float aleatorio(VistaEfectos& v) {
		v.semilla ^= v.semilla << 13;
		v.semilla ^= v.semilla >> 17;
		v.semilla ^= v.semilla << 5;
		return static_cast<float>(v.semilla % 10000u) / 10000.0f;
	}

	void generarParticulas(VistaEfectos& v, float x, float y, ALLEGRO_COLOR color) {
		for (int k = 0; k < PARTICULAS_POR_BAJA && v.cantParticulas < MAX_PARTICULAS; ++k) {
			const float angulo = aleatorio(v) * 6.2831853f;
			const float velocidad = 1.0f + aleatorio(v) * 3.0f;
			Particula& p = v.particulas[v.cantParticulas++];
			p.x = x;
			p.y = y;
			p.vx = cos(angulo) * velocidad;
			p.vy = sin(angulo) * velocidad;
			p.vida = TICKS_PARTICULA;
			p.color = color;
		}
	}

	bool enLista(const ListaEnemigos& enemigos, int id) {
		for (const NodoEnemigo* a = enemigos.primero(); a != nullptr; a = a->sig) {
			if (a->dato.id == id) return true;
		}
		return false;
	}

	void actualizarEfectos(VistaEfectos& v, const Motor& m) {
		const ListaEnemigos& enemigos = m.obtenerEnemigos();
		int bajas[TIPOS_ENEMIGO];
		for (int t = 0; t < TIPOS_ENEMIGO; ++t) bajas[t] = m.obtenerBajas(t);
		const int disparos = m.obtenerDisparos();
		const int erroneos = m.obtenerErroneos();
		const int pulsos = m.obtenerPulsos();
		const int vida = m.obtenerJugador().obtenerVidaNucleo();

		if (v.inicializado) {
			for (int i = 0; i < v.cantAnteriores; ++i) {
				const Instantanea& a = v.anteriores[i];
				if (!enLista(enemigos, a.id) && bajas[a.tipo] > v.bajasAnt[a.tipo]) {
					generarParticulas(v, a.x, a.y, colorDeIndice(a.color));
				}
			}
			bool huboBaja = false;
			for (int t = 0; t < TIPOS_ENEMIGO; ++t) {
				if (bajas[t] > v.bajasAnt[t]) huboBaja = true;
			}
			if (disparos > v.disparosAnt) reproducir(sonidos.disparo);
			if (huboBaja) reproducir(sonidos.baja);
			if (erroneos > v.erroneosAnt) reproducir(sonidos.error);
			if (vida < v.vidaAnt) reproducir(sonidos.dano);
			if (pulsos > v.pulsosAnt) reproducir(sonidos.pulso);
		}

		v.cantAnteriores = 0;
		for (const NodoEnemigo* n = enemigos.primero(); n != nullptr && v.cantAnteriores < MAX_SEGUIMIENTO; n = n->sig) {
			Instantanea& s = v.anteriores[v.cantAnteriores++];
			s.id = n->dato.id;
			s.tipo = static_cast<int>(n->dato.tipo);
			s.color = n->dato.color;
			posicionEnPantalla(n->dato.r, n->dato.theta, s.x, s.y);
		}
		for (int t = 0; t < TIPOS_ENEMIGO; ++t) v.bajasAnt[t] = bajas[t];
		v.disparosAnt = disparos;
		v.erroneosAnt = erroneos;
		v.pulsosAnt = pulsos;
		v.vidaAnt = vida;
		v.inicializado = true;

		int sobrevivientes = 0;
		for (int i = 0; i < v.cantParticulas; ++i) {
			Particula p = v.particulas[i];
			p.x += p.vx;
			p.y += p.vy;
			p.vx *= 0.9f;
			p.vy *= 0.9f;
			--p.vida;
			if (p.vida > 0) v.particulas[sobrevivientes++] = p;
		}
		v.cantParticulas = sobrevivientes;
	}

	void dibujarPoligono(float cx, float cy, float radio, int lados, float rotacionGrados, ALLEGRO_COLOR color, bool relleno) {
		float pts[12];
		for (int i = 0; i < lados; ++i) {
			const double ang = aRadianes(rotacionGrados + 360.0 * i / lados);
			pts[2 * i] = cx + radio * static_cast<float>(cos(ang));
			pts[2 * i + 1] = cy + radio * static_cast<float>(sin(ang));
		}
		if (relleno) {
			for (int i = 0; i < lados; ++i) {
				const int j = (i + 1) % lados;
				al_draw_filled_triangle(cx, cy, pts[2 * i], pts[2 * i + 1],
					pts[2 * j], pts[2 * j + 1], color);
			}
		}
		else {
			al_draw_polygon(pts, lados, ALLEGRO_LINE_JOIN_ROUND, color, 2.0f, 0.0f);
		}
	}

	// Cada tipo tiene forma propia: triangulo, cuadrado, hexagono, cruz, pentagono y anillo.
	void dibujarEnemigo(const Enemigo& e) {
		float x, y;
		posicionEnPantalla(e.r, e.theta, x, y);
		const ALLEGRO_COLOR c = colorDeIndice(e.color);
		const float r = static_cast<float>(e.radio) * 2.5f;
		const float rot = static_cast<float>(e.theta);
		switch (e.tipo) {
		case TipoEnemigo::Dron:
			dibujarPoligono(x, y, r, 3, rot, c, true);
			break;
		case TipoEnemigo::Girador:
			dibujarPoligono(x, y, r, 4, rot + 45.0f, c, true);
			break;
		case TipoEnemigo::Divisor:
			dibujarPoligono(x, y, r, 6, rot, c, true);
			break;
		case TipoEnemigo::Mini:
			al_draw_line(x - r, y, x + r, y, c, 2.0f);
			al_draw_line(x, y - r, x, y + r, c, 2.0f);
			break;
		case TipoEnemigo::Tanque:
			dibujarPoligono(x, y, r, 5, rot, c, true);
			dibujarPoligono(x, y, r + 3.0f, 5, rot, al_map_rgb(120, 220, 255), false);
			break;
		case TipoEnemigo::Camaleon:
			al_draw_circle(x, y, r, c, 2.0f);
			al_draw_filled_circle(x, y, r / 2.0f, c);
			break;
		}
	}

	void dibujarParticulas(const VistaEfectos& v) {
		for (int i = 0; i < v.cantParticulas; ++i) {
			const Particula& p = v.particulas[i];
			al_draw_filled_circle(p.x, p.y, 1.0f + p.vida / 6.0f, p.color);
		}
	}

	// El arco exterior muestra la vida restante del nucleo.
	void dibujarNucleo(int vida) {
		al_draw_circle(CENTRO_X, CENTRO_Y, RADIO_ORBITA, al_map_rgb(60, 45, 130), 1.0f);
		al_draw_filled_circle(CENTRO_X, CENTRO_Y, RADIO_NUCLEO, al_map_rgb(120, 220, 255));
		const float fraccion = static_cast<float>(vida) / 100.0f;
		al_draw_arc(CENTRO_X, CENTRO_Y, RADIO_NUCLEO + 8.0f, -1.5707963f,
			fraccion * 6.2831853f, al_map_rgb(170, 130, 255), 3.0f);
	}

	void dibujarPulso(const Pulso& pulso) {
		if (pulso.activo) {
			al_draw_circle(CENTRO_X, CENTRO_Y,
				static_cast<float>(pulso.radio_actual),
				al_map_rgb(170, 130, 255), 2.0f);
		}
	}

	void dibujarEnemigos(const ListaEnemigos& enemigos) {
		for (const NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			dibujarEnemigo(actual->dato);
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
		dibujarPoligono(x, y, TAMANO_NAVE + 3.0f, 3, static_cast<float>(jugador.obtenerPosicionAngular()),
			colorDeIndice(jugador.obtenerColorActual()), true);
	}
}

void dibujarMundo(const Motor& motor, const VistaEfectos& efectos) {
	dibujarNucleo(motor.obtenerJugador().obtenerVidaNucleo());
	dibujarPulso(motor.obtenerPulso());
	dibujarEnemigos(motor.obtenerEnemigos());
	dibujarBalas(motor.obtenerBalas());
	dibujarNave(motor.obtenerJugador());
	dibujarParticulas(efectos);
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
			e.colorSeleccionado = AMBAR; e.colorPedido = true; break;
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

// ============================================================
// PANTALLAS 
// ============================================================

constexpr int MAX_JUGADORES = 4;
constexpr int LARGO_NOMBRE = 10;
constexpr int OPCIONES_MENU = 5;

enum class Pantalla { Titulo, Menu, RegistroJugadores, Turno, Juego, Pausa, Resumen, Fin, Estadisticas, Ranking };

enum IndiceGB { GB_OSCURO, GB_MEDIO_OSCURO, GB_MEDIO_CLARO, GB_CLARO };

// Paleta espacial: azul noche, indigo, lavanda y azul hielo.
ALLEGRO_COLOR colorGB(int indice) {
	switch (indice) {
	case GB_OSCURO:       return al_map_rgb(8, 6, 28);
	case GB_MEDIO_OSCURO: return al_map_rgb(40, 28, 95);
	case GB_MEDIO_CLARO:  return al_map_rgb(170, 130, 255);
	default:              return al_map_rgb(120, 220, 255);
	}
}

const int TAM_PIXEL = 8;

// Glifos 5x7: cada fila es un patron de 5 bits (bit alto = columna izquierda).
struct Glifo {
	char caracter;
	unsigned char filas[7];
};

const Glifo GLIFOS[] = {
	{' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
	{'!', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100}},
	{'#', {0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010}},
	{'%', {0b11001, 0b11010, 0b00010, 0b00100, 0b01000, 0b01011, 0b10011}},
	{'(', {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010}},
	{')', {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000}},
	{'+', {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000}},
	{'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
	{'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100}},
	{'/', {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000}},
	{'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
	{'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
	{'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
	{'3', {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110}},
	{'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
	{'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
	{'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
	{'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}},
	{'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
	{'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
	{':', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000}},
	{'=', {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000}},
	{'?', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100}},
	{'A', {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
	{'B', {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}},
	{'C', {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}},
	{'D', {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100}},
	{'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
	{'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
	{'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111}},
	{'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
	{'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
	{'J', {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100}},
	{'K', {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}},
	{'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
	{'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
	{'N', {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001}},
	{'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
	{'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
	{'Q', {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}},
	{'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
	{'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
	{'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
	{'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
	{'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}},
	{'W', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}},
	{'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}},
	{'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
	{'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
	{'|', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
};

char mayuscula(char c) {
	return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

const unsigned char* buscarGlifo(char c) {
	for (const Glifo& g : GLIFOS) {
		if (g.caracter == c) return g.filas;
	}
	return nullptr;
}

ALLEGRO_FONT* fuenteParaEscala(int escala) {
	return fuentes[escala < 1 ? 1 : (escala > 5 ? 5 : escala)];
}

float anchoTexto(const char* t, int escala) {
	return al_get_text_width(fuenteParaEscala(escala), t);
}

void dibujarTextoPixel(const char* t, float x, float y, int escala, ALLEGRO_COLOR color) {
	al_draw_text(fuenteParaEscala(escala), color, x, y, ALLEGRO_ALIGN_LEFT, t);
}

void textoCentrado(const char* t, float y, int escala, ALLEGRO_COLOR color) {
	dibujarTextoPixel(t, (800.0f - anchoTexto(t, escala)) / 2.0f, y, escala, color);
}

void tituloCentrado(const char* t, float y, int escala) {
	const float x = (800.0f - anchoTexto(t, escala)) / 2.0f;
	dibujarTextoPixel(t, x + escala, y + escala, escala, colorGB(GB_MEDIO_CLARO));
	dibujarTextoPixel(t, x, y, escala, colorGB(GB_CLARO));
}

void dibujarPanel(float x1, float y1, float x2, float y2, ALLEGRO_COLOR borde) {
	al_draw_filled_rounded_rectangle(x1, y1, x2, y2, 10.0f, 10.0f, colorGB(GB_MEDIO_OSCURO));
	al_draw_rounded_rectangle(x1, y1, x2, y2, 10.0f, 10.0f, borde, 2.0f);
}

void dibujarEstrellas() {
	for (int i = 0; i < 90; ++i) {
		const float x = static_cast<float>((i * 197 + 37) % 800);
		const float y = static_cast<float>((i * 131 + 59) % 600);
		al_draw_filled_circle(x, y, i % 3 == 0 ? 2.0f : 1.0f, colorGB(GB_MEDIO_CLARO));
	}
}

// Un jugador aparece una sola vez: se cuenta en su primera aparicion.
bool esPrimeraAparicion(const historialPartidas& h, const NodoRegistroPartida* a) {
	for (const NodoRegistroPartida* p = h.primero(); p != a; p = p->sig) {
		if (p->dato.jugador == a->dato.jugador) return false;
	}
	return true;
}

int contarJugadoresHistorial(const historialPartidas& h) {
	int total = 0;
	for (const NodoRegistroPartida* a = h.primero(); a != nullptr; a = a->sig) {
		if (esPrimeraAparicion(h, a)) ++total;
	}
	return total;
}

const NodoRegistroPartida* primerRegistroDeJugador(const historialPartidas& h, int pagina) {
	int indice = 0;
	for (const NodoRegistroPartida* a = h.primero(); a != nullptr; a = a->sig) {
		if (!esPrimeraAparicion(h, a)) continue;
		if (indice == pagina) return a;
		++indice;
	}
	return nullptr;
}

void dibujarHUD(const Motor& motor, const char* nombreJugador) {
	const Jugador& jugador = motor.obtenerJugador();
	const ALLEGRO_COLOR color = colorGB(GB_CLARO);
	char linea[64];

	snprintf(linea, sizeof linea, "JUGADOR: %s", nombreJugador);
	dibujarTextoPixel(linea, 16.0f, 16.0f, 2, color);
	snprintf(linea, sizeof linea, "PUNTAJE: %.0f", jugador.obtenerPuntaje());
	dibujarTextoPixel(linea, 16.0f, 40.0f, 2, color);
	snprintf(linea, sizeof linea, "MULTIPLICADOR: X%d", jugador.obtenerComboActual());
	dibujarTextoPixel(linea, 16.0f, 64.0f, 2, color);
	snprintf(linea, sizeof linea, "OLEADA: %d", motor.obtenerOleada());
	dibujarTextoPixel(linea, 16.0f, 88.0f, 2, color);
	snprintf(linea, sizeof linea, "VIDA NUCLEO: %d", jugador.obtenerVidaNucleo());
	dibujarTextoPixel(linea, 16.0f, 112.0f, 2, color);

	const float carga = jugador.obtenerCargaPulse();
	snprintf(linea, sizeof linea, "CARGA PULSO: %.0f%%", carga);
	dibujarTextoPixel(linea, 16.0f, 136.0f, 2, color);
	al_draw_rectangle(16.0f, 162.0f, 216.0f, 174.0f, colorGB(GB_MEDIO_CLARO), 2.0f);
	al_draw_filled_rectangle(16.0f, 162.0f, 16.0f + carga * 2.0f, 174.0f, colorGB(GB_MEDIO_CLARO));
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

// Pantalla de inicio: título y aviso sobre el fondo estrellado.
void dibujarPantallaTitulo() {
	tituloCentrado("FASE ORBITAL", 230.0f, 5);
	textoCentrado("PRESIONA ENTER", 400.0f, 2, colorGB(GB_MEDIO_CLARO));
}

struct Sesion {
	Pantalla actual = Pantalla::Titulo;
	int opcionMenu = 0;
	int cantidadJugadores = 1;
	int jugadorRegistrando = 0;
	int jugadorActivo = 0;
	int paginaEstadisticas = 0;
	char nombres[MAX_JUGADORES][LARGO_NOMBRE + 1] = {};
	Motor* juego = nullptr;
	RegistroPartida resultados[MAX_JUGADORES];
	VistaEfectos efectos;
};

// Por especificacion, todo alias que empieza con BOT es controlado por el bot.
bool esBot(const char* nombre) {
	return std::strncmp(nombre, "BOT", 3) == 0;
}

// Copia el resultado del jugador activo al terminar su partida.
void capturarResultado(Sesion& s) {
	if (s.juego == nullptr) return;
	const Motor& m = *s.juego;
	RegistroPartida& r = s.resultados[s.jugadorActivo];
	r.semilla = static_cast<int>(m.obtenerSemilla());
	r.modo = "interactivo";
	r.jugador = s.nombres[s.jugadorActivo];
	r.es_bot = esBot(s.nombres[s.jugadorActivo]) ? 1 : 0;
	r.puntaje = static_cast<int>(m.obtenerJugador().obtenerPuntaje());
	r.oleada = m.obtenerOleada();
	r.disparos = m.obtenerDisparos();
	r.correctos = m.obtenerCorrectos();
	r.erroneos = m.obtenerErroneos();
	r.combo_max = m.obtenerJugador().obtenerComboMaximo();
	r.pulsos = m.obtenerPulsos();
	r.dano_total = m.obtenerDanoTotal();
	r.ticks = m.obtenerTickActual();
	r.bajas_dron = m.obtenerBajas(0);
	r.bajas_girador = m.obtenerBajas(1);
	r.bajas_divisor = m.obtenerBajas(2);
	r.bajas_mini = m.obtenerBajas(3);
	r.bajas_tanque = m.obtenerBajas(4);
	r.bajas_camaleon = m.obtenerBajas(5);
}

// Guarda una fila por jugador en historial.csv al terminar la sesion.
void guardarSesion(Sesion& s, historialPartidas& historial) {
	const int idPartida = siguienteIdPartida(historial);
	for (int i = 0; i < s.cantidadJugadores; ++i) {
		RegistroPartida r = s.resultados[i];
		r.id_partida = idPartida;
		r.posicion = 1;
		for (int j = 0; j < s.cantidadJugadores; ++j) {
			if (s.resultados[j].puntaje > r.puntaje) ++r.posicion;
		}
		historial.registroFinal(r);
	}
	historial.guardarArchivo(RUTA_HISTORIAL);
}

void iniciarJuego(Sesion& s, EstadoEntrada& e) {
	delete s.juego;
	s.juego = new Motor();
	s.efectos = VistaEfectos();
	e = EstadoEntrada();
	s.actual = Pantalla::Juego;
}

void procesarPantalla(const ALLEGRO_EVENT& ev, Sesion& s, EstadoEntrada& e, bool& salir, historialPartidas& historial) {
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
				s.paginaEstadisticas = 0;
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
				guardarSesion(s, historial);
				s.paginaEstadisticas = 0;
				s.actual = Pantalla::Estadisticas;
			}
		}
		break;

	case Pantalla::Estadisticas: {
		const int total = contarJugadoresHistorial(historial);
		if (derecha && s.paginaEstadisticas + 1 < total) ++s.paginaEstadisticas;
		if (izquierda && s.paginaEstadisticas > 0) --s.paginaEstadisticas;
		if (enter) s.actual = Pantalla::Ranking;
		break;
	}

	case Pantalla::Ranking:
		if (enter) s.actual = Pantalla::Menu;
		break;
	}
}

void dibujarPantalla(const Sesion& s, const historialPartidas& historial) {
	const bool enJuego = s.actual == Pantalla::Juego || s.actual == Pantalla::Pausa;

	al_clear_to_color(colorGB(GB_OSCURO));
	dibujarEstrellas();
	if (enJuego) {
		if (s.juego != nullptr) {
			dibujarMundo(*s.juego, s.efectos);
			dibujarHUD(*s.juego, s.nombres[s.jugadorActivo]);
		}
	}
	else {
		dibujarMarco();
	}

	switch (s.actual) {
	case Pantalla::Titulo:
		dibujarPantallaTitulo();
		break;
	case Pantalla::Menu: {
		const char* etiquetas[OPCIONES_MENU] = { "JUGADORES", "JUGAR", "ESTADISTICAS", "RANKING", "SALIR" };
		tituloCentrado("FASE ORBITAL", 60.0f, 4);
		for (int i = 0; i < OPCIONES_MENU; ++i) {
			const float y = 180.0f + i * 64.0f;
			const bool seleccionada = i == s.opcionMenu;
			dibujarPanel(230.0f, y, 570.0f, y + 44.0f, seleccionada ? colorGB(GB_CLARO) : colorGB(GB_MEDIO_CLARO));
			char linea[48];
			if (i == 0) {
				snprintf(linea, sizeof linea, "JUGADORES  < %d >", s.cantidadJugadores);
			}
			else {
				snprintf(linea, sizeof linea, "%s", etiquetas[i]);
			}
			textoCentrado(linea, y + 15.0f, 2, seleccionada ? colorGB(GB_CLARO) : colorGB(GB_MEDIO_CLARO));
		}
		break;
	}

	case Pantalla::RegistroJugadores: {
		char titulo[32];
		snprintf(titulo, sizeof titulo, "JUGADOR %d", s.jugadorRegistrando + 1);
		char linea[LARGO_NOMBRE + 2];
		snprintf(linea, sizeof linea, "%s_", s.nombres[s.jugadorRegistrando]);
		tituloCentrado(titulo, 120.0f, 3);
		dibujarPanel(200.0f, 200.0f, 600.0f, 260.0f, colorGB(GB_MEDIO_CLARO));
		textoCentrado(linea, 220.0f, 3, colorGB(GB_CLARO));
		textoCentrado("ESCRIBE TU NOMBRE", 300.0f, 2, colorGB(GB_MEDIO_CLARO));
		textoCentrado("ENTER PARA CONFIRMAR", 340.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;
	}

	case Pantalla::Turno:
		textoCentrado("TURNO DE", 170.0f, 2, colorGB(GB_MEDIO_CLARO));
		tituloCentrado(s.nombres[s.jugadorActivo], 220.0f, 4);
		textoCentrado("ENTER PARA EMPEZAR", 340.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;

	case Pantalla::Juego:
		break;

	case Pantalla::Pausa:
		al_draw_filled_rectangle(0.0f, 0.0f, 800.0f, 600.0f, al_map_rgba(8, 6, 28, 190));
		tituloCentrado("PAUSA", 240.0f, 4);
		textoCentrado("P / ESC PARA CONTINUAR", 320.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;

	case Pantalla::Resumen:
		tituloCentrado("RESUMEN", 80.0f, 3);
		dibujarPanel(200.0f, 150.0f, 600.0f, 330.0f, colorGB(GB_MEDIO_CLARO));
		if (s.juego != nullptr) {
			const Jugador& jugador = s.juego->obtenerJugador();
			char linea[64];
			snprintf(linea, sizeof linea, "PUNTAJE: %.0f", jugador.obtenerPuntaje());
			textoCentrado(linea, 180.0f, 2, colorGB(GB_CLARO));
			snprintf(linea, sizeof linea, "COMBO MAXIMO: %d", jugador.obtenerComboMaximo());
			textoCentrado(linea, 225.0f, 2, colorGB(GB_CLARO));
			snprintf(linea, sizeof linea, "OLEADA: %d", s.juego->obtenerOleada());
			textoCentrado(linea, 270.0f, 2, colorGB(GB_CLARO));
		}
		textoCentrado("ENTER PARA CONTINUAR", 420.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;

	case Pantalla::Fin:
		tituloCentrado("FIN DE PARTIDA", 240.0f, 3);
		textoCentrado("ENTER PARA CONTINUAR", 340.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;

	case Pantalla::Estadisticas: {
		tituloCentrado("ESTADISTICAS", 30.0f, 3);
		const int total = contarJugadoresHistorial(historial);
		if (total == 0) {
			textoCentrado("SIN DATOS EN HISTORIAL.CSV", 260.0f, 2, colorGB(GB_MEDIO_CLARO));
		}
		else {
			char linea[96];
			snprintf(linea, sizeof linea, "JUGADOR %d DE %d", s.paginaEstadisticas + 1, total);
			textoCentrado(linea, 90.0f, 2, colorGB(GB_MEDIO_CLARO));
			dibujarPanel(40.0f, 120.0f, 760.0f, 530.0f, colorGB(GB_MEDIO_CLARO));

			const NodoRegistroPartida* nodo = primerRegistroDeJugador(historial, s.paginaEstadisticas);
			ResumenJugador r;
			if (nodo != nullptr && resumenJugador(historial, nodo->dato.jugador, r)) {
				snprintf(linea, sizeof linea, "JUGADOR: %s", r.jugador.c_str());
				dibujarTextoPixel(linea, 70.0f, 146.0f, 2, colorGB(GB_CLARO));
				snprintf(linea, sizeof linea, "PARTIDAS %d   VICTORIAS %d", r.partidas, r.victorias);
				dibujarTextoPixel(linea, 70.0f, 170.0f, 2, colorGB(GB_CLARO));
				snprintf(linea, sizeof linea, "MEJOR %d   PEOR %d   PRECISION %.1f%%", r.mejor, r.peor, r.precisionPromedio);
				dibujarTextoPixel(linea, 70.0f, 194.0f, 2, colorGB(GB_CLARO));
				snprintf(linea, sizeof linea, "MEDIA %.1f   MEDIANA %.1f   DESV %.1f", r.media, r.mediana, r.desviacion);
				dibujarTextoPixel(linea, 70.0f, 218.0f, 2, colorGB(GB_CLARO));

				int maximo = 1;
				for (int b = 0; b < 10; ++b) {
					if (r.histograma[b] > maximo) maximo = r.histograma[b];
				}
				const float ancho = (r.mejor - r.peor) / 10.0f;
				for (int b = 0; b < 10; ++b) {
					char barra[32];
					const int largo = r.histograma[b] * 30 / maximo;
					for (int k = 0; k < largo; ++k) barra[k] = '#';
					barra[largo] = '\0';
					snprintf(linea, sizeof linea, "%5.0f-%-5.0f |%-30s %d",
						r.peor + b * ancho, r.peor + (b + 1) * ancho, barra, r.histograma[b]);
					dibujarTextoPixel(linea, 70.0f, 260.0f + b * 22.0f, 2, colorGB(GB_CLARO));
				}
			}
		}
		textoCentrado("IZQ / DER CAMBIAR JUGADOR   ENTER PARA RANKING", 556.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;
	}

	case Pantalla::Ranking: {
		tituloCentrado("RANKING", 40.0f, 3);
		Ranking ranking;
		historialRanking(historial, ranking);
		if (ranking.vacio()) {
			textoCentrado("SIN DATOS EN HISTORIAL.CSV", 260.0f, 2, colorGB(GB_MEDIO_CLARO));
		}
		else {
			dibujarPanel(180.0f, 110.0f, 620.0f, 480.0f, colorGB(GB_MEDIO_CLARO));
			EntradaRanking top[10];
			const int n = ranking.copiaDatos(10, top);
			for (int i = 0; i < n; ++i) {
				char linea[64];
				snprintf(linea, sizeof linea, "%2d. %-10s %8d", i + 1, top[i].nombre.c_str(), top[i].puntaje);
				dibujarTextoPixel(linea, 210.0f, 136.0f + i * 34.0f, 2, colorGB(GB_CLARO));
			}
		}
		textoCentrado("ENTER PARA VOLVER AL MENU", 520.0f, 2, colorGB(GB_MEDIO_CLARO));
		break;
	}
	}
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		OpcionesLotes op;
		if (!leerArgumentos(argc, argv, op)) return 1;
		if (op.lotes) return ejecutarModoLotes(op);
		if (op.rutaEstadisticas != "") return ejecutarEstadisticas(op.rutaEstadisticas);
		if (op.rutaRanking != "") return ejecutarRanking(op.rutaRanking);
		cout << "Uso: sin argumentos abre la ventana; --lotes, --estadisticas o --ranking para consola" << endl;
		return 1;
	}

	if (!al_init()) {
		fprintf(stderr, "No se pudo iniciar Allegro.\n");
		return -1;
	}

	al_install_keyboard();
	al_install_mouse();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_acodec_addon();

	if (!cargarFuentes()) {
		fprintf(stderr, "Falta el recurso: fuente TTF (fuente.ttf o una fuente del sistema).\n");
		return -1;
	}
	cargarSonidos();

	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	ALLEGRO_DISPLAY* display = al_create_display(1200, 900);
	ALLEGRO_BITMAP* lienzo = al_create_bitmap(800, 600);
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
	ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();

	if (!display || !lienzo || !timer || !cola) {
		fprintf(stderr, "No se pudo crear display, lienzo, timer o cola de eventos.\n");
		if (cola) al_destroy_event_queue(cola);
		if (timer) al_destroy_timer(timer);
		if (lienzo) al_destroy_bitmap(lienzo);
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

	historialPartidas historial;
	historial.cargaHistorial(RUTA_HISTORIAL);

	while (!salir) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(cola, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			salir = true;
			continue;
		}
		if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			al_acknowledge_resize(display);
		}

		procesarEvento(ev, entrada);
		procesarPantalla(ev, sesion, entrada, salir, historial);

		if (ev.type == ALLEGRO_EVENT_TIMER) {
			if (sesion.actual == Pantalla::Juego && sesion.juego != nullptr) {
				if (esBot(sesion.nombres[sesion.jugadorActivo])) {
					sesion.juego->tickBot();
				}
				else {
					sesion.juego->tick(construirAccion(entrada, sesion.juego->obtenerJugador()));
				}
				actualizarEfectos(sesion.efectos, *sesion.juego);
				if (sesion.juego->terminada()) {
					capturarResultado(sesion);
					sesion.actual = Pantalla::Resumen;
				}
			}

			al_set_target_bitmap(lienzo);
			dibujarPantalla(sesion, historial);

			al_set_target_backbuffer(display);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			const float ancho = static_cast<float>(al_get_display_width(display));
			const float alto = static_cast<float>(al_get_display_height(display));
			const float escala = std::fmin(ancho / 800.0f, alto / 600.0f);
			const float ox = (ancho - 800.0f * escala) / 2.0f;
			const float oy = (alto - 600.0f * escala) / 2.0f;
			al_draw_scaled_bitmap(lienzo, 0.0f, 0.0f, 800.0f, 600.0f, ox, oy, 800.0f * escala, 600.0f * escala, 0);
			al_flip_display();
		}
	}

	delete sesion.juego;
	al_stop_timer(timer);
	al_destroy_event_queue(cola);
	al_destroy_timer(timer);
	al_destroy_bitmap(lienzo);
	al_destroy_display(display);
	liberarSonidos();
	liberarFuentes();

	return 0;
}