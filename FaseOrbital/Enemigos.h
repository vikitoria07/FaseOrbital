#pragma once

// ENEMIGOS: En este header se definen los tipos de enemigos y la lista doblemente enlazada que los contiene. Se define un struct para cada enemigo, el cual guarda sus datos y un struct para cada nodo de la lista, el cual guarda un enemigo y punteros al nodo anterior y siguiente. Se define una clase para la lista, la cual tiene métodos para insertar, eliminar y recorrer la lista.


// En esta parte se establecen los seis tipos de enemigos del programa. Guarda los datos de cada enemigo en el cual id identifica al enemigo, tipo dice que clase es,
// el tipo de color se guarda por medio de números, r indica la distancia al núcleo, theta el ángulo, omega cuánto gira e ángulo por medio de un tick,
// vida indica la cantidad de golpes restantes para que muera, radio es el tamaño en el cual puede moverse y ticksColor para que el camaleón pueda cambiar de color.
enum class TipoEnemigo { Dron, Girador, Divisor, Mini, Tanque, Camaleon };
struct Enemigo {
	int id;
	TipoEnemigo tipo;
	int color;
	double r;
	double theta;
	double omega;
	int vida;
	int radio;
	int ticksColor;
	int sentido; // Para girador 0 = horario 1 = antihorario
};

//En el siguiente struct es para establecer la lista doble, cada nodo guarda un tipo de enemigo(dato),
// la lista se puede recorrer hacia adelante y atrás gracias a ant y sig.
struct NodoEnemigo {
	Enemigo dato;
	NodoEnemigo* ant;
	NodoEnemigo* sig;
};


// En este class se define la lista doblemente enlazada de enemigos. Guarda un puntero a la cabeza el cual es el primer nodo y uno a la
// cola que es el último nodo, tamaño establece cuantos nodos existen.
class ListaEnemigos {
	NodoEnemigo* cabeza;
	NodoEnemigo* cola;
	int tam;
public:
	ListaEnemigos() {
		cabeza = nullptr;
		cola = nullptr;
		tam = 0;
	}

	// Esta parte se ejecuta cuando la lista deja de existir y libera todos los nodos para no dejar memoria basura.
	// Se avanza al siguiente nodo antes de hacer el delete para poder volver leer la lista al volver a ejecutarse.
	~ListaEnemigos() {
		NodoEnemigo* Aux = cabeza;
		while (Aux != nullptr) {
			NodoEnemigo* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	ListaEnemigos(const ListaEnemigos&) = delete;
	ListaEnemigos& operator=(const ListaEnemigos&) = delete;

	// la siguiente función es para insertar un enemigo al final de la lista y devuelve el nodo creado.
	//Si la lista estaba vacía, el nodo nuevo es a la vez la cabeza y la cola.
	NodoEnemigo* insertarFinal(const Enemigo& e) {
		NodoEnemigo* nuevo = new NodoEnemigo;
		nuevo->dato = e;
		nuevo->sig = nullptr;
		nuevo->ant = cola;
		if (cola == nullptr) {
			cabeza = nuevo;
		}
		else {
			cola->sig = nuevo;
		}
		cola = nuevo;
		tam++;
		return nuevo;
	}

	// En la siguiente parte se agrega un enemigo justo después del nodo n y devuelve el nodo creado.
	// Si n era la cola, el nodo nuevo pasa a ser el último nodo y si n es nullptr no se inserta nada.
	NodoEnemigo* insertarDespues(NodoEnemigo* n, const Enemigo& e) {
		if (n == nullptr) {
			return nullptr;
		}
		NodoEnemigo* nuevo = new NodoEnemigo;
		nuevo->dato = e;
		nuevo->ant = n;
		nuevo->sig = n->sig;
		if (n->sig != nullptr) {
			n->sig->ant = nuevo;
		}
		else {
			cola = nuevo;
		}
		n->sig = nuevo;
		tam++;
		return nuevo;
	}

	// En esta parte se elimina el nodo n y devuelve el nodo siguiente para poder seguir el recorrido de la lista. Si n es nullptr no se realiza nada y se devuelve nullptr.
	NodoEnemigo* eliminar(NodoEnemigo* n) {
		if (n == nullptr) {
			return nullptr;
		}
		NodoEnemigo* siguiente = n->sig;
		if (n->ant != nullptr) {
			n->ant->sig = n->sig;
		}
		else {
			cabeza = n->sig;
		}
		if (n->sig != nullptr) {
			n->sig->ant = n->ant;
		}
		else {
			cola = n->ant;
		}
		delete n;
		tam--;
		return siguiente;
	}
	// Se usan const para que no se pueda modificar la lista y devolver la cabeza(primero), cola(último), el tamaño o indicar si esta vacía.
	NodoEnemigo* primero() const { return cabeza; }
	NodoEnemigo* ultimo() const { return cola; }
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};


// FUNCIONES PARA ENEMIGOS

void moverEnemigo(Enemigo& enemigo, int oleada) {
	double velocidad = velocidadBaseEnemigo(oleada);

	// DRON: Rapidez no tanta, sin sentido
	if (enemigo.tipo == TipoEnemigo::Dron) {
		enemigo.r -= velocidad;
		enemigo.omega = 0.0;
	}
	else if (enemigo.tipo == TipoEnemigo::Girador) {
		enemigo.r -= 0.85 * velocidad;

		enemigo.omega = 0.9 + 0.05 * oleada;

		// si el sentido es antihorario
		if (enemigo.sentido == 0) {
			enemigo.theta -= enemigo.omega;
		}
		else {
			enemigo.theta += enemigo.omega;
		}

		enemigo.theta = normalizarAngulo(enemigo.theta);
	}
	else if (enemigo.tipo == TipoEnemigo::Divisor) {
		enemigo.r -= 0.8 * velocidad;
		enemigo.omega = 0.0;
	}
	else if (enemigo.tipo == TipoEnemigo::Mini) {
		enemigo.r -= 1.25 * velocidad;
		enemigo.theta += enemigo.omega;
		enemigo.theta = normalizarAngulo(enemigo.theta);
	}
	else if (enemigo.tipo == TipoEnemigo::Tanque) {
		enemigo.r -= 0.5 * velocidad;
		enemigo.omega = 0.0;
	}
	else if (enemigo.tipo == TipoEnemigo::Camaleon) {
		enemigo.r -= 0.85 * velocidad;
		enemigo.omega = 0.0;

		enemigo.ticksColor++;

		if (enemigo.ticksColor >= 42) {
			enemigo.ticksColor = 0;
			enemigo.color = (enemigo.color + 1) % 3;
		}
	}

}
