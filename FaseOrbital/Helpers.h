#pragma once

#include <cmath>
#include <iostream>
#include <cstdint>

using namespace std;

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

inline std::uint64_t semillaOleada(int oleada, std::uint32_t semilla) {
	return static_cast<std::uint64_t>(semilla) * 1000ULL
		+ static_cast<std::uint32_t>(oleada);
}

// Este bool indica si la ronda es impar, si esto se cumple realiza un recorrido de tipo "horario". Por otro lado,
// si las rondas son pares realiza un recorrido "antihorario".
bool rondaimpar(int rondaactual) {
	return rondaactual % 2 == 1;
}

int separacionCampos(string linea, string campos[], int maximo) {
	int cantidad = 0;
	string actual = "";
	for (int i = 0; i < static_cast<int>(linea.size()); i++) {
		if (linea[i] == ',') {
			if (cantidad < maximo) {
				campos[cantidad] = actual;
			}
			cantidad++;
			actual = "";
		}
		else if (linea[i] != '\r') {
			actual = actual + linea[i];
		}
	}
	if (cantidad < maximo) {
		campos[cantidad] = actual;
	}
	cantidad++;
	return cantidad;
}
bool valorenteroValido(string entero) {
	if (entero.size() == 0 || entero.size() > 9) {
		return false;
	}
	for (int i = 0; i < static_cast<int>(entero.size()); i++) {
		if (entero[i] < '0' || entero[i] > '9') {
			return false;
		}
	}
	return true;
}
int valorEntero(string entero) {
	int valor = 0;
	for (int i = 0; i < static_cast<int>(entero.size()); i++) {
		valor = valor * 10 + (entero[i] - '0');
	}
	return valor;
}