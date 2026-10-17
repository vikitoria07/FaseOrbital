// FaseOrbital.cpp 
#include <iostream>
#include <string>
#include <cmath>
#include "Generador.h"
#include "Jugador.h"
#include "Helpers.h"
#include <allegro5/allegro.h>
using namespace std;

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
	int eliminarSi(Pred debeEliminarse) {
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

struct Pulso {
	bool activo;
	double radio_actual;
};
// En este struct se establece el nodo de la cola enlazada de apariciones, cada nodo guarda una aparición (un dato) y un puntero sig al nodo que sigue en la cola.
struct NodoAparicion {
	Aparicion dato;
	NodoAparicion* sig;
};

// La siguiente clase define la cola enlazada de apariciones de la oleada (ronda del juego). Esto lo hace guardando un puntero al frente que es el primer nodo en salir y un puntero al final que es
// el último nodo en entrar y tam (tamaño) establece cuantos nodos existen. Como las apariciones entran por el final y salen por el frente, se respeta el orden de llegada y eso indica que encolar y descolar son 0(1).
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
	ColaSpawn(const ColaSpawn& otra) {
		frente = nullptr;
		final = nullptr;
		tam = 0;
		NodoAparicion* Aux = otra.frente;
		while (Aux != nullptr) {
			encolar(Aux->dato);
			Aux = Aux->sig;
		}
	}

	// Este es el operador de asignación, realiza primero una revisión la cual verifica que no se esté asignando la cola a si misma y después de esta revisión
	// libera los nodos que ya tenía esta cola y por último copia uno por uno los nodos de la otra cola.
	ColaSpawn& operator=(const ColaSpawn& otra) {
		if (this != &otra) {
			NodoAparicion* Aux = frente;
			while (Aux != nullptr) {
				NodoAparicion* borrar = Aux;
				Aux = Aux->sig;
				delete borrar;
			}
			frente = nullptr;
			final = nullptr;
			tam = 0;
			Aux = otra.frente;
			while (Aux != nullptr) {
				encolar(Aux->dato);
				Aux = Aux->sig;
			}
		}
		return *this;
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

void moverTodosLosEnemigos(ListaEnemigos& enemigos,int oleada) {
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

bool enemigoChocoNave(const Enemigo& enemigo,const Jugador& jugador) {
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

// En este struct se establece el nodo de la lista circular de jugadores, cada nodo guarda un puntero al jugador y como se explico en funciones anteriores al ser una lista doble se puede recorrer hacia adelante y atrás. 
// En esta lista ningún nodo apunta a nullptr, ya que el primer nodo apunta al último y el último al primero.
struct NodoJugador {
	Jugador* j;
	NodoJugador* ant;
	NodoJugador* sig;
};

// En este class se guarda un puntero con dirección a actual que es el jugador al que le toca jugar y tam establece cuantos jugadores hay. Como es circular no hay cabeza ni cola, sino que se avanza desde actual en un sentido (horario es siguiente y antihorario es anterior).
// La lista solo guarda punteros por lo que no crea ni borra jugadores. En la función Agregar, avanzar, eliminarActual y verActual son O(1), en cambio recorrer toda la ronda es O(n).
class RondaJugadores {
	NodoJugador* actual;
	int tam;
public:
	// Esta parte como en funciones anteriores deja la ronda vacía y el tamaño en cero ya que su finalidad es ser el constructor de la ronda.
	RondaJugadores() {
		actual = nullptr;
		tam = 0;
	}

	// Esta parte se ejecuta cuando la ronda deja de existir y libera todos los nodos para no dejar memoria basura. Como la lista es circular no hay un nullptr que marque el final,
	// por eso se recorre exactamente tam (cantidad  establecida) veces y se avanza al siguiente nodo antes de hacer el delete. Los jugadores no se borran porque no pertenecen a la ronda.
	~RondaJugadores() {
		NodoJugador* Aux = actual;
		int cantidad = tam;
		for (int i = 0; i < cantidad; i++) {
			NodoJugador* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}

	// Esta función es para eliminar la copia, ya que si se copian las rondas estas apuntarían a los mismos nodos y se liberarían dos veces.
	RondaJugadores(const RondaJugadores&) = delete;
	RondaJugadores& operator=(const RondaJugadores&) = delete;

	// Esta función es para agregar un jugador justo antes de actual, como es una lista circular es como si se agregara al final de la ronda, así los jugadores quedan en el orden
	// en que se registran. Si la ronda estaba vacía, el nodo nuevo es el actual y se apunta a si mismo en los dos sentidos.
	void agregar(Jugador* j) {
		NodoJugador* nuevo = new(NodoJugador);
		nuevo->j = j;
		if (actual == nullptr) {
			nuevo->ant = nuevo;
			nuevo->sig = nuevo;
			actual = nuevo;
		}
		else {
			nuevo->ant = actual->ant;
			nuevo->sig = actual;
			actual->ant->sig = nuevo;
			actual->ant = nuevo;
		}
		tam++;
	}

	// Esta función es para pasar el turno al siguiente jugador. Si horario es true se avanza al siguiente nodo y si es false se devuelve al anterior.
	void avanzar(bool horario) {
		if (actual == nullptr) {
			return;
		}
		if (horario == true) {
			actual = actual->sig;
		}
		else {
			actual = actual->ant;
		}
	}

	// En esta parte se elimina el jugador actual y devuelve el jugador eliminado para poder cerrar sus estadísticas. El nuevo actual es el siguiente si horario es true
	// o el anterior si es false, para seguir con el recorrido en el mismo sentido. Si era el único jugador la ronda queda vacía y actual en nullptr, y si la ronda ya estaba vacía devuelve nullptr.
	Jugador* eliminarActual(bool horario) {
		if (actual == nullptr) {
			return nullptr;
		}
		NodoJugador* borrar = actual;
		Jugador* eliminado = borrar->j;
		if (tam == 1) {
			actual = nullptr;
		}
		else {
			borrar->ant->sig = borrar->sig;
			borrar->sig->ant = borrar->ant;
			if (horario == true) {
				actual = borrar->sig;
			}
			else {
				actual = borrar->ant;
			}
		}
		delete borrar;
		tam--;
		return eliminado;
	}

	// Esta función es para ver el jugador al que le toca jugar sin cambiar el turno, si la ronda está vacía devuelve nullptr.
	Jugador* verActual() const {
		if (actual == nullptr) {
			return nullptr;
		}
		return actual->j;
	}

	// Se usan const para que no se pueda modificar la ronda y devolver el tamaño o indicar si esta vacía.
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};

// Este bool es para cumplir la condición de eliminarSi el cual recibe una bala y si devuelve true es porque el duenio es par y se debe borrar la bala. 
bool condicionpar(const Bala& b) {
	return b.duenio % 2 == 0;
}

// Este bool es una condición para la prueba del resultado "P04", donde establece la cabeza que es el 7 y dos grupos de balas consecutivas.
bool marcabalas(const Bala& b) {
	return b.duenio == 7 || b.duenio == 6 || b.duenio == 5 || b.duenio == 3 || b.duenio == 2;
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

// Esta función es para crear un tipo de bala que se distinga de las otras, estableciendole todos sus datos como posición etc.
Bala crearBala(int duenio) {
	Bala b;
	b.x = 0;
	b.y = 0;
	b.vx = 0;
	b.vy = 0;
	b.color = 0;
	b.duenio = duenio;
	return b;
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
Jugador crearJugador(string nombre, bool Bot) {
	Jugador jug;
	jug.ponerNombre(nombre);
	jug.aumentarPuntaje(100);
	jug.cambiarComboMaximo(100);
	jug.cambiarColorActual(MAGENTA);
	jug.aumentarPulso(100);
	return jug;
}

// AÑADIR BALAS A LISTA DE BALAS
void procesarTick(int tick, Jugador& jugador, ListaBalas& listaBalas) {
	if (tick % 4 == 0) {
		Bala bala = crearBalaAutomatica(
			jugador,
			jugador.obtenerIdentificador()
		);

		listaBalas.insertarInicio(bala);
	}
}

ColaSpawn generarOleada(int oleada, int semilla) {
	ColaSpawn nuevaOleada;

	printf("Iniciando creacion de oleada...\n");

	if (oleada <= 0) {
		return nuevaOleada;
	}

	GeneradorPseudoaleatorio generador(semillaOleada(oleada, semilla));


	// DEFINICION DE TIPOS POR OLEADA
	TipoEnemigo tipos[8];
	int cantidadTipos = 2;

	printf("OLEADA 1: Drones\n");
	tipos[0] = TipoEnemigo::Dron;
	tipos[1] = TipoEnemigo::Dron;

	if (oleada >= 2) {
		printf("OLEADA 2: Girador entrando\n");
		tipos[cantidadTipos++] = TipoEnemigo::Girador;
	}
	if (oleada >= 3) {
		printf("OLEADA 3: Divisor entrando\n");
		tipos[cantidadTipos++] = TipoEnemigo::Divisor;
	}
	if (oleada >= 4) {
		printf("OLEADA 4: Tanque entrando\n");
		tipos[cantidadTipos++] = TipoEnemigo::Tanque;
	}
	if (oleada >= 5) {
		printf("OLEADA 5: Camaleon y Girador entrando\n");
		tipos[cantidadTipos++] = TipoEnemigo::Camaleon;
		tipos[cantidadTipos++] = TipoEnemigo::Girador;
	}

	// APARICIONES
	int cantidadEnemigos = numEnemigos(oleada);
	int tickActual = 60;

	printf("Cantidad de enemigos en esta oleada: %i\n", cantidadEnemigos);
	printf("Tick actual: %i\n", tickActual);

	for (int i = 0; i < cantidadEnemigos; i++) {
		Aparicion aparicion;

		int numeroTipo = generador.siguiente();
		aparicion.tipo = tipos[numeroTipo % cantidadTipos];

		int numeroColor = generador.siguiente();
		aparicion.color = numeroColor % 3;

		int numeroTheta = generador.siguiente();
		aparicion.theta = numeroTheta % 360;

		aparicion.sentido = 0; // si no es girador no hay sentido


		// Si el tipo de enemigo es Girador, se genera un número aleatorio para determinar su sentido (0 o 1).
		if (aparicion.tipo == TipoEnemigo::Girador) {
			int numeroSentido = generador.siguiente();
			aparicion.sentido = numeroSentido % 2;
		}

		aparicion.tick = tickActual;
		
		// ENCOLAMOS LA APARICION
		nuevaOleada.encolar(aparicion);

		int numeroIntervalo = generador.siguiente();

		int intervaloBase = 35 - 2 * oleada;

		if (intervaloBase < 9) {
			intervaloBase = 9;
		}

		int intervaloReal = (intervaloBase * (60 + numeroIntervalo % 81))/ 100;

		// Desde la oleada 7 puede haber aparicion  doble

		if (oleada >= 7 && numeroIntervalo % 4 == 0) {
			intervaloReal = 0;
		}

		tickActual += intervaloReal;
	}

	return nuevaOleada;
}

enum class TipoAccion {
	NONE, 
	GIRAR_A,
	CAMBIAR_COLOR,
	ACTIVAR_PULSO
};

// ============================================================
// ACCIONES
// ============================================================

struct Accion {
	bool tieneGiro;
	float anguloObjetivo;

	bool tieneColor;
	Color color;

	bool activarPulso;

	Accion()
		: tieneGiro(false),
		anguloObjetivo(0.0f),
		tieneColor(false),
		color(Color::MAGENTA),
		activarPulso(false) {
	}
};


// ============================================================
// MOTOR
// ============================================================

class Motor {
private:
	int tickActual;
	int ultimoTickProcesado;
	int oleadaActual;
	int tickInicioOleada;
	int siguienteEnemigo;
	int semillaPartida;
	bool partidaTerminada;

	Jugador jugadorActual;
	ColaSpawn colaSpawn;
	ListaEnemigos enemigos;
	ListaBalas balas;
	Pulso pulso;
	Accion accionPendiente;

	// FAS-50: elige el enemigo elegible con menor radio,
	// prepara el giro y el cambio de color, y decide si activa el pulso.
	Accion decidirAccionBot() const {
		Accion accion;

		const NodoEnemigo* objetivo = nullptr;
		int enemigosCercanos = 0;

		for (const NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Enemigo& enemigo = actual->dato;

			if (enemigo.r < 160.0) {
				++enemigosCercanos;
			}

			if (enemigo.r > 100.0 &&
				(objetivo == nullptr ||
					enemigo.r < objetivo->dato.r)) {
				objetivo = actual;
			}
		}

		if (objetivo != nullptr) {
			accion.tieneGiro = true;
			accion.anguloObjetivo =
				static_cast<float>(objetivo->dato.theta);

			const Color colorObjetivo =
				static_cast<Color>(objetivo->dato.color);

			if (jugadorActual.obtenerColorActual() !=
				static_cast<int>(colorObjetivo)) {
				accion.tieneColor = true;
				accion.color = colorObjetivo;
			}
		}

		accion.activarPulso =
			jugadorActual.obtenerCargaPulse() >= 100.0f &&
			enemigosCercanos >= 5;

		return accion;
	}

	// 1. Leer acciones: aquí se guardan para aplicarlas en su etapa.
	void leerAcciones(const Accion& accion) {
		accionPendiente = accion;
	}

	// 2. Actualizar nave.
	void actualizarNave() {
		if (accionPendiente.tieneGiro) {
			jugadorActual.girarHacia(
				accionPendiente.anguloObjetivo
			);
		}

		if (accionPendiente.tieneColor) {
			jugadorActual.cambiarColorActual(
				accionPendiente.color
			);
		}
	}

	// 3. Insertar en el juego las apariciones cuyo tick ya llegó.
	void generarEnemigos() {
		const int tickRelativoOleada =
			tickActual - tickInicioOleada;

		while (!colaSpawn.vacia()) {
			const Aparicion* frente = colaSpawn.verFrente();

			if (frente == nullptr ||
				frente->tick > tickRelativoOleada) {
				break;
			}

			Aparicion aparicion;
			colaSpawn.desencolar(aparicion);

			Enemigo enemigo = crearEnemigoAparicion(
				aparicion,
				siguienteEnemigo++
			);

			enemigos.insertarFinal(enemigo);
		}
	}

	// 4. Crear una bala cada cuatro ticks.
	void disparoAutomatico() {
		if (tickActual % 4 == 0) {
			Bala bala = crearBalaAutomatica(
				jugadorActual,
				jugadorActual.obtenerIdentificador()
			);

			balas.insertarInicio(bala);
		}
	}

	// 5. Mover todos los enemigos.
	void moverEnemigos() {
		for (NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			moverEnemigo(actual->dato, oleadaActual);
		}
	}

	// 6. Resolver colisiones de enemigos con núcleo y nave.
	void resolverColisionesEnemigos() {
		NodoEnemigo* actual = enemigos.primero();

		while (actual != nullptr) {
			if (enemigoAlcanzoNucelo(
				actual->dato,
				jugadorActual
			)) {
				actual = enemigos.eliminar(actual);
				continue;
			}

			if (enemigoChocoNave(
				actual->dato,
				jugadorActual
			)) {
				jugadorActual.recibirDanno(12);
				jugadorActual.resetearCombo();

				actual = enemigos.eliminar(actual);
				continue;
			}

			actual = actual->sig;
		}
	}

	// 7. Mover balas y resolver sus impactos.
	void moverBalasYResolverImpactos() {
		for (NodoBala* balaActual = balas.primero();
			balaActual != nullptr;
			balaActual = balaActual->sig) {
			balaActual->dato.x += balaActual->dato.vx;
			balaActual->dato.y += balaActual->dato.vy;

			NodoEnemigo* enemigoActual = enemigos.primero();

			while (enemigoActual != nullptr) {
				if (balaImpactaEnemigo(
					balaActual->dato,
					enemigoActual->dato
				)) {
					resolverImpacto(
						balaActual->dato,
						enemigoActual->dato,
						jugadorActual
					);

					balaActual->dato.marcada = true;

					if (enemigoActual->dato.vida <= 0) {
						enemigos.eliminar(enemigoActual);
					}

					// Una bala solo puede impactar a un enemigo.
					break;
				}

				enemigoActual = enemigoActual->sig;
			}

			const double radioBala = std::sqrt(
				balaActual->dato.x * balaActual->dato.x +
				balaActual->dato.y * balaActual->dato.y
			);

			if (radioBala > 320.0) {
				balaActual->dato.marcada = true;
			}
		}

		balas.eliminarSi([](const Bala& bala) {
			return bala.marcada;
			});
	}

	// 8. Activar y avanzar el pulso.
	void procesarPulso() {
		if (accionPendiente.activarPulso) {
			activarPulso(pulso, jugadorActual);
		}

		avanzarPulso(pulso, enemigos, jugadorActual);
	}

	// 9. Lugar reservado para actualizar estadísticas.
	void actualizarEstadisticas() {
		// Conectar aquí los contadores y la bitácora cuando estén implementados.
	}

	// 10. Verificar fin de partida y avance de oleada.
	void verificarFin() {
		if (jugadorActual.obtenerVidaNucleo() <= 0) {
			partidaTerminada = true;
			cout << "[FIN] Partida terminada: núcleo destruido."
				<< endl;
			return;
		}

		if (!colaSpawn.vacia() || !enemigos.vacia()) {
			return;
		}

		cout << "[FIN] Oleada " << oleadaActual
			<< " terminada." << endl;

		++oleadaActual;
		tickInicioOleada = tickActual + 1;

		colaSpawn = generarOleada(
			oleadaActual,
			semillaPartida
		);
	}

public:
	Motor()
		: tickActual(0),
		ultimoTickProcesado(-1),
		oleadaActual(1),
		tickInicioOleada(0),
		siguienteEnemigo(1),
		semillaPartida(42),
		partidaTerminada(false),
		pulso{ false, 0.0 } {
		colaSpawn = generarOleada(
			oleadaActual,
			semillaPartida
		);
	}

	// Para probar el bot sin entradas humanas.
	void tickBot() {
		tick(decidirAccionBot());
	}

	// También permite suministrar una acción manual/de prueba.
	void tick(const Accion& accion) {
		if (partidaTerminada) {
			return;
		}

		ultimoTickProcesado = tickActual;

		// El orden de estas etapas es el requerido por FAS-40.
		leerAcciones(accion);               // 1
		actualizarNave();                   // 2
		generarEnemigos();                  // 3
		disparoAutomatico();                // 4
		moverEnemigos();                    // 5
		resolverColisionesEnemigos();       // 6
		moverBalasYResolverImpactos();      // 7
		procesarPulso();                    // 8
		actualizarEstadisticas();           // 9
		verificarFin();                     // 10

		++tickActual;
	}

	bool terminada() const {
		return partidaTerminada;
	}

	void imprimirEstado() const {
		cout << "Tick: " << ultimoTickProcesado
			<< " | Oleada: " << oleadaActual
			<< " | Enemigos: " << enemigos.tamano()
			<< " | Balas: " << balas.tamano()
			<< " | Vida: " << jugadorActual.obtenerVidaNucleo()
			<< " | Puntaje: " << jugadorActual.obtenerPuntaje()
			<< " | Combo: " << jugadorActual.obtenerComboActual()
			<< " | Carga: " << jugadorActual.obtenerCargaPulse()
			<< endl;
	}
};


// ============================================================
// SIMULACIÓN DE CONSOLA CON BOT1
// ============================================================

int main() {
	Motor motor;
	const int limiteTicks = 20000;

	cout << "----- SIMULACION FASE ORBITAL: BOT1 -----"
		<< endl;

	for (int tick = 0;
		tick < limiteTicks && !motor.terminada();
		++tick) {
		motor.tickBot();

		// Reduce el volumen de salida: muestra estado cada 30 ticks.
		if ((tick + 1) % 30 == 0) {
			motor.imprimirEstado();
		}
	}

	motor.imprimirEstado();

	if (motor.terminada()) {
		cout << "El bot terminó la partida." << endl;
	}
	else {
		cout << "Simulacion detenida al llegar al limite de ticks."
			<< endl;
	}

	return 0;
}