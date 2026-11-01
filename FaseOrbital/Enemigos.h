#pragma once

#include "Jugador.h"
#include "Balas.h"
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


// APARICIONES DE ENEMIGOS: En esta seccion se definen las apariciones de enemigos, que son los enemigos que todavía no han aparecido en el juego. Se define un struct para cada aparición, el cual guarda los datos del enemigo que va a aparecer y un struct para cada nodo de la lista, el cual guarda una aparición y punteros al nodo anterior y siguiente. Se define una clase para la lista, la cual tiene métodos para insertar, eliminar y recorrer la lista.

// Continuando con la misma base de las estructuras anteriores, este struct guarda los datos de cada aparición pendiente de la oleada (guarda el enemigo que todavía no ha aparecido).
// El tipo en el struct establece que clase de enemigo va a aparecer, color indica el color con el que aparece, theta es el ángulo, tick indica en cuál tick de la oleada le toca aparecer 
// y sentido indica hacia donde gira el enemigo (tomando en cuenta que 0 es horario y 1 antihorario).
struct Aparicion {
	TipoEnemigo tipo;
	int color;
	int theta;
	int tick;
	int sentido;
};

// En este struct se establece el nodo de la cola enlazada de apariciones, cada nodo guarda una aparición (un dato) y un puntero sig al nodo que sigue en la cola.
struct NodoAparicion {
	Aparicion dato;
	NodoAparicion* sig;
};

// La siguiente clase define la cola enlazada de apariciones de la oleada (ronda del juego). Esto lo hace guardando un puntero al frente que es el primer nodo en salir y un puntero al final que es
// el último nodo en entrar y tam (tamaño) establece cuantos nodos existen. Como las apariciones entran por el final y salen por el frente, se respeta el orden de llegada y eso indica que encolar y descolar son O(1).
// Por otro lado, la copia recorre todos los nodos y esto indica que es O(n).
class ColaSpawn {
	NodoAparicion* frente;
	NodoAparicion* final;
	int tam;
public:
	//  En esta parte se indica la cola que este vacía y el tamaño en cero, haciéndolo que sea el constructor de la cola.
	ColaSpawn() {
		frente = nullptr;
		final = nullptr;
		tam = 0;
	}

	// Las siguientes líneas son las constructoras de copia, se hace esta copia para que cuando se recorra la cola original y por cada nodo se cree un nuevo nodo con los mismos datos.
	ColaSpawn(const ColaSpawn& otra)
		: frente(nullptr), final(nullptr), tam(0) {
		try {
			const NodoAparicion* actual = otra.frente;

			while (actual != nullptr) {
				encolar(actual->dato);
				actual = actual->sig;
			}
		}
		catch (...) {
			limpiar();
			throw;
		}
	}

	ColaSpawn& operator=(const ColaSpawn& otra) {
		if (this != &otra) {
			ColaSpawn copia(otra);
			intercambiar(copia);
		}

		return *this;
	}

	template<class Operacion>
	void recorrer(Operacion aplicar) const {
		const NodoAparicion* actual = frente;

		while (actual != nullptr) {
			aplicar(actual->dato);
			actual = actual->sig;
		}
	}

	// Esta parte se ejecuta cuando la cola deja de existir y libera todos los nodos para no dejar memoria basura.
	// Se utiliza un sig para que al hacer delete no se pierda el resto de la cola.
	~ColaSpawn() {
		NodoAparicion* Aux = frente;
		while (Aux != nullptr) {
			NodoAparicion* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}

	// Esta función es para agregar una aparición al final de la cola. Si la cola estaba vacía, el nodo nuevo es a la vez el frente y el final,
	// si no, el antiguo final apunta al nodo nuevo y este pasa a ser el final.
	void encolar(const Aparicion& a) {
		NodoAparicion* nuevo = new(NodoAparicion);
		nuevo->dato = a;
		nuevo->sig = nullptr;
		if (final == nullptr) {
			frente = nuevo;
		}
		else {
			final->sig = nuevo;
		}
		final = nuevo;
		tam++;
	}

	// Esta función es para sacar la aparición que está en el frente de la cola. Guarda el dato en salida y devuelve true si se pudo sacar,
	// si la cola está vacía no hace nada y devuelve false. Si al sacarla la cola queda vacía se pone al final en nullptr.
	bool desencolar(Aparicion& salida) {
		if (frente == nullptr) {
			return false;
		}
		NodoAparicion* borrar = frente;
		salida = borrar->dato;
		frente = borrar->sig;
		if (frente == nullptr) {
			final = nullptr;
		}
		delete borrar;
		tam--;
		return true;
	}

	// Esta función es para ver la aparición que está en el frente sin sacarla de la cola. Devuelve un puntero const para que no se pueda modificar
	// y si la cola está vacía devuelve nullptr.
	const Aparicion* verFrente() const {
		if (frente == nullptr) {
			return nullptr;
		}
		return &(frente->dato);
	}

	// Se usan const para que no se pueda modificar la cola y devolver el tamaño o indicar si esta vacía.
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }

