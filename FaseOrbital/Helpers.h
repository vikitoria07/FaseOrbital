#include <cmath>
#include <iostream>

constexpr double PI = 3.141592;

float normalizarAngulo(float angulo) {
	while (angulo < 0.0f) {
		angulo += 360.0f;
	}

	while (angulo >= 360.0f) {
		angulo -= 360.0f;
	}

	return angulo;
}

// Este void es para imprimir si el resultado se cumplio y si no se muestra error.
void reportar(const char* id, const char* caso, bool cumplio) {
	cout << id << " " << caso << ": ";
	if (cumplio == true) {
		cout << "resultado esperado\n";
	}
	else {
		cout << "error\n";
	}
}

double aRadianes(double grados) {
	return grados * PI / 180.0;
}

double distancia(double x1, double y1, double x2, double y2) {
	double dx = x1 - x2;
	double dy = y1 - y2;
	return std::sqrt(dx * dx + dy * dy);
}

int calcMultiplicador(int combo) {
	int multiplicador = 1 + combo / 6;

	if (multiplicador > 8) {
		multiplicador = 8;
	}
	
	return multiplicador;

}

double velocidadBaseEnemigo(int oleada) {
	return 0.8 + 0.08 * oleada;
}

int numEnemigos(int oleada) {
	return 6 + 3 * oleada;
}

long long semillaOleada(int oleada, int semilla) {
	return semilla * 1000LL + oleada;
}