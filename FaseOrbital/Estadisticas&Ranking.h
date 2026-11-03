#pragma once
#include "Bitacora.h"

// El siguiente struct es para establecer donde se van a guardar  los datos sobre el ranking de cada jugador
struct EntradaRanking {
	string nombre;
	int puntaje;
	double precision;
};

struct NodoRanking {
	EntradaRanking dato;
	NodoRanking* sig;
};

//En este bool se comparan dos puntajes, compara de primero el puntaje mayor y si estan iguales sigue con la precisión y si quedan iguales se compara con el nombre en orden alfabético.
bool comparacionPuntajes(const EntradaRanking& M, const EntradaRanking& m) {
	if (M.puntaje != m.puntaje) {
		return M.puntaje > m.puntaje;
	}
	if (fabs(M.precision - m.precision) > 0.000001) {
		return M.precision > m.precision;
	}
	return M.nombre < m.nombre;
};

// La siguiente clase es de ranking la cual esta construida con una lista simple, utilizando los nodos creados anteriormente.
//Además, usa posiciónNueva para recorrer la lista hasta encontrar la posición solicitada.
class Ranking {
	NodoRanking* cabeza;
	int tam;
public:
	Ranking() {
		cabeza = nullptr;
		tam = 0;
	}
	~Ranking() {
		NodoRanking* Aux = cabeza;
		while (Aux != nullptr) {
			NodoRanking* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	Ranking(const Ranking&) = delete;
	Ranking& operator=(const Ranking&) = delete;
	// Esta es la función antes mencionada, la cual por medio de punteros ubica la entrada nueva en la posición que se establecio en posicionNueva
	void posicionNueva(const EntradaRanking& e) {
		NodoRanking* nuevo = new(NodoRanking);
		nuevo->dato = e;
		nuevo->sig = nullptr;
		NodoRanking** p = &cabeza;
		while (*p != nullptr && comparacionPuntajes(e, (*p)->dato) == false) {
			p = &((*p)->sig);
		}
		nuevo->sig = *p;
		*p = nuevo;
		tam++;
	}
	// La siguiente función es para copiar las primeras entradas por medio de una variable (n) y devuelve cuántas copio.
	int copiaDatos(int n, EntradaRanking devuelve[]) const {
		int copias = 0;
		NodoRanking* Aux = cabeza;
		while (Aux != nullptr && copias < n) {
			devuelve[copias] = Aux->dato;
			copias++;
			Aux = Aux->sig;
		}
		return copias;
	}
	// Este void imprime la tabla con los primeros jugadores, indicando posición, puntaje, precisión y nombre
	void tablaRanking(int n) const {
		cout << "----- Podio " << n << " Mejores Jugadores -----" << endl;
		if (tam == 0) {
			cout << "No hay jugadores" << endl;
			return;
		}
		int posicion = 1;
		NodoRanking* Aux = cabeza;
		while (Aux != nullptr && posicion <= n) {
			cout << "Posicion: " << setw(2) << posicion
				<< " | Jugador: " << left << setw(10) << Aux->dato.nombre << right
				<< " | Puntos: " << setw(6) << Aux->dato.puntaje
				<< " | Precision: " << fixed << setprecision(1) << Aux->dato.precision << "%" << endl;
			posicion++;
			Aux = Aux->sig;
		}
		cout.unsetf(ios::fixed);
	}
	NodoRanking* primero() const { return cabeza; }
	int tamano() const { return tam; }
	bool vacio() const { return tam == 0; }
};

// La siguiente función es para recorrer el historial de partidas y por cada registro se inserta una nueva entrada de datos.
// Precisión se calcula aciertos/disparos * 100 y si no hubieron disparos se establece en 0 para no tener que hacer la división.
void historialRanking(const historialPartidas& historial, Ranking& ranking) {
	NodoRegistroPartida* Aux = historial.primero();
	while (Aux != nullptr) {
		EntradaRanking e;
		e.nombre = Aux->dato.jugador;
		e.puntaje = Aux->dato.puntaje;
		e.precision = 0.0;
		if (Aux->dato.disparos > 0) {
			e.precision = 100.0 * Aux->dato.correctos / Aux->dato.disparos;
		}
		ranking.posicionNueva(e);
		Aux = Aux->sig;
	}
}

// Resumen estadistico de un jugador calculado desde el historial.
struct ResumenJugador {
	string jugador;
	int partidas;
	int victorias;
	int mejor;
	int peor;
	double media;
	double mediana;
	double desviacion;
	double precisionPromedio;
	int histograma[10];
};

// Llena r con los datos del jugador; devuelve false si no tiene partidas.
bool resumenJugador(const historialPartidas& historial, const string& jugador, ResumenJugador& r) {
	r.jugador = jugador;
	r.partidas = 0;
	r.victorias = 0;
	r.mejor = 0;
	r.peor = 0;
	r.media = 0.0;
	r.mediana = 0.0;
	r.desviacion = 0.0;
	r.precisionPromedio = 0.0;
	for (int i = 0; i < 10; ++i) r.histograma[i] = 0;

	ListaPuntajes lista;
	double sumaPrecision = 0.0;
	int conDisparos = 0;
	for (NodoRegistroPartida* a = historial.primero(); a != nullptr; a = a->sig) {
		if (a->dato.jugador != jugador) continue;
		lista.insertarFinal(a->dato.puntaje);
		r.partidas++;
		if (a->dato.posicion == 1) r.victorias++;
		if (a->dato.disparos > 0) {
			sumaPrecision += 100.0 * a->dato.correctos / a->dato.disparos;
			conDisparos++;
		}
		if (r.partidas == 1 || a->dato.puntaje > r.mejor) r.mejor = a->dato.puntaje;
		if (r.partidas == 1 || a->dato.puntaje < r.peor) r.peor = a->dato.puntaje;
	}
	if (r.partidas == 0) return false;

	r.media = lista.media();
	r.mediana = lista.mediana();
	r.desviacion = lista.desviacion();
	if (conDisparos > 0) r.precisionPromedio = sumaPrecision / conDisparos;

	// Histograma de 10 intervalos de igual ancho entre el minimo y el maximo.
	const double ancho = (r.mejor - r.peor) / 10.0;
	for (NodoRegistroPartida* a = historial.primero(); a != nullptr; a = a->sig) {
		if (a->dato.jugador != jugador) continue;
		int bin = 0;
		if (ancho > 0.0) {
			bin = static_cast<int>((a->dato.puntaje - r.peor) / ancho);
			if (bin > 9) bin = 9;
		}
		r.histograma[bin]++;
	}
	return true;
}

// Esta función imprime la tabla con los mejores jugadores (top 10)
int ejecutarRanking(const string& ruta) {
	historialPartidas historial;
	if (historial.cargaHistorial(ruta) < 0) {
		cout << "No se pudo abrir el archivo " << ruta << endl;
		return 1;
	}
	Ranking ranking;
	historialRanking(historial, ranking);
	ranking.tablaRanking(10);
	return 0;
}

// Esta función lee historial.csv e imprime las estadísticas de cada jugador
int ejecutarEstadisticas(const string& ruta) {
	historialPartidas historial;
	if (historial.cargaHistorial(ruta) < 0) {
		cout << "No se pudo abrir el archivo " << ruta << endl;
		return 1;
	}
	if (historial.vacio()) {
		cout << "El historial no tiene partidas" << endl;
		return 0;
	}
	cout << "===== ESTADISTICAS HISTORICAS (" << ruta << ") =====" << endl;
	historial.imprimirtodasEstadisticas();
	return 0;
}