	void limpiar() {

		while (frente != nullptr) {
			NodoAparicion* borrar = frente;
			frente = frente->sig;

			delete borrar;
		}

		final = nullptr;
		tam = 0;
	}

	void intercambiar(ColaSpawn& otra) {
		NodoAparicion* frenteTemp = frente;
		frente = otra.frente;
		otra.frente = frenteTemp;

		NodoAparicion* finalTemp = final;
		final = otra.final;
		otra.final = finalTemp;

		int tamTemp = tam;
		tam = otra.tam;
		otra.tam = tamTemp;
	}
};

Enemigo crearEnemigoAparicion(const Aparicion& aparicion, int id) {
	Enemigo nuevoEnemigo;

	nuevoEnemigo.id = id;
	nuevoEnemigo.tipo = aparicion.tipo;
	nuevoEnemigo.color = aparicion.color;
	nuevoEnemigo.r = 300.0;
	nuevoEnemigo.theta = aparicion.theta;
	nuevoEnemigo.omega = 0.0;
	nuevoEnemigo.ticksColor = 0;
	nuevoEnemigo.sentido = aparicion.sentido;

	switch (nuevoEnemigo.tipo) {

	case TipoEnemigo::Dron:
		nuevoEnemigo.vida = 1;
		nuevoEnemigo.radio = 6;
		break;
	case TipoEnemigo::Girador:
		nuevoEnemigo.vida = 1;
		nuevoEnemigo.radio = 6;
		break;
	case TipoEnemigo::Divisor:
		nuevoEnemigo.vida = 1;
		nuevoEnemigo.radio = 8;
		break;
	case TipoEnemigo::Mini:
		nuevoEnemigo.vida = 1;
		nuevoEnemigo.radio = 4;
		break;
	case TipoEnemigo::Tanque:
		nuevoEnemigo.vida = 3;
		nuevoEnemigo.radio = 10;
		break;
	case TipoEnemigo::Camaleon:
		nuevoEnemigo.vida = 1;
		nuevoEnemigo.radio = 7;
		break;
	}
	return nuevoEnemigo;
}

Enemigo crearMini(const Enemigo& divisor, int id, double desplazamientoAngular, double velocidadAngular) {
	Enemigo mini;

	mini.id = id;
	mini.tipo = TipoEnemigo::Mini;
	mini.r = divisor.r;
	mini.theta = normalizarAngulo(
		divisor.theta + desplazamientoAngular);
	mini.omega = velocidadAngular;
	mini.vida = 1;
	mini.radio = 4;
	mini.ticksColor = 0;
	mini.sentido = 0;

	// los colores deben ser diferentes a los del padre

	if (desplazamientoAngular < 0.0) {
		mini.color = (divisor.color + 1) % 3;
	}
	else {
		mini.color = (divisor.color + 2) % 3;
	}

	return mini;

}

void reemplazarPorMinis(ListaEnemigos& enemigos, NodoEnemigo* nodo, int& siguienteId) {
	if (nodo == nullptr) {
		return;
	}

	const Enemigo divisor = nodo->dato;

	if (divisor.tipo == TipoEnemigo::Divisor) {
		NodoEnemigo* miniHorario = enemigos.insertarDespues(
			nodo,
			crearMini(divisor, siguienteId++, -4.0, -1.7)
		);

		enemigos.insertarDespues(
			miniHorario,
			crearMini(divisor, siguienteId++, 4.0, 1.7)
		);
	}

	enemigos.eliminar(nodo);
}

void moverTodosLosEnemigos(ListaEnemigos& enemigos, int oleada) {
	NodoEnemigo* actual = enemigos.primero();

	while (actual != nullptr) {
		moverEnemigo(actual->dato, oleada);
		actual = actual->sig;
	}
}


