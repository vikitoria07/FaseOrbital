#pragma once

// BALAS: En este header se definen los tipos de balas y la lista doblemente enlazada que las contiene. Se define un struct para cada bala, el cual guarda sus datos y un struct para cada nodo de la lista, el cual guarda una bala y punteros al nodo anterior y siguiente. Se define una clase para la lista, la cual tiene métodos para insertar, eliminar y recorrer la lista.

// Como en la estructura anterior, se configura la estructura bala que guarda los datos de cada bala
// disparada por la nave (usuario). Como en enemigo, x y y son su posición y vx y vy como se mueve en su ángulo por medio de tick, el color se le asigna igualmente que a enemigo
// y duenio es el jugador.
struct Bala {
	double x, y, vx, vy;
	int color;
	int duenio;
	bool marcada;
};

// Este struct establece el nodo de la lista simple de balas.
struct NodoBala {
	Bala dato;
	NodoBala* sig;
};

// Esta clase utiliza la misma estructura como la de enemigos, pero esta solo guarda los datos de cabeza por que las balas siempre se agregan al inicio.
class ListaBalas {
	NodoBala* cabeza;
	int tam;
public:
	// Esta parte deja la lista vacía y el tamaño en cero como constructor de la lista.
	ListaBalas() {
		cabeza = nullptr;
		tam = 0;
	}

	//  En cambio, esta se puede tomar como la que la elimina ya que recorre la lista y libera todos los nodos.
	~ListaBalas() {
		NodoBala* Aux = cabeza;
		while (Aux != nullptr) {
			NodoBala* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}

	ListaBalas(const ListaBalas&) = delete;
	ListaBalas& operator=(const ListaBalas&) = delete;

	// Este void es para agregar una bala al inicio de la lista. 
	void insertarInicio(const Bala& b) {
		NodoBala* nuevo = new(NodoBala);
		nuevo->dato = b;
		nuevo->sig = cabeza;
		cabeza = nuevo;
		tam++;
	}

	// Esta función es para borrar todas las balas para las que la condición debeEliminarse se cumpla e indique cuantas se borraron.
	// Por medio de un puntero recorre la lista y apunta al puntero que enlaza el nodo actual.
	template<class Pred>
	int eliminarBalas(Pred debeEliminarse) {
		int borradas = 0;
		NodoBala** p = &cabeza;
		while (*p != nullptr) {
			if (debeEliminarse((*p)->dato) == true) {
				NodoBala* borrar = *p;
				*p = borrar->sig;
				delete borrar;
				tam--;
				borradas++;
			}
			else {
				p = &((*p)->sig);
			}
		}
		return borradas;
	}

	// Como en enemigo, se usan como consulta y se usan const para que no se pueda modificar la lista y devolver la cabeza(primero), cola(último), el tamaño o indicar si esta vacía.
	NodoBala* primero() const { return cabeza; }
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};

Bala crearBalaAutomatica(const Jugador& jugador, int identificadorJugador) {
	constexpr double PI = 3.141592;
	constexpr double RADIO_ORBITA = 100.0;
	constexpr double VELOCIDAD_BALA = 9;

	double anguloRadianes =
		jugador.obtenerPosicionAngular() * PI / 180.0;

	Bala bala;

	bala.x = RADIO_ORBITA * cos(anguloRadianes);
	bala.y = RADIO_ORBITA * sin(anguloRadianes);

	bala.vx = VELOCIDAD_BALA * cos(anguloRadianes);
	bala.vy = VELOCIDAD_BALA * sin(anguloRadianes);

	bala.color = static_cast<int>(jugador.obtenerColorActual());
	bala.duenio = identificadorJugador;
	bala.marcada = false;

	return bala;
}