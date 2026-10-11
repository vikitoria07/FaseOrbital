enum Color { CIAN=0, MAGENTA=1, AMARILLO=2};
#include <cmath>

class Jugador {
private:
	string nombre;
	int identificador;
	float posicion_angular;
	Color color_actual;
	float carga_pulse;
	float puntaje;
	int combo_actual;
	int combo_maximo;
	bool eliminado;
	int vida_nucleo;

	static float normalizarAngulo(float angulo) {
		while (angulo < 0.0f) {
			angulo += 360.0f;
		}

		while (angulo >= 360.0f) {
			angulo -= 360.0f;
		}

		return angulo;
}

public:
	// Constructor: Inicializa los atributos del jugador con valores predeterminados.
	Jugador() :
		nombre(""),
		identificador(0),
		posicion_angular(270.0f),
		color_actual(Color::MAGENTA),
		carga_pulse(100),
		puntaje(0),
		combo_actual(0),
		combo_maximo(0),
		eliminado(false),
		vida_nucleo(100)
	{}

	// GETTERS
	int obtenerIdentificador() const {
		return identificador;
	}
	float obtenerPosicionAngular() const {
		return posicion_angular;
	}
	int obtenerVidaNucleo() const {
		return vida_nucleo;
	}
	int obtenerColorActual() const {
		return color_actual;
	}
	float obtenerCargaPulse() const {
		return carga_pulse;
	}
	float obtenerPuntaje() const {
		return puntaje;
	}
	int obtenerComboActual() const {
		return combo_actual;
	}
	int obtenerComboMaximo() const {
		return combo_maximo;
	}
	bool estaEliminado() const {
		return eliminado;
	}

	// SETTERS

	void ponerNombre(string n) {
		nombre = n;
	}
	void cambiarPosicionAngular(float nueva_posicion) {
		posicion_angular = nueva_posicion;
	}
	void cambiarColorActual(Color nuevo_color) {
		color_actual = nuevo_color;
	}

	void aumentarPulso(float incremento) {
		carga_pulse += incremento;

		if (carga_pulse > 100.0f) {
			carga_pulse = 100;
		}
	}
	void resetearPulso() {
		carga_pulse = 0.0f;
	}
	void aumentarPuntaje(float puntos) {
		puntaje += puntos;
	}
	void incrementarCombo() {
		combo_actual++;
		if (combo_actual > combo_maximo) {
			combo_maximo = combo_actual;
		}
	}
	void resetearCombo() {
		combo_actual = 0;
	}
	// SOLO CUANDO PIERDE LA PARTIDA, SE REINICIAN TODOS LOS ATRIBUTOS DEL JUGADOR A SUS VALORES INICIALES.
	void reiniciarJugador() {
		posicion_angular = 0.0f;
		color_actual = Color::MAGENTA;
		carga_pulse = 100.0f;
		puntaje = 0.0f;
		combo_actual = 0;
		combo_maximo = 0;
		eliminado = false;
	}
	void cambiarComboMaximo(int nuevo_combo_maximo) {
		combo_maximo = nuevo_combo_maximo;
	}
	void eliminarJugador() {
		eliminado = true;
	}

	void girarHacia(float angulo) {
		angulo = normalizarAngulo(angulo);

		float dif = angulo - obtenerPosicionAngular();

		if (dif > 180.0f) {
			dif -= 360.0f;
		}
		else if (dif < -180.0f) {
			dif += 360.0f;
		}

		if (std::fabs(dif) <= 6.0f) {
			cambiarPosicionAngular(angulo);
		}
		else if (dif > 0.0f) {
			cambiarPosicionAngular(posicion_angular + 6.0f);
		}
		else {
			cambiarPosicionAngular(posicion_angular - 6.0f);
		}
	}

	void recibirDanno(int danno) {
		vida_nucleo -= danno;
	}
	void reiniciarNucleo() {
		vida_nucleo = 100;
	}
};
