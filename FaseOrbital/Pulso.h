#pragma once

#include "Jugador.h"
#include "Enemigos.h"

struct Pulso {
	bool activo;
	double radio_actual;
};

void activarPulso(Pulso& pulso, Jugador& jugador) {
	if (pulso.activo) {
		return;
	}

	if (jugador.obtenerCargaPulse() >= 100.0f) {
		pulso.activo = true;
		pulso.radio_actual = 20.0;
		jugador.resetearPulso();

		cout << "[PULSO] Activado. Carga: " << jugador.obtenerCargaPulse() << endl;
	}
}

void avanzarPulso(Pulso& pulso, ListaEnemigos& enemigos, Jugador& jugador) {

	if (!pulso.activo) {
		return;
	}

	pulso.radio_actual += 12.0;

	cout << "'[PULSO] Radio actual: " << pulso.radio_actual << endl;

	NodoEnemigo* actual = enemigos.primero();

	while (actual != nullptr) {
		NodoEnemigo* siguiente = actual->sig;

		double diferencia = fabs(actual->dato.r - pulso.radio_actual);

		if (diferencia < actual->dato.radio + 3.0) {

			cout << "[PULSO] Enemigo destruido " << endl;
			int puntosBase = puntosPorTipo(actual->dato.tipo);
			jugador.aumentarPuntaje(puntosBase);
			enemigos.eliminar(actual);
		}

		actual = siguiente;
	}
	if (pulso.radio_actual > 320.0) {
		pulso.activo = false;
		pulso.radio_actual = 0.0;

		cout << "[PULSO] Onda finalizada" << endl;
	}
}