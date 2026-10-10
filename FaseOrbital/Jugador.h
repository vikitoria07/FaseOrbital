enum Color { CIAN=0, MAGENTA=1, AMARILLO=2};

class Jugador {
private:
	int identificador;
	float posicion_angular;
	Color color_actual;
	float carga_pulse;
	float puntaje;
	int combo_actual;
	int combo_maximo;
	bool eliminado;

public:
	// Constructor: Inicializa los atributos del jugador con valores predeterminados.
	Jugador() :
		identificador(0),
		posicion_angular(0.0f),
		color_actual(Color::MAGENTA),
		carga_pulse(100),
		puntaje(0),
		combo_actual(0),
		combo_maximo(0),
		eliminado(false) {
	}

	// GETTERS
	int obtenerIdentificador() const {
		return identificador;
	}
	float obtenerPosicionAngular() const {
		return posicion_angular;
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
	void cambiarPosicionAngular(float nueva_posicion) {
		posicion_angular = nueva_posicion;
	}
	void cambiarColorActual(Color nuevo_color) {
		color_actual = nuevo_color;
	}

	void aumentarPulso(float incremento) {
		carga_pulse += incremento;
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
};