double enemigoX(const Enemigo& enemigo) {
	return enemigo.r * cos(aRadianes(enemigo.theta));
}

double enemigoY(const Enemigo& enemigo) {
	return enemigo.r * sin(aRadianes(enemigo.theta));
}

bool enemigoAlcanzoNucelo(Enemigo& enemigo, Jugador& jugador) {
	double limite = 20.0 + 0.4 * enemigo.radio;

	if (enemigo.r <= limite) {
		int danno = enemigo.tipo == TipoEnemigo::Tanque ? 22 : 10; // si es tanque el daño es 22

		jugador.recibirDanno(danno);
		jugador.resetearCombo();

		return true;
	}

	return false;
}

bool enemigoChocoNave(const Enemigo& enemigo, const Jugador& jugador) {
	double angulo = aRadianes(jugador.obtenerPosicionAngular());

	double naveX = 100.0 * cos(angulo);
	double naveY = 100.0 * sin(angulo);

	double enemigoPosX = enemigoX(enemigo);
	double enemigoPosY = enemigoY(enemigo);

	// sacamos la distancia entre la nave y el enemigo
	double distanciaColision = distancia(enemigoPosX, enemigoPosY, naveX, naveY);

	return distanciaColision < enemigo.radio + 5.0;

}

bool balaImpactaEnemigo(const Bala& bala, Enemigo& enemigo) {
	double x = enemigoX(enemigo);
	double y = enemigoY(enemigo);

	// la distancia entre la bala y la posicion del enemigo es menor al radio de enemigo
	return distancia(bala.x, bala.y, x, y) < enemigo.radio;
}

int puntosPorTipo(TipoEnemigo tipo) {
	switch (tipo) {
	case TipoEnemigo::Dron:
		return 100;
	case TipoEnemigo::Girador:
		return 150;
	case TipoEnemigo::Divisor:
		return 200;
	case TipoEnemigo::Mini:
		return 75;
	case TipoEnemigo::Tanque:
		return 400;
	case TipoEnemigo::Camaleon:
		return 300;
	}
	return 0;
}

void resolverImpacto(Bala& bala, Enemigo& enemigo, Jugador& jugador) {

	// Si hubo un tiro fallido se resetea el combo
	if (bala.color != enemigo.color) {
		cout << "[IMPACTO INCORRECTO]" << endl;
		enemigo.r += 9.0;
		jugador.resetearCombo();
		return;
	}

	// si no es fallido el enemigo pierde vida

	enemigo.vida--;

	// si el enemigo no muere, no pasa nada
	if (enemigo.vida > 0) {
		return;
	}

	// Si el enemigo si muere

	jugador.incrementarCombo();
	int multiplicador = calcMultiplicador(jugador.obtenerComboActual());
	int puntos = puntosPorTipo(enemigo.tipo) * multiplicador;
	jugador.aumentarPuntaje(puntos);

	// Aumentamos el pulso
	int carga = enemigo.tipo == TipoEnemigo::Tanque ? 14 : 5;
	jugador.aumentarPulso(carga);
}

// Se realiza esta función para crear un enemigo para probarlo y obtener los resultados esperados, se establece su tipo, color, ángulo, distancia con el núcleo etc.
Enemigo crearEnemigo(int id) {
	Enemigo e;
	e.id = id;
	e.tipo = TipoEnemigo::Dron;
	e.color = 0;
	e.r = 300;
	e.theta = 0;
	e.omega = 0;
	e.vida = 1;
	e.radio = 6;
	e.ticksColor = 0;
	return e;
}

// Esta función es para crear una aparición que se distinga de las otras, el número n se usa como tick y para calcular el ángulo, así cada aparición
// tiene datos diferentes y se puede comprobar el orden de la cola y que las copias tengan los mismos datos.
Aparicion crearAparicion(int n) { // n es el tick de la aparición y se usa para calcular el ángulo y el sentido
	Aparicion a;
	a.tipo = TipoEnemigo::Dron;
	a.color = n % 3;
	a.theta = n * 10.0;
	a.tick = n;
	a.sentido = n % 2;
	return a;
}

bool mismaAparicion(const Aparicion& a, const Aparicion& b) {
	return a.tipo == b.tipo &&
		a.color == b.color &&
		a.theta == b.theta &&
		a.tick == b.tick &&
		a.sentido == b.sentido;
}
