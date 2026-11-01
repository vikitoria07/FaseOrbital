// FaseOrbital.cpp 

// IMPORTACIONES

#include <iostream>
#include <string>
#include <cmath>
#include <allegro5/allegro.h>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <cstdint>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstring>

// HEADERS

#include "Generador.h"
#include "Jugador.h"
#include "Helpers.h"
#include "Enemigos.h"
#include "Balas.h"

using namespace std;

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

void reemplazarPorMinis(
	ListaEnemigos& enemigos,
	NodoEnemigo* nodo,
	int& siguienteId
) {
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

// Este bool indica si la ronda es impar, si esto se cumple realiza un recorrido de tipo "horario". Por otro lado,
// si las rondas son pares realiza un recorrido "antihorario".
bool rondaimpar( int rondaactual) {
	return rondaactual % 2 == 1;
}

// La siguiente función va a ejecutar una ronda con un jugador a la vez. Como antes mencionado,
// la ronda es recorrida en sentido horario si es impar y sentido antihorario si es par.
// Por último, el jugador inicia la ronda en donde quedo actual.
template<class Operacion>
void ejecutarronda(RondaJugadores& jugadores, int rondaactual, Operacion Jugarturno) {
	bool direccionhorario = rondaimpar(rondaactual);
	int totalturnos = jugadores.tamano();
	for (int posicion = 0; posicion < totalturnos; posicion ++) {
		Jugarturno(jugadores.verActual());
		if (posicion < totalturnos - 1) {
			jugadores.avanzar(direccionhorario);
		}
	}
}


// El siguiete class es para realizar la bitácora del juego, donde se van a establecer los tipos de evento que se guardan.
enum class TipoEvento { Baja, Impacto_erroneo, Dano_nucleo, Choque_nave, Pulso, Fin_Oleada, Eliminacion };
// Este struct es para establecer los datos de cada evento.
struct Registro {
	int tick;
	int oleada;
	int jugador;
	TipoEvento tipo_evento;
	int tipo_enemigo;
	int color;
	int valor;
};
// En este struct se establece el nodo de la bitácora, en el cual guarda un dato (registro) y el puntero continua con el siguiente.
struct RegistroNodo {
	Registro dato;
	RegistroNodo* sig;
};
// Se realiza una lista simple la cual guarde el final, recorrer y filtrar son O(n) y agregar eventos O(1).
class BitacoraEventos {
	RegistroNodo* inicio;
	RegistroNodo* fin;
	int posicion;
	int total;
public:
	BitacoraEventos()
		: inicio(nullptr), fin(nullptr), posicion(0), total(0) {
	}
	~BitacoraEventos() {
		RegistroNodo* Aux = inicio;
		while (Aux != nullptr) {
			RegistroNodo* eliminar = Aux;
			Aux = Aux->sig;
			delete eliminar;
		}
	}
	BitacoraEventos(const BitacoraEventos&) = delete;
	BitacoraEventos& operator=(const BitacoraEventos&) = delete;
	// El siguiente void tiene como finalidad agregar un dato al final por medio del puntero fin
	void agregarfinal(const Registro& f) {
		RegistroNodo* nuevo = new(RegistroNodo);
		nuevo->dato = f;
		nuevo->sig = nullptr;
		if (fin == nullptr) {
			inicio = nuevo;
		}
		else {
			fin->sig = nuevo;
		}
		fin = nuevo;
		total++;
	}
	// Esta operación va a recorrer todos los eventos y desde el más viejo hasta el más nuevo.
	template<class Operacion>
	void recorrido(Operacion aplicar) const {
		RegistroNodo* Aux = inicio;
		while (Aux != nullptr) {
			aplicar(Aux->dato);
			Aux = Aux->sig;
		}
	}
	// En esta operación realiza el mismo recorrido pero solo a los eventos seleccioandos( jugador, oleada etc).
	template<class Operacion>
	int seleccionados(int idjugador, int numoleada, Operacion aplicar) const {
		int obtenidos = 0;
		RegistroNodo* Aux = inicio;
		while (Aux != nullptr) {
			if (Aux->dato.jugador == idjugador && Aux->dato.oleada == numoleada) {
				aplicar(Aux->dato);
				obtenidos++;
			}
			Aux = Aux->sig;
		}
		return obtenidos;
	}

int tamano() const { return total;
}
bool vacio() const { return total == 0;
}
};
// El siguiente struct es para crear la lista simple sobre los puntajes
struct NodoPuntaje {
	int dato;
	NodoPuntaje* sig;
};

// Esta función de nodos va a dividir la lista usando dos punteros, los cuales un nodo avanza uno por uno y el otro nodo de dos en dos. 
// Cuando el nodo "segundo" llega al final, se corta el recorrido en la mitad que es donde se encuentra el primero
NodoPuntaje* listaDividida(NodoPuntaje* cabeza) {
	NodoPuntaje* primero = cabeza;
	NodoPuntaje* segundo = cabeza->sig;
	while (segundo != nullptr && segundo->sig != nullptr) {
		primero = primero->sig;
		segundo = segundo->sig->sig;
	}
	NodoPuntaje* segunda = primero->sig;
	primero->sig = nullptr;
	return segunda;
}
// Esta parte va a mezclar dos listas convirtiendolas en una sola.
NodoPuntaje* mezclarListas(NodoPuntaje* l, NodoPuntaje* u) {
	NodoPuntaje* resultado = nullptr;
	NodoPuntaje** p = &resultado;
	while (l != nullptr && u != nullptr) {
		if (l->dato <= u->dato) {
			*p = l;
			l = l->sig;
		}
		else {
			*p = u;
			u = u->sig;
		}
		p = &((*p)->sig);
	}
	if (l != nullptr) {
		*p = l;
	}
	else {
		*p = u;
	}
	return resultado;
}

//La siguiente función es sobre el merge sort de la lista enlazada, la cual si esta vacía ya se encuentra ordenada,
// si no se divide en dos mitades y se ordenan con la misma función utilizada anteriormente.
NodoPuntaje* mergeSort(NodoPuntaje* cabeza) {
	if (cabeza == nullptr || cabeza->sig == nullptr) {
		return cabeza;
	}
	NodoPuntaje* segunda = listaDividida(cabeza);
	NodoPuntaje* izquierda = mergeSort(cabeza);
	NodoPuntaje* derecha = mergeSort(segunda);
	return mezclarListas(izquierda, derecha);
}
// La siguiente clase es ora estableceer los puntajes de un jugador por medio de una lista enlazada.
class ListaPuntajes {
	NodoPuntaje* cabeza;
	NodoPuntaje* cola;
	int tam;
public:
	ListaPuntajes() {
		cabeza = nullptr;
		cola = nullptr;
		tam = 0;
	}
	~ListaPuntajes() {
		NodoPuntaje* Aux = cabeza;
		while (Aux != nullptr) {
			NodoPuntaje* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	ListaPuntajes(const ListaPuntajes&) = delete;
	ListaPuntajes& operator=(const ListaPuntajes&) = delete;

	// El siguiente void es para mostrar un puntaje después de haber terminado de recorrer la lista
	void insertarFinal(int puntaje) {
		NodoPuntaje* nuevo = new(NodoPuntaje);
		nuevo->dato = puntaje;
		nuevo->sig = nullptr;
		if (cola == nullptr) {
			cabeza = nuevo;
		}
		else {
			cola->sig = nuevo;
		}
		cola = nuevo;
		tam++;
	}
	// Esta parte es la lista auxiliar utilizada para la mediana, la cual copia todos los puntajes de otra lista
	void copiaPuntajes(const ListaPuntajes& nueva) {
		NodoPuntaje* Aux = nueva.cabeza;
		while (Aux != nullptr) {
			insertarFinal(Aux->dato);
			Aux = Aux->sig;
		}
	}
	// La siguiente función es para ordenar la lista de menor a mayor por medio del merge sort
	void ordenLista() {
		cabeza = mergeSort(cabeza);
		cola = cabeza;
		while (cola != nullptr && cola->sig != nullptr) {
			cola = cola->sig;
		}
	}
	//Esta revisa si se ordene de manera correcta la lista
	bool listaOrdenada() const {
		NodoPuntaje* Aux = cabeza;
		while (Aux != nullptr && Aux->sig != nullptr) {
			if (Aux->dato > Aux->sig->dato) {
				return false;
			}
			Aux = Aux->sig;
		}
		return true;
	}
	// Se utiliza la función de media ya que por medio de la suma de todos los puntajes dividida entre la cantidad de jugadores conocer el puntaje que se distribuye entre los jugadores.
	double media() const {
		if (tam == 0) {
			return 0.0;
		}
		double suma = 0.0;
		NodoPuntaje* Aux = cabeza;
		while (Aux != nullptr) {
			suma += Aux->dato;
			Aux = Aux->sig;
		}
		return suma / tam;
	}

// La mediana va a copiar los valores a una lista auxiliar, la va a ordenar con el merge sort y conocer los valores promedios si estan ubicados en el centro.
double mediana() const {
	if (tam == 0) {
		return 0.0;
	}
	ListaPuntajes auxiliar;
	auxiliar.copiaPuntajes(*this);
	auxiliar.ordenLista();
	NodoPuntaje* Aux = auxiliar.cabeza;
	int mitad = tam / 2;
	for (int i = 0; i < mitad - 1 + (tam % 2); i++) {
		Aux = Aux->sig;
	}
//si la cantidad es impar aux queda en el valor central y si es par queda en el central de la izquierda
	if (tam % 2 == 1) {
		return Aux->dato;
	}
	return (Aux->dato + Aux->sig->dato) / 2.0;
}
// La desviación se usa para medir que tanta distancia tienen los puntajes de la media
double desviacion() const {
	if (tam == 0) {
		return 0.0;
	}
	double mu = media();
	double suma = 0.0;
	NodoPuntaje* Aux = cabeza;
	while (Aux != nullptr) {
		double diferencia = Aux->dato - mu;
		suma += diferencia * diferencia;
		Aux = Aux->sig;
	}
	return sqrt(suma / tam);
}
//La siguiente función es para imprimir un histograma ASCII el cual contiene 10 intervalos y cuenta en cual intervalo queda cada puntaje
void imprimirHistograma() const {
	if (tam == 0) {
		cout << "Sin puntajes para el histograma." << endl;
		return;
	}
	int minimo = cabeza->dato;
	int maximo = cabeza->dato;
	for (NodoPuntaje* Aux = cabeza; Aux != nullptr; Aux = Aux->sig) {
		if (Aux->dato < minimo) {
			minimo = Aux->dato;
		}
		if (Aux->dato > maximo) {
			maximo = Aux->dato;
		}
	}
	int cantidad[10];
	for (int i = 0; i < 10; i++) {
		cantidad[i] = 0;
	}
	double largo = (maximo - minimo) / 10.0;
	for (NodoPuntaje* Aux = cabeza; Aux != nullptr; Aux = Aux->sig) {
		int puntaje = 0;
		if (largo > 0.0) {
			puntaje = static_cast<int>((Aux->dato - minimo) / largo);
		}
		if (puntaje > 9) {
			puntaje = 9;
		}
		cantidad[puntaje]++;
	}
	for (int i = 0; i < 10; i++) {
		double comienzo = minimo + i * largo;
		double final = minimo + (i + 1) * largo;
		if (i == 9) {
			cout << "[" << comienzo << " - " << final << "] | ";
		}
		else {
			cout << "[" << comienzo << " - " << final << ") | ";
		}
		for (int j = 0; j < cantidad[i]; j++) {
			cout << "#";
		}
		cout << " (" << cantidad[i] << ")" << endl;
	}
}
NodoPuntaje* primero() const { return cabeza; }
int tamano() const { return tam; }
bool vacia() const { return tam == 0; }
};
//El siguiente struct es para guardar los datos de una partida de un jugador
struct RegistroPartida {
	int id_partida;
	int semilla;
	string modo;
	string jugador;
	int es_bot;
	int posicion;
	int puntaje;
	int oleada;
	int disparos;
	int correctos;
	int erroneos;
	int combo_max;
	int pulsos;
	int dano_total;
	int ticks;
	int bajas_dron;
	int bajas_girador;
	int bajas_divisor;
	int bajas_mini;
	int bajas_tanque;
	int bajas_camaleon;
};
struct NodoRegistroPartida {
	RegistroPartida dato;
	NodoRegistroPartida* sig;
};
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
bool lineaenRegistro(string linea, RegistroPartida& r) {
	string campos[21];

	if (separacionCampos(linea, campos, 21) != 21) {
		return false;
	}

	if (campos[3].empty()) {
		return false;
	}

	for (int i = 0; i < 21; ++i) {
		if (i != 2 && i != 3 && !valorenteroValido(campos[i])) {
			return false;
		}
	}

	r.id_partida = valorEntero(campos[0]);
	r.semilla = valorEntero(campos[1]);
	r.modo = campos[2];
	r.jugador = campos[3];
	r.es_bot = valorEntero(campos[4]);
	r.posicion = valorEntero(campos[5]);
	r.puntaje = valorEntero(campos[6]);
	r.oleada = valorEntero(campos[7]);
	r.disparos = valorEntero(campos[8]);
	r.correctos = valorEntero(campos[9]);
	r.erroneos = valorEntero(campos[10]);
	r.combo_max = valorEntero(campos[11]);
	r.pulsos = valorEntero(campos[12]);
	r.dano_total = valorEntero(campos[13]);
	r.ticks = valorEntero(campos[14]);
	r.bajas_dron = valorEntero(campos[15]);
	r.bajas_girador = valorEntero(campos[16]);
	r.bajas_divisor = valorEntero(campos[17]);
	r.bajas_mini = valorEntero(campos[18]);
	r.bajas_tanque = valorEntero(campos[19]);
	r.bajas_camaleon = valorEntero(campos[20]);

	return true;
}// La siguiente class es para establecer el historial de partidas utilizando una lista simple
class historialPartidas {
	NodoRegistroPartida* cabeza;
	NodoRegistroPartida* cola;
	int tam;
public:
	historialPartidas() {
		cabeza = nullptr;
		cola = nullptr;
		tam = 0;
	}
	~historialPartidas() {
		NodoRegistroPartida* Aux = cabeza;
		while (Aux != nullptr) {
			NodoRegistroPartida* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	historialPartidas(const historialPartidas&) = delete;
	historialPartidas& operator=(const historialPartidas&) = delete;
	// Al igual que el pasado, esta función agrega un registro al final del historial
	void registroFinal(const RegistroPartida& f) {
		NodoRegistroPartida* nuevo = new(NodoRegistroPartida);
		nuevo->dato = f;
		nuevo->sig = nullptr;
		if (cola == nullptr) {
			cabeza = nuevo;
		}
		else {
			cola->sig = nuevo;
		}
		cola = nuevo;
		tam++;
	}

	// La siguiente función es para cargar el historial por medio del archivo.csv
	int cargaHistorial(string cargar) {
		ifstream archivo(cargar.c_str());
		if (!archivo.is_open()) {
			return -1;
		}
		string linea;
		int numlin = 0;
		int cargados = 0;
		while (getline(archivo, linea)) {
			numlin++;
			if (numlin == 1 || linea.size() == 0 || linea == "\f") {
				continue;
			}
			RegistroPartida f;
			if (lineaenRegistro(linea, f) == true) {
				registroFinal(f);
				cargados++;
			}
			else {
				cout << "Línea CSV " << numlin << " mal generada" << endl;
			}
		}
		archivo.close();
		return cargados;
	}
	// Esta función guarda todo el historial en el archivo csv
	bool guardarArchivo(string cargar) const {
		ofstream archivo(cargar.c_str());
		if (!archivo.is_open()) {
			return false;
		}
		archivo << "id_partida,semilla,modo,jugador,es_bot,posicion,puntaje,oleada,"
			"disparos,correctos,erroneos,combo_max,pulsos,dano_total,ticks,"
			"bajas_dron,bajas_girador,bajas_divisor,bajas_mini,bajas_tanque,"
			"bajas_camaleon\n";

		for (NodoRegistroPartida* actual = cabeza;
			actual != nullptr;
			actual = actual->sig) {
			const RegistroPartida& f = actual->dato;

			archivo << f.id_partida << "," << f.semilla << "," << f.modo << ","
				<< f.jugador << "," << f.es_bot << "," << f.posicion << ","
				<< f.puntaje << "," << f.oleada << "," << f.disparos << ","
				<< f.correctos << "," << f.erroneos << "," << f.combo_max << ","
				<< f.pulsos << "," << f.dano_total << "," << f.ticks << ","
				<< f.bajas_dron << "," << f.bajas_girador << ","
				<< f.bajas_divisor << "," << f.bajas_mini << ","
				<< f.bajas_tanque << "," << f.bajas_camaleon << "\n";
		}
		archivo.close();
		return true;
	}
	// La siguiente función es para tener una lista auxiliar sobre los puntajes de las partidas
	void copiaPuntajes(string jugador, ListaPuntajes& cargar) const {
		NodoRegistroPartida* Aux = cabeza;
		while (Aux != nullptr) {
			if (Aux->dato.jugador == jugador) {
				cargar.insertarFinal(Aux->dato.puntaje);
			}
			Aux = Aux->sig;
		}
	}
			void imprimirEstadisticas(string jugador) const {
				ListaPuntajes lista;
				copiaPuntajes(jugador, lista);
				cout << "----- Tabla de Estadisticas " << jugador << " -----" << endl;
				cout << "Partidas: " << lista.tamano() << endl;
				cout << "Media: " << lista.media() << endl;
				cout << "Mediana: " << lista.mediana() << endl;
				cout << "Desviacion estandar: " << lista.desviacion() << endl;
				lista.imprimirHistograma();
			}
			void imprimirtodasEstadisticas() const {
				NodoRegistroPartida* Aux = cabeza;
				while (Aux != nullptr) {
					bool repetido = false;
					NodoRegistroPartida* Previo = cabeza;
					while (Previo != Aux) {
						if (Previo->dato.jugador == Aux->dato.jugador) {
							repetido = true;
						}
						Previo = Previo->sig;
					}
					if (repetido == false) {
						imprimirEstadisticas(Aux->dato.jugador);
					}
					Aux = Aux->sig;
				}
			}
			NodoRegistroPartida* primero() const { return cabeza; }
			int tamano() const { return tam; }
			bool vacio() const { return tam == 0; }
		};

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
		}
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


// Este bool es para cumplir la condición de eliminarBalas el cual recibe una bala y si devuelve true es porque el duenio es par y se debe borrar la bala. 
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

bool mismaAparicion(const Aparicion& a, const Aparicion& b) {
	return a.tipo == b.tipo &&
		a.color == b.color &&
		a.theta == b.theta &&
		a.tick == b.tick &&
		a.sentido == b.sentido;
}

void pruebaCopiaProfundaOleada() {
	const ColaSpawn oleadaBase = generarOleada(1, 42);
	ColaSpawn colaAna(oleadaBase);
	ColaSpawn colaLuis(oleadaBase);

	const int cantidadInicial = oleadaBase.tamano();

	Aparicion consumida;
	int consumidasPorAna = 0;

	while (colaAna.desencolar(consumida)) {
		++consumidasPorAna;
	}

	const Aparicion* frenteBase = oleadaBase.verFrente();
	const Aparicion* frenteLuis = colaLuis.verFrente();

	bool copiasIntactas =
		consumidasPorAna == cantidadInicial &&
		colaAna.vacia() &&
		oleadaBase.tamano() == cantidadInicial &&
		colaLuis.tamano() == cantidadInicial &&
		frenteBase != nullptr &&
		frenteLuis != nullptr &&
		mismaAparicion(*frenteBase, *frenteLuis);

	ColaSpawn colaAsignada;
	colaAsignada = oleadaBase;

	while (colaAsignada.desencolar(consumida)) {
	}

	copiasIntactas =
		copiasIntactas &&
		colaAsignada.vacia() &&
		oleadaBase.tamano() == cantidadInicial &&
		colaLuis.tamano() == cantidadInicial;

	reportar("FAS-71", "copias independientes de ColaSpawn", copiasIntactas);
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
	uint32_t semillaPartida;
	bool partidaTerminada;
	int cantidadDisparos;
	int disparosAcertados;

	Jugador jugadorActual;
	ColaSpawn colaSpawn;
	ListaEnemigos enemigos;
	ListaBalas balas;
	Pulso pulso;
	Accion accionPendiente;
	RondaJugadores ronda;

	BitacoraEventos bitacora;
	int indiceJugador = 0;
	int erroneos = 0;
	int pulsosUsados = 0;
	int danoTotal = 0;
	int bajas[6] = { 0, 0, 0, 0, 0, 0 };
	// Este void es para guardar los datos y eventos de la partida en la bitácora.
	void registrarEvento(TipoEvento tipo, int tipoEnemigo, int color, int valor) {
		Registro r;
		r.tick = tickActual;
		r.oleada = oleadaActual;
		r.jugador = indiceJugador;
		r.tipo_evento = tipo;
		r.tipo_enemigo = tipoEnemigo;
		r.color = color;
		r.valor = valor;
		bitacora.agregarfinal(r);
	}
	// Esta función cuenta la cantidad de enemigos de los distintos tipos que hay en la lista.
	// Se usa antes y después de pulso para saber cuantos enemigos se abatieron.
	void contarPorTipo(int cantidades[6]) const {
		for (int i = 0; i < 6; i++) {
			cantidades[i] = 0;
		}
		for (const NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			cantidades[static_cast<int>(actual->dato.tipo)]++;
		}
	}


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
			++cantidadDisparos;
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

	// 6. Resolver colisiones de enemigos con núcleo y nave. Además, se registra cada choque y daño en la bitácora.
	void resolverColisionesEnemigos() {
		NodoEnemigo* actual = enemigos.primero();

		while (actual != nullptr) {
			const int primeraVida = static_cast<int>(jugadorActual.obtenerVidaNucleo());
			if (enemigoAlcanzoNucelo(
				actual->dato,
				jugadorActual
			)) {

				const int dano = primeraVida - static_cast<int>(jugadorActual.obtenerVidaNucleo());
				danoTotal += dano;
				registrarEvento(TipoEvento::Dano_nucleo, static_cast<int>(actual->dato.tipo), actual->dato.color, dano);
				actual = enemigos.eliminar(actual);
				continue;
			}

			if (enemigoChocoNave(
				actual->dato,
				jugadorActual
			)) {
				jugadorActual.recibirDanno(12);
				jugadorActual.resetearCombo();

				const int dano = primeraVida - static_cast<int>(jugadorActual.obtenerVidaNucleo());
				danoTotal += dano;
				registrarEvento(TipoEvento::Choque_nave, static_cast<int>(actual->dato.tipo), actual->dato.color, dano);

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
					const int tipoEnemigo = static_cast<int>(enemigoActual->dato.tipo);
					const int colorEnemigo = enemigoActual->dato.color;
					const float puntajeAntes = jugadorActual.obtenerPuntaje();

					if (balaActual->dato.color == enemigoActual->dato.color){
						++disparosAcertados;
					}
					else {
						++erroneos;
						registrarEvento(TipoEvento::Impacto_erroneo, tipoEnemigo, colorEnemigo, balaActual->dato.color);
					}
					resolverImpacto(
						balaActual->dato,
						enemigoActual->dato,
						jugadorActual
					);

					balaActual->dato.marcada = true;

					if (enemigoActual->dato.vida <= 0) {
						++bajas[tipoEnemigo];
						registrarEvento(TipoEvento::Baja, tipoEnemigo, colorEnemigo,
							static_cast<int>(jugadorActual.obtenerPuntaje() - puntajeAntes));

						reemplazarPorMinis(enemigos, enemigoActual, siguienteEnemigo);
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

		balas.eliminarBalas([](const Bala& bala) {
			return bala.marcada;
			});
	}

	// 8. Activar y avanzar el pulso.
	void procesarPulso() {
		const bool estabaActivo = pulso.activo;
		int antes[6];
		contarPorTipo(antes);

		if (accionPendiente.activarPulso) {
			activarPulso(pulso, jugadorActual);
		}

		if (!estabaActivo && pulso.activo) {
			++pulsosUsados;
			registrarEvento(TipoEvento::Pulso, -1, -1, 100);
		}

		avanzarPulso(pulso, enemigos, jugadorActual);

		int despues[6];
		contarPorTipo(despues);

		for (int t = 0; t < 6; t++) {
			const int eliminados = antes[t] - despues[t];
			for (int k = 0; k < eliminados; k++) {
				++bajas[t];
				registrarEvento(TipoEvento::Baja, t, -1, puntosPorTipo(static_cast<TipoEnemigo>(t)));
			}
		}
	}

	// 9. Lugar reservado para actualizar estadísticas.
	void actualizarEstadisticas() {
		// Conectar aquí los contadores y la bitácora cuando estén implementados.
	}

	//10.Se ejecuta cuando vida núcleo <= 0, por lo que saca al jugador de la lista circular.
	void eliminarJugador () {
		Jugador* jugadorRemovido = ronda.verActual ();
		if (jugadorRemovido == nullptr ) {
			partidaTerminada = true;
			return;
		}
		double acierto = 0.0;
		if (cantidadDisparos > 0) {
			acierto = 100.0 * disparosAcertados / cantidadDisparos;
		}
		cout << " Partida Terminada" << endl;
		cout << "Puntos obtenidos: " << jugadorRemovido->obtenerPuntaje() << endl;
		cout << "Disparos: " << cantidadDisparos << endl;
		cout << "Disparos acertados: " << disparosAcertados << endl;
			cout << "Porcentaje de aciertos de disparos: " << acierto << "%" << endl;
		ronda.eliminarActual(true);
		if (ronda.vacia()) {
			partidaTerminada = true;
		}
		else if (ronda.tamano() == 1){
			cout << "Felicidades, ganador por ser último juagdor con vida" << endl;
			partidaTerminada = true;
		}
	}
			

	// 11. Verificar fin de partida y avance de oleada.
	void verificarFin() {
		if (jugadorActual.obtenerVidaNucleo() <= 0) {
			registrarEvento(TipoEvento::Eliminacion, -1, -1, static_cast<int>(jugadorActual.obtenerPuntaje()));
			eliminarJugador();
			return;
		}

		if (!colaSpawn.vacia() || !enemigos.vacia()) {
			return;
		}

		cout << "[FIN] Oleada " << oleadaActual
			<< " terminada." << endl;

		registrarEvento(TipoEvento::Fin_Oleada, -1, -1, static_cast<int>(jugadorActual.obtenerPuntaje()));

		jugadorActual.repararNucleo(8);
		++oleadaActual;
		tickInicioOleada = tickActual + 1;

		colaSpawn = generarOleada(
			oleadaActual,
			semillaPartida
		);
	}


public:
	Motor(uint32_t semilla = 42)
		: tickActual(0),
		ultimoTickProcesado(-1),
		oleadaActual(1),
		tickInicioOleada(0),
		siguienteEnemigo(1),
		semillaPartida(semilla),
		partidaTerminada(false),
		cantidadDisparos(0),
		disparosAcertados(0),
		pulso{ false, 0.0 } {
		ronda.agregar(&jugadorActual);
		colaSpawn = generarOleada(
			oleadaActual,
			semillaPartida
		);
	}

	const Jugador& obtenerJugador() const { return jugadorActual; }
	const BitacoraEventos& obtenerBitacora() const { return bitacora; }
	int obtenerOleada() const { return oleadaActual; }
	int obtenerTickActual() const { return tickActual; }
	int obtenerTickRelativo() const { return tickActual - tickInicioOleada; }
	int obtenerDisparos() const { return cantidadDisparos; }
	int obtenerCorrectos() const { return disparosAcertados; }
	int obtenerErroneos() const { return erroneos; }
	int obtenerPulsos() const { return pulsosUsados; }
	int obtenerDanoTotal() const { return danoTotal; }
	int obtenerBajas(int tipo) const { return bajas[tipo]; }

	// La siguiente función es para que los lotes puedan diseñar al jugador
	void configurarJugador(int indice, const string& nombre) {
		indiceJugador = indice;
		jugadorActual.ponerNombre(nombre);
	}

	Jugador* punteroJugador() {
		return &jugadorActual;
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

	const ListaEnemigos& obtenerEnemigos() const { return enemigos; }
	const ListaBalas& obtenerBalas() const { return balas; }
	const Pulso& obtenerPulso() const { return pulso; }

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
	string firmaEstado() const {
		ostringstream salida;
		salida << setprecision(17)
			<< tickActual << "|" << ultimoTickProcesado << "|"
			<< oleadaActual << "|" << tickInicioOleada << "|"
			<< siguienteEnemigo << "|" << partidaTerminada << "|"
			<< cantidadDisparos << "|" << disparosAcertados << "|"
			<< jugadorActual.obtenerPosicionAngular() << "|"
			<< static_cast<int>(jugadorActual.obtenerColorActual()) << "|"
			<< jugadorActual.obtenerCargaPulse() << "|"
			<< jugadorActual.obtenerPuntaje() << "|"
			<< jugadorActual.obtenerComboActual() << "|"
			<< jugadorActual.obtenerComboMaximo() << "|"
			<< jugadorActual.obtenerVidaNucleo() << "|"
			<< pulso.activo << "|" << pulso.radio_actual << "|";

		for (NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Enemigo& e = actual->dato;
			salida << e.id << "," << static_cast<int>(e.tipo) << ","
				<< e.color << "," << e.r << "," << e.theta << ","
				<< e.omega << "," << e.vida << "," << e.radio << ","
				<< e.ticksColor << "," << e.sentido << ";";
		}

		salida << "|";

		for (NodoBala* actual = balas.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Bala& b = actual->dato;
			salida << b.x << "," << b.y << "," << b.vx << "," << b.vy
				<< "," << b.color << "," << b.duenio << ","
				<< b.marcada << ";";
		}

		salida << "|";

		colaSpawn.recorrer([&salida](const Aparicion& a) {
			salida << a.tick << "," << static_cast<int>(a.tipo) << ","
				<< a.color << "," << a.theta << "," << a.sentido << ";";
			});

		return salida.str();
	}

};

// El siguiente bool se utiliza para comprobar que la lista siga su recorrido hacia adelante y hacia atrás.
bool verificarRecorrido (RondaJugadores& r, Jugador* requeridos [], int j) {
	if (r.tamano() != j) return false;
	for (int i = 0; i < j; i++) {
		if (r.verActual() != requeridos[i]) return false;
		r.avanzar(true);
	}
	if (r.verActual() != requeridos[0]) return false;
		for (int i = 0; i < j; i++) {
			if (r.verActual() != requeridos[(j - i) % j]) return false;
			r.avanzar(false);
		}
	return r.verActual() == requeridos[0];
}
void reportarPrueba(const char* nombre, bool resultado) {
	if (resultado == true) {
		cout << "´Ronda de prueba" << nombre << ": Correcta" << endl;
	}
	else {
		cout << "Ronda de prueba" << nombre << ": Incorrecta" << endl;
	}

}
void pruebasRondaJugadores() {
	Jugador j[4];

	{   // Esta primera parte tiene como función eliminar el primer jugador, al ser una prueba de 4 jugadores solo van a ser eliminados 3 y dejarse 1.
		RondaJugadores r;
		for (int i = 0; i < 4; i++) r.agregar(&j[i]);
		Jugador* eliminado = r.eliminarActual(true);
		Jugador* requeridos[] = { &j[1], &j[2], &j[3] };
		reportarPrueba("Eliminación primer jugador", eliminado == &j[0] && verificarRecorrido(r, requeridos, 3));
	}
	{   // En este se elimina el segundo jugador
		RondaJugadores r;
		for (int i = 0; i < 4; i++) r.agregar(&j[i]);
		r.avanzar(true); r.avanzar(true);    
		Jugador* eliminado = r.eliminarActual(true);
		Jugador* requeridos[] = { &j[3], &j[0], &j[1] };
		reportarPrueba("Eliminación segundo jugador", eliminado == &j[2] && verificarRecorrido(r, requeridos, 3));
	}
	{   // Esta parte elimina al tercer jugador el cual es el último removido de la lista
		RondaJugadores r;
		for (int i = 0; i < 4; i++) r.agregar(&j[i]);
		r.avanzar(false);                    
		Jugador* eliminado = r.eliminarActual(true);
		Jugador* requeridos[] = { &j[0], &j[1], &j[2] };
		reportarPrueba("Eliminación tercer juagdor", eliminado == &j[3] && verificarRecorrido(r, requeridos, 3));
	}
	{   // En esta parte se establece el último jugador de la lista como ganador
		RondaJugadores r;
		r.agregar(&j[0]);
		r.agregar(&j[1]);
		r.eliminarActual(true);              
		Jugador* ultimo[] = { &j[1] };
		bool ganador = r.tamano() == 1 && verificarRecorrido(r, ultimo, 1);

		Jugador* eliminado = r.eliminarActual(true);
		bool vacia = r.vacia() && r.verActual() == nullptr && r.eliminarActual(true) == nullptr;

		reportarPrueba("Ganador! Último jugador en pie", ganador);
		reportarPrueba("Liberación de lista(vacía)", eliminado == &j[1] && vacia);
	}
}
//La siguiente función es para mostrar la prueba de que Ranking si funciona en el programa, se piensa comprobar los tres criterios los cuales son que gana el mayor puntaje
// si hay un empate en puntaje gana la mejor precisión y si también empatan en precisión se ordenan los nombres en orden alfabético. Además, muestra solo 10 jugadores si es que se encuentran más.
void pruebasRanking() {
	Ranking r;
	r.posicionNueva({ "Adriel", 800, 95.0 });
	r.posicionNueva({ "María", 700, 90.0 });
	r.posicionNueva({ "Sebastián", 300, 50.0 });
	r.posicionNueva({ "Victor", 500, 70.0 });
	r.posicionNueva({ "Julian", 500, 70.0 });

	string esperado[] = { "Adriel", "María", "Julian", "Victor", "Sebastián" };
	EntradaRanking salida[10];
	int cantidad = r.copiaDatos(10, salida);
	bool correcto = (cantidad == 5);
	for (int i = 0; i < cantidad && correcto; i++) {
		if (salida[i].nombre != esperado[i]) {
			correcto = false;
		}
	}
	reportarPrueba("Ranking de la partida", correcto);

	Ranking grande;
	for (int i = 0; i < 15; i++) {
		grande.posicionNueva({ "J" + to_string(i), i * 10, 50.0 });
	}
	reportarPrueba("Ranking top 10", grande.copiaDatos(10, salida) == 10 && salida[0].puntaje == 140);
}

// ============================================================
// SIMULACIÓN DE CONSOLA CON BOT1
// ============================================================
void pruebaReproducibilidad() {
	Motor primero(42);
	Motor segundo(42);
	bool iguales = true;

	for (int tick = 0; tick < 20000 && !primero.terminada(); ++tick) {
		Accion accion;

		if (tick % 53 == 0) {
			accion.tieneGiro = true;
			accion.anguloObjetivo = static_cast<float>((tick * 37) % 360);
		}

		if (tick % 79 == 0) {
			accion.tieneColor = true;
			accion.color = static_cast<Color>((tick / 79) % 3);
		}

		accion.activarPulso = tick % 211 == 0;

		primero.tick(accion);
		segundo.tick(accion);

		if (primero.firmaEstado() != segundo.firmaEstado()) {
			iguales = false;
			break;
		}
	}

	iguales = iguales &&
		primero.firmaEstado() == segundo.firmaEstado() &&
		primero.terminada() == segundo.terminada();

	reportar("P08", "reproducibilidad", iguales);
}

void pruebaColorIncorrecto() {
	Jugador jugador;
	jugador.incrementarCombo();
	jugador.incrementarCombo();

	Enemigo enemigo = crearEnemigo(1);
	enemigo.r = 120.0;
	enemigo.color = MAGENTA;
	const int vidaInicial = enemigo.vida;

	Bala bala = crearBala(0);
	bala.color = CIAN;

	resolverImpacto(bala, enemigo, jugador);

	const bool correcto =
		fabs(enemigo.r - 129.0) < 0.000001 &&
		enemigo.vida == vidaInicial &&
		jugador.obtenerComboActual() == 0 &&
		jugador.obtenerPuntaje() == 0.0f;

	reportar("P09", "impacto de color incorrecto", correcto);
}

void pruebaDivisor() {
	ListaEnemigos enemigos;
	int siguienteId = 10;

	Enemigo anterior = crearEnemigo(1);
	Enemigo divisor = crearEnemigo(2);
	Enemigo posterior = crearEnemigo(3);

	divisor.tipo = TipoEnemigo::Divisor;
	divisor.color = CIAN;
	divisor.r = 150.0;
	divisor.theta = 90.0;

	enemigos.insertarFinal(anterior);
	NodoEnemigo* nodoDivisor = enemigos.insertarFinal(divisor);
	enemigos.insertarFinal(posterior);

	reemplazarPorMinis(enemigos, nodoDivisor, siguienteId);

	NodoEnemigo* mini1 = enemigos.primero()->sig;
	NodoEnemigo* mini2 = mini1->sig;
	NodoEnemigo* nodoPosterior = mini2->sig;

	const bool correcto =
		enemigos.tamano() == 4 &&
		mini1->dato.tipo == TipoEnemigo::Mini &&
		mini2->dato.tipo == TipoEnemigo::Mini &&
		nodoPosterior->dato.id == posterior.id &&
		mini1->dato.color != divisor.color &&
		mini2->dato.color != divisor.color &&
		mini1->dato.color != mini2->dato.color &&
		fabs(mini1->dato.r - divisor.r) < 0.000001 &&
		fabs(mini2->dato.r - divisor.r) < 0.000001;

	reportar("P10", "divisor inserta dos minis en su posición", correcto);
}

void pruebaPulso() {
	Jugador jugador;
	Pulso pulso{ false, 0.0 };

	jugador.aumentarPulso(99.0f);
	activarPulso(pulso, jugador);
	const bool noActivaConCargaIncompleta = !pulso.activo;

	jugador.aumentarPulso(1.0f);
	activarPulso(pulso, jugador);

	ListaEnemigos enemigos;
	Enemigo dentro = crearEnemigo(1);
	dentro.r = 32.0;
	dentro.color = MAGENTA;

	Enemigo fuera = crearEnemigo(2);
	fuera.r = 100.0;

	enemigos.insertarFinal(dentro);
	enemigos.insertarFinal(fuera);

	avanzarPulso(pulso, enemigos, jugador);

	const bool correcto =
		noActivaConCargaIncompleta &&
		pulso.activo &&
		jugador.obtenerCargaPulse() == 0.0f &&
		enemigos.tamano() == 1 &&
		enemigos.primero()->dato.id == fuera.id &&
		jugador.obtenerPuntaje() == puntosPorTipo(TipoEnemigo::Dron) &&
		jugador.obtenerComboActual() == 0;

	reportar("P11", "activación y bajas por pulso", correcto);
}

void pruebasMotor() {
	pruebaReproducibilidad();
	pruebaColorIncorrecto();
	pruebaDivisor();
	pruebaPulso();
	pruebaCopiaProfundaOleada();
}


// ============================================================
// SCRIPTS: pruebas de motor y de vista
// ============================================================

// La siguiente parte es la consola por lotes para el programa, la cual cumple que el motor del juego se ejecuta sin utilizar allegro.
// Los siguientes const son para configurar la consola como por ejemplo donde se guardan y leen los lotes o establecer limites de seguridad del programa.
const string RUTA_HISTORIAL = "historial.csv";
const int LIMITE_TICKS_OLEADA = 100000;
const string NOMBRES_TIPO[6] = { "Dron", "Girador", "Divisor", "Mini", "Tanque", "Camaleon" };

//La siguiente clase es para que no se permita la salida del cout durante la ejecucción de la partida.
class BufferNulo : public streambuf {
protected:
	int overflow(int c) {
		return c;
	}
};
// Este struct guarda las acciones que realize el jugador durante la partida
struct AccionScript {
	string jugador;
	int oleada;
	int tick;
	TipoAccion tipo;
	int parametro;
};
struct NodoAccionScript {
	AccionScript dato;
	NodoAccionScript* sig;
};
// En este script se guardan todas las acciones del jugador por medio de la lista simple
class ListaScript {
	NodoAccionScript* cabeza;
	NodoAccionScript* cola;
	int tam;
public:
	ListaScript() {
		cabeza = nullptr;
		cola = nullptr;
		tam = 0;
	}
	// Destructor de la lista simple
	~ListaScript() {
		NodoAccionScript* Aux = cabeza;
		while (Aux != nullptr) {
			NodoAccionScript* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	ListaScript(const ListaScript&) = delete;
	ListaScript& operator=(const ListaScript&) = delete;
	// Esta función es para agregar una acción al final de la lista
	void insertarFinal(const AccionScript& a) {
		NodoAccionScript* nuevo = new(NodoAccionScript);
		nuevo->dato = a;
		nuevo->sig = nullptr;
		if (cola == nullptr) {
			cabeza = nuevo;
		}
		else {
			cola->sig = nuevo;
		}
		cola = nuevo;
		tam++;
	}
	template<class Operacion>
	void recorrerTick(const string& jugador, int oleada, int tick, Operacion aplicar) const {
		NodoAccionScript* Aux = cabeza;
		while (Aux != nullptr) {
			if (Aux->dato.jugador == jugador && Aux->dato.oleada == oleada && Aux->dato.tick == tick) {
				aplicar(Aux->dato);
			}
			Aux = Aux->sig;
		}
	}
	// En esta función se devuelve la oleada más grande del script (número mayor)
	int mayorOleada() const {
		int mayor = 0;
		NodoAccionScript* Aux = cabeza;
		while (Aux != nullptr) {
			if (Aux->dato.oleada > mayor) {
				mayor = Aux->dato.oleada;
			}
			Aux = Aux->sig;
		}
		return mayor;
	}

	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};
// Este struct guarda los datos que se indiquen durante la ejecucción como cantidad de oleadas
struct OpcionesLotes {
	bool lotes;
	string alias[16];
	int cantidadJugadores;
	int semilla;
	int oleadas;
	string rutaScript;
	string rutaEstadisticas;
	string rutaRanking;

	OpcionesLotes()
		: lotes(false), cantidadJugadores(0), semilla(42), oleadas(0) {
	}
};
// El bool establece si el alias es un bot
bool esAliasBot(const string& alias) {
	return alias.size() >= 3 && alias.substr(0, 3) == "Bot";
}
// La siguiente función lee el script de las acciones
int cargarScript(const string& ruta, ListaScript& script) {
	ifstream archivo(ruta.c_str());
	if (!archivo.is_open()) {
		return -1;
	}
	string linea;
	int numlin = 0;
	int cargadas = 0;
	while (getline(archivo, linea)) {
		numlin++;
		istringstream entrada(linea);
		string jugador;
		string textoOleada;
		string textoTick;
		string nombreAccion;
		string parametro;

		if (!(entrada >> jugador)) {
			continue;
		}
		if (jugador[0] == '#') {
			continue;
		}
		entrada >> textoOleada >> textoTick >> nombreAccion >> parametro;

		AccionScript a;
		a.jugador = jugador;
		a.oleada = 0;
		a.tick = 0;
		a.tipo = TipoAccion::NONE;
		a.parametro = 0;

		bool valida = valorenteroValido(textoOleada) && valorenteroValido(textoTick);
		if (valida) {
			a.oleada = valorEntero(textoOleada);
			a.tick = valorEntero(textoTick);
		}

		if (nombreAccion == "Color") {
			a.tipo = TipoAccion::CAMBIAR_COLOR;
			if (parametro == "C") {
				a.parametro = 0;
			}
			else if (parametro == "M") {
				a.parametro = 1;
			}
			else if (parametro == "A") {
				a.parametro = 2;
			}
			else {
				valida = false;
			}
		}
		else if (nombreAccion == "Girar_A") {
			a.tipo = TipoAccion::GIRAR_A;
			if (valorenteroValido(parametro) && valorEntero(parametro) <= 359) {
				a.parametro = valorEntero(parametro);
			}
			else {
				valida = false;
			}
		}
		else if (nombreAccion == "Pulso") {
			a.tipo = TipoAccion::ACTIVAR_PULSO;
		}
		else {
			valida = false;
		}
		if (valida == true) {
			script.insertarFinal(a);
			cargadas++;
		}
		else {
			cout << "Linea de script " << numlin << " mal escrita" << endl;
		}
	}
	archivo.close();
	return cargadas;
}


// Esta función lee los argumentos de los comandos del programa, también lee las versiones con doble guion
bool leerArgumentos(int argc, char* argv[], OpcionesLotes& op) {
	for (int i = 1; i < argc; i++) {
		string bandera = argv[i];

		if (bandera == "-lotes" || bandera == "--lotes") {
			op.lotes = true;
		}
		else if (bandera == "--jugadores" || bandera == "--semilla" || bandera == "--oleadas" || bandera == "--script"
			|| bandera == "-estadisticas" || bandera == "--estadisticas" || bandera == "-ranking" || bandera == "--ranking") {
			if (i + 1 >= argc) {
				cout << "Falta el valor de la bandera " << bandera << endl;
				return false;
			}
			string valor = argv[i + 1];
			i++;

			if (bandera == "--jugadores") {
				string campos[16];
				int cantidad = separacionCampos(valor, campos, 16);
				if (cantidad > 16) {
					cout << "El maximo es de 16 jugadores" << endl;
					return false;
				}
				for (int k = 0; k < cantidad; k++) {
					if (campos[k].empty()) {
						cout << "Hay un alias vacio en --jugadores" << endl;
						return false;
					}
					for (int m = 0; m < k; m++) {
						if (campos[m] == campos[k]) {
							cout << "El alias " << campos[k] << " esta repetido" << endl;
							return false;
						}
					}
					op.alias[k] = campos[k];
				}
				op.cantidadJugadores = cantidad;
			}
			else if (bandera == "--semilla") {
				if (!valorenteroValido(valor)) {
					cout << "La semilla debe ser un entero" << endl;
					return false;
				}
				op.semilla = valorEntero(valor);
			}
			else if (bandera == "--oleadas") {
				if (!valorenteroValido(valor) || valorEntero(valor) == 0) {
					cout << "Las oleadas deben ser un entero mayor que cero" << endl;
					return false;
				}
				op.oleadas = valorEntero(valor);
			}
			else if (bandera == "--script") {
				op.rutaScript = valor;
			}
			else if (bandera == "-estadisticas" || bandera == "--estadisticas") {
				op.rutaEstadisticas = valor;
			}
			else {
				op.rutaRanking = valor;
			}
		}
		else {
			cout << "Bandera desconocida: " << bandera << endl;
			return false;
		}
	}
	return true;
}
// Este struct guarda todos los datos necesarios para que el jugador se ejecute de manera correcta
struct JugadorLote {
	string alias;
	bool esBot;
	Motor* motor;
	bool tieneObjetivo;
	float anguloObjetivo;

	JugadorLote()
		: esBot(false), motor(nullptr), tieneObjetivo(false), anguloObjetivo(0.0f) {
	}
};
// Esta clase es la más importante, ya que permite que se ejecute la partida por medio de los lotes
class SimulacionLotes {
	JugadorLote* jugadores;
	int cantidad;
	int semilla;
	int oleadas;
	const ListaScript& script;
	RondaJugadores ronda;
public:
	// La siguiente función es la constructora del programa ya que crea un motor por cada alias
	SimulacionLotes(const OpcionesLotes& op, const ListaScript& s)
		: jugadores(nullptr), cantidad(op.cantidadJugadores), semilla(op.semilla), oleadas(op.oleadas), script(s) {
		jugadores = new JugadorLote[cantidad];
		for (int i = 0; i < cantidad; i++) {
			jugadores[i].alias = op.alias[i];
			jugadores[i].esBot = esAliasBot(op.alias[i]);
			jugadores[i].motor = new Motor(static_cast<uint32_t>(semilla));
			jugadores[i].motor->configurarJugador(i, op.alias[i]);
			ronda.agregar(jugadores[i].motor->punteroJugador());
		}
	}
	// Este es el destructor y libera los motores creados
	~SimulacionLotes() {
		for (int i = 0; i < cantidad; i++) {
			delete jugadores[i].motor;
		}
		delete[] jugadores;
	}
	SimulacionLotes(const SimulacionLotes&) = delete;
	SimulacionLotes& operator=(const SimulacionLotes&) = delete;

	// Esta función busca a qué posición de la lista de jugadores le pertenece el puntero que entrega la ronda, devuelve -1 si no lo encuentra.
	int indiceDe(const Jugador* j) const {
		for (int i = 0; i < cantidad; i++) {
			if (jugadores[i].motor->punteroJugador() == j) {
				return i;
			}
		}
		return -1;
	}
	// Esta función crea la acción del alias (persona)
	Accion construirAccionHumano(JugadorLote& jl, int oleada, int tickRelativo) {
		Accion accion;

		script.recorrerTick(jl.alias, oleada, tickRelativo, [&](const AccionScript& a) {
			if (a.tipo == TipoAccion::CAMBIAR_COLOR) {
				accion.tieneColor = true;
				accion.color = static_cast<Color>(a.parametro);
			}
			else if (a.tipo == TipoAccion::GIRAR_A) {
				jl.tieneObjetivo = true;
				jl.anguloObjetivo = static_cast<float>(a.parametro);
			}
			else if (a.tipo == TipoAccion::ACTIVAR_PULSO) {
				accion.activarPulso = true;
			}
			});

		accion.tieneGiro = jl.tieneObjetivo;
		accion.anguloObjetivo = jl.anguloObjetivo;
		return accion;
	}
	// Esta función es para cuando un jugador juegue una ronda completa
	void jugarOleada(int i, int oleada) {
		JugadorLote& jl = jugadores[i];
		Motor& motor = *jl.motor;
		int ticksOleada = 0;

		while (!motor.terminada() && motor.obtenerOleada() == oleada && ticksOleada < LIMITE_TICKS_OLEADA) {
			if (jl.esBot) {
				motor.tickBot();
			}
			else {
				motor.tick(construirAccionHumano(jl, oleada, motor.obtenerTickRelativo()));
			}
			ticksOleada++;
		}
	}
	// Esta función saca a los jugadores eliminados y si queda almenos un jugador empieza la siguiente oleada
	void retirarEliminados(bool horario) {
		int turnos = ronda.tamano();
		for (int i = 0; i < turnos; i++) {
			int indice = indiceDe(ronda.verActual());
			if (jugadores[indice].motor->terminada()) {
				ronda.eliminarActual(horario);
			}
			else {
				ronda.avanzar(horario);
			}
		}
	}
	// Esta función permite que se realize la ejecucción completa
	void ejecutar() {
		for (int oleada = 1; oleada <= oleadas && !ronda.vacia(); oleada++) {
			ejecutarronda(ronda, oleada, [&](Jugador* j) {
				jugarOleada(indiceDe(j), oleada);
				});
			retirarEliminados(rondaimpar(oleada));
		}
	}
	// Esta función da como resultado la ronda en la que se termino la partida ( número de oleada)
	int oleadaAlcanzada(int i) const {
		const Motor& m = *jugadores[i].motor;
		if (m.terminada()) {
			return m.obtenerOleada();
		}
		return m.obtenerOleada() - 1;
	}
	// Esta función da la posición en la cual quedo el jugador
	int posicionDe(int i) const {
		int posicion = 1;
		float propio = jugadores[i].motor->obtenerJugador().obtenerPuntaje();
		for (int k = 0; k < cantidad; k++) {
			if (jugadores[k].motor->obtenerJugador().obtenerPuntaje() > propio) {
				posicion++;
			}
		}
		return posicion;
	}
	// Esta función imprime lo que le ocurrio a cada jugador en cada oleada y los datos salen de la bitácora
	void imprimirLineaTiempo(int i) const {
		const Motor& m = *jugadores[i].motor;
		int alcanzada = oleadaAlcanzada(i);

		cout << "Linea de tiempo por oleada:" << endl;
		for (int o = 1; o <= alcanzada; o++) {
			int bajasOleada = 0;
			int erroneosOleada = 0;
			int danoOleada = 0;
			int choquesOleada = 0;
			int pulsosOleada = 0;
			int tickFin = -1;
			bool eliminado = false;

			m.obtenerBitacora().seleccionados(i, o, [&](const Registro& r) {
				switch (r.tipo_evento) {
				case TipoEvento::Baja:
					bajasOleada++;
					break;
				case TipoEvento::Impacto_erroneo:
					erroneosOleada++;
					break;
				case TipoEvento::Dano_nucleo:
					danoOleada += r.valor;
					break;
				case TipoEvento::Choque_nave:
					choquesOleada++;
					danoOleada += r.valor;
					break;
				case TipoEvento::Pulso:
					pulsosOleada++;
					break;
				case TipoEvento::Fin_Oleada:
					tickFin = r.tick;
					break;
				case TipoEvento::Eliminacion:
					eliminado = true;
					tickFin = r.tick;
					break;
				}
				});

			cout << "  Oleada " << setw(2) << o
				<< " | bajas: " << setw(3) << bajasOleada
				<< " | erroneos: " << setw(3) << erroneosOleada
				<< " | dano: " << setw(3) << danoOleada
				<< " | choques: " << choquesOleada
				<< " | pulsos: " << pulsosOleada;
			if (eliminado == true) {
				cout << " | Eliminado en el tick " << tickFin;
			}
			else if (tickFin >= 0) {
				cout << " | fin en el tick " << tickFin;
			}
			cout << endl;
		}
	}
	// Esta función imprime los resultados de cada jugador
	void imprimirJugador(int i) const {
		const JugadorLote& jl = jugadores[i];
		const Motor& m = *jl.motor;
		int disparos = m.obtenerDisparos();
		double precision = 0.0;
		if (disparos > 0) {
			precision = 100.0 * m.obtenerCorrectos() / disparos;
		}

		cout << "----- Jugador " << jl.alias << (jl.esBot ? " (BOT)" : " (humano)") << " -----" << endl;
		cout << "Posicion final: " << posicionDe(i)
			<< " | Puntaje: " << static_cast<int>(m.obtenerJugador().obtenerPuntaje())
			<< " | Oleada alcanzada: " << oleadaAlcanzada(i)
			<< " | Ticks: " << m.obtenerTickActual() << endl;
		cout << "Disparos: " << disparos
			<< " | Correctos: " << m.obtenerCorrectos()
			<< " | Fallidos: " << m.obtenerErroneos()
			<< " | Precision: " << fixed << setprecision(1) << precision << "%" << endl;
		cout.unsetf(ios::fixed);
		cout << setprecision(6);
		cout << "Combo maximo: " << static_cast<int>(m.obtenerJugador().obtenerComboMaximo())
			<< " | Pulsos usados: " << m.obtenerPulsos()
			<< " | Daño total al nucleo: " << m.obtenerDanoTotal() << endl;
		cout << "Bajas:";
		for (int t = 0; t < 6; t++) {
			cout << " " << NOMBRES_TIPO[t] << " " << m.obtenerBajas(t);
			if (t < 5) {
				cout << " |";
			}
		}
		cout << endl;
		imprimirLineaTiempo(i);
	}
	void imprimirReporte() const {
		cout << "===== REPORTE FINAL DE LA PARTIDA =====" << endl;
		for (int i = 0; i < cantidad; i++) {
			imprimirJugador(i);
		}
	}
	// Esta función guarda al historial el registro de los datos de cada jugador
	void guardarEnHistorial(historialPartidas& historial, int idPartida) const {
		for (int i = 0; i < cantidad; i++) {
			const Motor& m = *jugadores[i].motor;
			RegistroPartida r;
			r.id_partida = idPartida;
			r.semilla = semilla;
			r.modo = "lotes";
			r.jugador = jugadores[i].alias;
			r.es_bot = jugadores[i].esBot ? 1 : 0;
			r.posicion = posicionDe(i);
			r.puntaje = static_cast<int>(m.obtenerJugador().obtenerPuntaje());
			r.oleada = oleadaAlcanzada(i);
			r.disparos = m.obtenerDisparos();
			r.correctos = m.obtenerCorrectos();
			r.erroneos = m.obtenerErroneos();
			r.combo_max = static_cast<int>(m.obtenerJugador().obtenerComboMaximo());
			r.pulsos = m.obtenerPulsos();
			r.dano_total = m.obtenerDanoTotal();
			r.ticks = m.obtenerTickActual();
			r.bajas_dron = m.obtenerBajas(0);
			r.bajas_girador = m.obtenerBajas(1);
			r.bajas_divisor = m.obtenerBajas(2);
			r.bajas_mini = m.obtenerBajas(3);
			r.bajas_tanque = m.obtenerBajas(4);
			r.bajas_camaleon = m.obtenerBajas(5);
			historial.registroFinal(r);
		}
	}
};
int siguienteIdPartida(const historialPartidas& historial) {
	int mayor = 0;
	NodoRegistroPartida* Aux = historial.primero();
	while (Aux != nullptr) {
		if (Aux->dato.id_partida > mayor) {
			mayor = Aux->dato.id_partida;
		}
		Aux = Aux->sig;
	}
	return mayor + 1;
}
// La siguiente función es la cual permite que se ejecute el programa por que da el funcionamiento de los lotes y brinda todos los datos como cargar los scripts etc.
int ejecutarModoLotes(OpcionesLotes& op) {
	if (op.cantidadJugadores == 0) {
		cout << "Falta la bandera --jugadores con la lista de alias" << endl;
		return 1;
	}

	ListaScript script;
	if (op.rutaScript != "") {
		if (cargarScript(op.rutaScript, script) < 0) {
			cout << "No se pudo abrir el script " << op.rutaScript << endl;
			return 1;
		}
	}

	if (op.oleadas == 0) {
		op.oleadas = 10;
		if (script.mayorOleada() > 0) {
			op.oleadas = script.mayorOleada();
		}
	}

	cout << "Modo por lotes | Jugadores: " << op.cantidadJugadores
		<< " | Semilla: " << op.semilla
		<< " | Oleadas: " << op.oleadas << endl;

	SimulacionLotes simulacion(op, script);

	BufferNulo nulo;
	streambuf* original = cout.rdbuf(&nulo);
	simulacion.ejecutar();
	cout.rdbuf(original);

	simulacion.imprimirReporte();

	historialPartidas historial;
	historial.cargaHistorial(RUTA_HISTORIAL);
	simulacion.guardarEnHistorial(historial, siguienteIdPartida(historial));
	if (!historial.guardarArchivo(RUTA_HISTORIAL)) {
		cout << "No se pudo guardar el archivo " << RUTA_HISTORIAL << endl;
	}

	cout << "===== ESTADISTICAS HISTORICAS =====" << endl;
	historial.imprimirtodasEstadisticas();

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

// ============================================================
// VISTA (solo lee el estado del motor, no aplica reglas)
// ============================================================

namespace {
	constexpr float CENTRO_X = 400.0f;
	constexpr float CENTRO_Y = 300.0f;
	constexpr float RADIO_ORBITA = 100.0f;
	constexpr float RADIO_NUCLEO = 20.0f;
	constexpr float TAMANO_NAVE = 6.0f;
	constexpr float TAMANO_BALA = 3.0f;

	// Convierte (r, theta en grados) del motor a coordenadas de pantalla.
	void posicionEnPantalla(double r, double thetaGrados, float& x, float& y) {
		double rad = aRadianes(thetaGrados);
		x = CENTRO_X + static_cast<float>(r * cos(rad));
		y = CENTRO_Y + static_cast<float>(r * sin(rad));
	}

	// Los valores 0, 1 y 2 coinciden con CIAN, MAGENTA y AMARILLO.
	ALLEGRO_COLOR colorDeIndice(int indice) {
		switch (indice) {
		case CIAN:     return al_map_rgb(0, 255, 255);
		case MAGENTA:  return al_map_rgb(255, 0, 255);
		default:       return al_map_rgb(255, 255, 0);
		}
	}

	void dibujarNucleo() {
		al_draw_circle(CENTRO_X, CENTRO_Y, RADIO_ORBITA, al_map_rgb(70, 70, 70), 1.0f);
		al_draw_filled_circle(CENTRO_X, CENTRO_Y, RADIO_NUCLEO, al_map_rgb(255, 255, 255));
	}

	void dibujarPulso(const Pulso& pulso) {
		if (pulso.activo) {
			al_draw_circle(CENTRO_X, CENTRO_Y,
				static_cast<float>(pulso.radio_actual),
				al_map_rgb(255, 255, 255), 2.0f);
		}
	}

	void dibujarEnemigos(const ListaEnemigos& enemigos) {
		for (const NodoEnemigo* actual = enemigos.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Enemigo& e = actual->dato;
			float x, y;
			posicionEnPantalla(e.r, e.theta, x, y);
			al_draw_filled_circle(x, y, static_cast<float>(e.radio), colorDeIndice(e.color));
		}
	}

	void dibujarBalas(const ListaBalas& balas) {
		for (const NodoBala* actual = balas.primero();
			actual != nullptr;
			actual = actual->sig) {
			const Bala& b = actual->dato;
			al_draw_filled_circle(CENTRO_X + static_cast<float>(b.x),
				CENTRO_Y + static_cast<float>(b.y),
				TAMANO_BALA, colorDeIndice(b.color));
		}
	}

	void dibujarNave(const Jugador& jugador) {
		float x, y;
		posicionEnPantalla(RADIO_ORBITA, jugador.obtenerPosicionAngular(), x, y);
		al_draw_filled_circle(x, y, TAMANO_NAVE, colorDeIndice(jugador.obtenerColorActual()));
	}
}

void dibujarMundo(const Motor& motor) {
	dibujarNucleo();
	dibujarPulso(motor.obtenerPulso());
	dibujarEnemigos(motor.obtenerEnemigos());
	dibujarBalas(motor.obtenerBalas());
	dibujarNave(motor.obtenerJugador());
}



// ============================================================
// ENTRADA (traduce teclado y mouse a Accion, sin aplicar reglas)
// ============================================================

namespace {
	constexpr float VELOCIDAD_GIRO = 4.0f; // grados por tick
}

struct EstadoEntrada {
	bool izquierda = false;
	bool derecha = false;
	bool pausado = false;
	bool pulsoPedido = false;
	bool colorPedido = false;
	Color colorSeleccionado = MAGENTA;
	bool mouseMovido = false;
	float anguloMouse = 0.0f;
};

void procesarEvento(const ALLEGRO_EVENT& ev, EstadoEntrada& e) {
	switch (ev.type) {
	case ALLEGRO_EVENT_KEY_DOWN:
		switch (ev.keyboard.keycode) {
		case ALLEGRO_KEY_LEFT:  e.izquierda = true; break;
		case ALLEGRO_KEY_RIGHT: e.derecha = true; break;
		case ALLEGRO_KEY_1:
		case ALLEGRO_KEY_PAD_1:
			e.colorSeleccionado = CIAN; e.colorPedido = true; break;
		case ALLEGRO_KEY_2:
		case ALLEGRO_KEY_PAD_2:
			e.colorSeleccionado = MAGENTA; e.colorPedido = true; break;
		case ALLEGRO_KEY_3:
		case ALLEGRO_KEY_PAD_3:
			e.colorSeleccionado = AMARILLO; e.colorPedido = true; break;
		case ALLEGRO_KEY_SPACE: e.pulsoPedido = true; break;
		case ALLEGRO_KEY_P:
		case ALLEGRO_KEY_ESCAPE: e.pausado = !e.pausado; break;
		}
		break;

	case ALLEGRO_EVENT_KEY_UP:
		if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)  e.izquierda = false;
		if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) e.derecha = false;
		break;

	case ALLEGRO_EVENT_MOUSE_AXES: {
		float dx = static_cast<float>(ev.mouse.x) - CENTRO_X;
		float dy = static_cast<float>(ev.mouse.y) - CENTRO_Y;
		if (dx != 0.0f || dy != 0.0f) {
			e.anguloMouse = normalizarAngulo(
				static_cast<float>(atan2(dy, dx) * 180.0 / PI));
			e.mouseMovido = true;
		}
		break;
	}

	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		if (ev.mouse.button == 1) {
			e.pulsoPedido = true;
		}
		break;
	}
}

// Las flechas tienen prioridad sobre el mouse. Los eventos de un solo uso se consumen aquí.
Accion construirAccion(EstadoEntrada& e, const Jugador& jugador) {
	Accion accion;

	if (e.izquierda || e.derecha) {
		float delta = e.derecha ? VELOCIDAD_GIRO : -VELOCIDAD_GIRO;
		accion.tieneGiro = true;
		accion.anguloObjetivo =
			normalizarAngulo(jugador.obtenerPosicionAngular() + delta);
	}
	else if (e.mouseMovido) {
		accion.tieneGiro = true;
		accion.anguloObjetivo = e.anguloMouse;
		e.mouseMovido = false;
	}

	if (e.colorPedido) {
		accion.tieneColor = true;
		accion.color = e.colorSeleccionado;
		e.colorPedido = false;
	}

	accion.activarPulso = e.pulsoPedido;
	e.pulsoPedido = false;

	return accion;
}

void dibujarPausa() {
	al_draw_filled_rectangle(380.0f, 280.0f, 392.0f, 320.0f, al_map_rgb(255, 255, 255));
	al_draw_filled_rectangle(408.0f, 280.0f, 420.0f, 320.0f, al_map_rgb(255, 255, 255));
}

void dibujarHUD(const Motor& motor, const ALLEGRO_FONT* fuente, const char* nombreJugador) {
	const Jugador& jugador = motor.obtenerJugador();
	const ALLEGRO_COLOR blanco = al_map_rgb(255, 255, 255);

	al_draw_textf(fuente, blanco, 10.0f, 10.0f, ALLEGRO_ALIGN_LEFT,
		"Jugador: %s", nombreJugador);
	al_draw_textf(fuente, blanco, 10.0f, 28.0f, ALLEGRO_ALIGN_LEFT,
		"Puntaje: %.0f", jugador.obtenerPuntaje());
	al_draw_textf(fuente, blanco, 10.0f, 46.0f, ALLEGRO_ALIGN_LEFT,
		"Multiplicador: x%d", jugador.obtenerComboActual());
	al_draw_textf(fuente, blanco, 10.0f, 64.0f, ALLEGRO_ALIGN_LEFT,
		"Oleada: %d", motor.obtenerOleada());
	al_draw_textf(fuente, blanco, 10.0f, 82.0f, ALLEGRO_ALIGN_LEFT,
		"Vida nucleo: %d", jugador.obtenerVidaNucleo());
	al_draw_textf(fuente, blanco, 10.0f, 100.0f, ALLEGRO_ALIGN_LEFT,
		"Carga pulso: %.0f%%", jugador.obtenerCargaPulse());

	const float carga = jugador.obtenerCargaPulse();
	al_draw_rectangle(10.0f, 120.0f, 110.0f, 130.0f, blanco, 1.0f);
	al_draw_filled_rectangle(10.0f, 120.0f, 10.0f + carga, 130.0f,
		al_map_rgb(255, 255, 0));
}

// ============================================================
// PANTALLAS (estetica Game Boy)
// ============================================================

constexpr int MAX_JUGADORES = 4;
constexpr int LARGO_NOMBRE = 10;
constexpr int OPCIONES_MENU = 5;

enum class Pantalla { Titulo, Menu, RegistroJugadores, Turno, Juego, Pausa, Resumen, Fin, Estadisticas, Ranking };

enum IndiceGB { GB_OSCURO, GB_MEDIO_OSCURO, GB_MEDIO_CLARO, GB_CLARO };

// Paleta pixel art: azul noche, morado, magenta y amarillo.
ALLEGRO_COLOR colorGB(int indice) {
	switch (indice) {
	case 0: return al_map_rgb(16, 12, 40);     // fondo
	case 1: return al_map_rgb(70, 40, 120);    // sombra / marco
	case 2: return al_map_rgb(255, 60, 200);   // magenta
	default: return al_map_rgb(255, 230, 80);  // amarillo
	}
}

const int TAM_PIXEL = 8;

// Glifo 5x5 por carácter: '1' es píxel encendido.
const char* glifo(char c) {
	switch (c) {
	case 'A': return "01110" "10001" "11111" "10001" "10001";
	case 'B': return "11110" "10001" "11110" "10001" "11110";
	case 'E': return "11111" "10000" "11110" "10000" "11111";
	case 'F': return "11111" "10000" "11110" "10000" "10000";
	case 'I': return "11111" "00100" "00100" "00100" "11111";
	case 'L': return "10000" "10000" "10000" "10000" "11111";
	case 'O': return "01110" "10001" "10001" "10001" "01110";
	case 'R': return "11110" "10001" "11110" "10100" "10010";
	case 'S': return "01111" "10000" "01110" "00001" "11110";
	case 'T': return "11111" "00100" "00100" "00100" "00100";
	default:  return "00000" "00000" "00000" "00000" "00000";
	}
}

void dibujarTextoPixel(const char* texto, float x, float y, ALLEGRO_COLOR color) {
	for (const char* p = texto; *p != '\0'; ++p) {
		const char* g = glifo(*p);
		for (int fila = 0; fila < 5; ++fila) {
			for (int col = 0; col < 5; ++col) {
				if (g[fila * 5 + col] == '1') {
					float px = x + col * TAM_PIXEL;
					float py = y + fila * TAM_PIXEL;
					al_draw_filled_rectangle(px, py, px + TAM_PIXEL, py + TAM_PIXEL, color);
				}
			}
		}
		x += 6 * TAM_PIXEL;
	}
}

// Título centrado con sombra de un píxel de desplazamiento.
void dibujarTituloPixel(const char* texto, float y) {
	float ancho = static_cast<float>(std::strlen(texto) * 6 * TAM_PIXEL - TAM_PIXEL);
	float x = (800.0f - ancho) / 2.0f;
	dibujarTextoPixel(texto, x + TAM_PIXEL, y + TAM_PIXEL, colorGB(1));
	dibujarTextoPixel(texto, x, y, colorGB(3));
}

// Marco de bloques alternados alrededor de la pantalla.
void dibujarMarco() {
	for (int i = 0; i < 800; i += TAM_PIXEL * 2) {
		ALLEGRO_COLOR c = (i / TAM_PIXEL) % 2 == 0 ? colorGB(1) : colorGB(2);
		al_draw_filled_rectangle(i, 0, i + TAM_PIXEL, TAM_PIXEL, c);
		al_draw_filled_rectangle(i, 600 - TAM_PIXEL, i + TAM_PIXEL, 600, c);
	}
	for (int j = 0; j < 600; j += TAM_PIXEL * 2) {
		ALLEGRO_COLOR c = (j / TAM_PIXEL) % 2 == 0 ? colorGB(1) : colorGB(2);
		al_draw_filled_rectangle(0, j, TAM_PIXEL, j + TAM_PIXEL, c);
		al_draw_filled_rectangle(800 - TAM_PIXEL, j, 800, j + TAM_PIXEL, c);
	}
}

void texto(const ALLEGRO_FONT* fuente, float y, ALLEGRO_COLOR color, const char* t) {
	al_draw_text(fuente, color, 400.0f, y, ALLEGRO_ALIGN_CENTRE, t);
}

// Pantalla de inicio: fondo, marco, título y aviso.
void dibujarPantallaTitulo(const ALLEGRO_FONT* fuente) {
	al_clear_to_color(colorGB(0));
	dibujarMarco();
	dibujarTituloPixel("FASE ORBITAL", 220.0f);
	texto(fuente, 400.0f, colorGB(3), "PRESIONA ENTER");
}



struct Sesion {
	Pantalla actual = Pantalla::Titulo;
	int opcionMenu = 0;
	int cantidadJugadores = 1;
	int jugadorRegistrando = 0;
	int jugadorActivo = 0;
	char nombres[MAX_JUGADORES][LARGO_NOMBRE + 1] = {};
	Motor* juego = nullptr;
};

void iniciarJuego(Sesion& s, EstadoEntrada& e) {
	delete s.juego;
	s.juego = new Motor();
	e = EstadoEntrada();
	s.actual = Pantalla::Juego;
}

void procesarPantalla(const ALLEGRO_EVENT& ev, Sesion& s, EstadoEntrada& e, bool& salir) {
	const bool teclaAbajo = ev.type == ALLEGRO_EVENT_KEY_DOWN;
	const int tecla = ev.keyboard.keycode;
	const bool enter = teclaAbajo && (tecla == ALLEGRO_KEY_ENTER || tecla == ALLEGRO_KEY_PAD_ENTER);
	const bool pausa = teclaAbajo && (tecla == ALLEGRO_KEY_P || tecla == ALLEGRO_KEY_ESCAPE);
	const bool arriba = teclaAbajo && tecla == ALLEGRO_KEY_UP;
	const bool abajo = teclaAbajo && tecla == ALLEGRO_KEY_DOWN;
	const bool izquierda = teclaAbajo && tecla == ALLEGRO_KEY_LEFT;
	const bool derecha = teclaAbajo && tecla == ALLEGRO_KEY_RIGHT;

	switch (s.actual) {
	case Pantalla::Titulo:
		if (enter) s.actual = Pantalla::Menu;
		break;
	case Pantalla::Menu:
		if (arriba) s.opcionMenu = (s.opcionMenu + OPCIONES_MENU - 1) % OPCIONES_MENU;
		if (abajo) s.opcionMenu = (s.opcionMenu + 1) % OPCIONES_MENU;
		if (s.opcionMenu == 0 && izquierda && s.cantidadJugadores > 1) --s.cantidadJugadores;
		if (s.opcionMenu == 0 && derecha && s.cantidadJugadores < MAX_JUGADORES) ++s.cantidadJugadores;
		if (enter) {
			if (s.opcionMenu == 1) {
				for (int i = 0; i < MAX_JUGADORES; ++i) s.nombres[i][0] = '\0';
				s.jugadorRegistrando = 0;
				s.actual = Pantalla::RegistroJugadores;
			}
			else if (s.opcionMenu == 2) {
				s.actual = Pantalla::Estadisticas;
			}
			else if (s.opcionMenu == 3) {
				s.actual = Pantalla::Ranking;
			} 
			else if (s.opcionMenu == 4) {
				salir = true;
			}
		}
		break;

	case Pantalla::RegistroJugadores: {
		char* nombre = s.nombres[s.jugadorRegistrando];
		const int largo = static_cast<int>(strlen(nombre));

		if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
			const int c = ev.keyboard.unichar;
			if (c >= 32 && c <= 126 && largo < LARGO_NOMBRE) {
				nombre[largo] = static_cast<char>(c);
				nombre[largo + 1] = '\0';
			}
		}
		else if (teclaAbajo && tecla == ALLEGRO_KEY_BACKSPACE && largo > 0) {
			nombre[largo - 1] = '\0';
		}
		else if (enter && largo > 0) {
			++s.jugadorRegistrando;
			if (s.jugadorRegistrando == s.cantidadJugadores) {
				s.jugadorActivo = 0;
				s.actual = Pantalla::Turno;
			}
		}
		break;
	}

	case Pantalla::Turno:
		if (enter) iniciarJuego(s, e);
		break;

	case Pantalla::Juego:
		if (pausa) s.actual = Pantalla::Pausa;
		break;

	case Pantalla::Pausa:
		if (pausa) s.actual = Pantalla::Juego;
		break;

	case Pantalla::Resumen:
		if (enter) s.actual = Pantalla::Fin;
		break;

	case Pantalla::Fin:
		if (enter) {
			if (s.jugadorActivo + 1 < s.cantidadJugadores) {
				++s.jugadorActivo;
				s.actual = Pantalla::Turno;
			}
			else {
				s.actual = Pantalla::Estadisticas;
			}
		}
		break;

	case Pantalla::Estadisticas:
		if (enter) s.actual = Pantalla::Ranking;
		break;

	case Pantalla::Ranking:
		if (enter) s.actual = Pantalla::Menu;
		break;
	}
}

void dibujarPantalla(const Sesion& s, const ALLEGRO_FONT* fuente) {
	const ALLEGRO_COLOR claro = colorGB(GB_CLARO);
	const ALLEGRO_COLOR medio = colorGB(GB_MEDIO_CLARO);
	const ALLEGRO_COLOR medioOscuro = colorGB(GB_MEDIO_OSCURO);
	const ALLEGRO_COLOR oscuro = colorGB(GB_OSCURO);
	const bool enJuego = s.actual == Pantalla::Juego || s.actual == Pantalla::Pausa;

	if (enJuego) {
		al_clear_to_color(al_map_rgb(0, 0, 0));
		if (s.juego != nullptr) {
			dibujarMundo(*s.juego);
			dibujarHUD(*s.juego, fuente, s.nombres[s.jugadorActivo]);
		}
	}
	else {
		al_clear_to_color(claro);
		al_draw_rectangle(8.0f, 8.0f, 792.0f, 592.0f, oscuro, 6.0f);
	}

	switch (s.actual) {
	case Pantalla::Titulo: 
		dibujarPantallaTitulo(fuente);
		break;
	case Pantalla::Menu: {
		const char* etiquetas[OPCIONES_MENU] = { "JUGADORES", "JUGAR", "ESTADISTICAS", "RANKING", "SALIR" };
		texto(fuente, 60.0f, oscuro, "FASE ORBITAL");
		for (int i = 0; i < OPCIONES_MENU; ++i) {
			const char* marca = (i == s.opcionMenu) ? "> " : "  ";
			char linea[48];
			if (i == 0) {
				snprintf(linea, sizeof linea, "%s%s < %d >", marca, etiquetas[i], s.cantidadJugadores);
			}
			else {
				snprintf(linea, sizeof linea, "%s%s", marca, etiquetas[i]);
			}
			texto(fuente, 150.0f + i * 40.0f, i == s.opcionMenu ? oscuro : medioOscuro, linea);
		}
		break;
	}

	case Pantalla::RegistroJugadores: {
		char titulo[32];
		snprintf(titulo, sizeof titulo, "JUGADOR %d", s.jugadorRegistrando + 1);
		char linea[LARGO_NOMBRE + 2];
		snprintf(linea, sizeof linea, "%s_", s.nombres[s.jugadorRegistrando]);
		texto(fuente, 120.0f, oscuro, titulo);
		texto(fuente, 180.0f, oscuro, linea);
		texto(fuente, 300.0f, medioOscuro, "ESCRIBE TU NOMBRE");
		texto(fuente, 330.0f, medioOscuro, "ENTER para confirmar");
		break;
	}

	case Pantalla::Turno:
		texto(fuente, 150.0f, medioOscuro, "TURNO DE");
		texto(fuente, 190.0f, oscuro, s.nombres[s.jugadorActivo]);
		texto(fuente, 300.0f, medioOscuro, "ENTER para empezar");
		break;

	case Pantalla::Juego:
		break;

	case Pantalla::Pausa:
		al_draw_filled_rectangle(0.0f, 0.0f, 800.0f, 600.0f, al_map_rgba(0, 0, 0, 160));
		texto(fuente, 280.0f, claro, "PAUSA");
		texto(fuente, 310.0f, claro, "P / ESC para continuar");
		break;

	case Pantalla::Resumen:
		texto(fuente, 100.0f, oscuro, "RESUMEN");
		if (s.juego != nullptr) {
			const Jugador& jugador = s.juego->obtenerJugador();
			al_draw_textf(fuente, oscuro, 400.0f, 160.0f, ALLEGRO_ALIGN_CENTRE,
				"Puntaje: %.0f", jugador.obtenerPuntaje());
			al_draw_textf(fuente, oscuro, 400.0f, 180.0f, ALLEGRO_ALIGN_CENTRE,
				"Combo maximo: %d", jugador.obtenerComboMaximo());
			al_draw_textf(fuente, oscuro, 400.0f, 200.0f, ALLEGRO_ALIGN_CENTRE,
				"Oleada: %d", s.juego->obtenerOleada());
		}
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Fin:
		texto(fuente, 200.0f, oscuro, "FIN DE PARTIDA");
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Estadisticas:
		texto(fuente, 100.0f, oscuro, "ESTADISTICAS");
		texto(fuente, 200.0f, medioOscuro, "Sin datos: falta conectar historial.csv");
		texto(fuente, 300.0f, medioOscuro, "ENTER para continuar");
		break;

	case Pantalla::Ranking:
		texto(fuente, 100.0f, oscuro, "RANKING");
		texto(fuente, 200.0f, medioOscuro, "Sin datos: falta conectar historial.csv");
		texto(fuente, 300.0f, medioOscuro, "ENTER para volver al menu");
		break;
	}
}

int main() {
	if (!al_init()) {
		fprintf(stderr, "No se pudo iniciar Allegro.\n");
		return -1;
	}

	al_install_keyboard();
	al_install_mouse();
	al_init_primitives_addon();
	al_init_font_addon();

	ALLEGRO_DISPLAY* display = al_create_display(800, 600);
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
	ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();
	ALLEGRO_FONT* fuente = al_create_builtin_font();

	if (!display || !timer || !cola || !fuente) {
		fprintf(stderr, "No se pudo crear display, timer, cola de eventos o fuente.\n");
		if (fuente) al_destroy_font(fuente);
		if (cola) al_destroy_event_queue(cola);
		if (timer) al_destroy_timer(timer);
		if (display) al_destroy_display(display);
		return -1;
	}

	al_set_window_title(display, "Fase Orbital");

	al_register_event_source(cola, al_get_keyboard_event_source());
	al_register_event_source(cola, al_get_mouse_event_source());
	al_register_event_source(cola, al_get_display_event_source(display));
	al_register_event_source(cola, al_get_timer_event_source(timer));

	al_start_timer(timer);

	Sesion sesion;
	EstadoEntrada entrada;
	bool salir = false;

	while (!salir) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(cola, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			salir = true;
			continue;
		}

		procesarEvento(ev, entrada);
		procesarPantalla(ev, sesion, entrada, salir);

		if (ev.type == ALLEGRO_EVENT_TIMER) {
			if (sesion.actual == Pantalla::Juego && sesion.juego != nullptr) {
				sesion.juego->tick(construirAccion(entrada, sesion.juego->obtenerJugador()));
				if (sesion.juego->terminada()) {
					sesion.actual = Pantalla::Resumen;
				}
			}

			dibujarPantalla(sesion, fuente);
			al_flip_display();
		}
	}

	delete sesion.juego;
	al_stop_timer(timer);
	al_destroy_font(fuente);
	al_destroy_event_queue(cola);
	al_destroy_timer(timer);
	al_destroy_display(display);

	return 0;
}