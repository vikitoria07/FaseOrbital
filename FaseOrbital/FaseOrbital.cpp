// FaseOrbital.cpp : ListaEnemigos utiliza listas dobles y ListaBalas utiliza listas simples, además, incluye las pruebas solicitadas P01 a P04.
#include <iostream>
using namespace std;

// En esta parte se establecen los  seis tipos de enemigos del programa. Guarda los datos de cada enemigo en el cual id identifica al enemigo, tipo dice que clase es,
// el tipo de color se guarda por medio de números, r indica la distancia al núcleo, theta el ángulo, omega cuanto gira e ángulo por medio de un tick,
// vida indica la cantidad de golpes restantes para que muera, radio es el tamaño en el cual puede moverse y tickscolor para que el camaléon pueda cambiar de color.
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
};

//En el siguiente struct es para establecer la lista doble, cada nodo guarda un tipo de enemigo(dato),
// la lista se puede recorrer hacia adelante y atras gracias a ant y sig.
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
		cabeza = NULL;
		cola = NULL;
		tam = 0;
	}

	// Esta parte se ejecuta cuando la lista deja de existir y libera todos los nodos para no dejar memoria basura.
	// Se avanza al siguiente nodo antes de hacer el delete para poder volver leer la lista al volver a ejecutarse.
	~ListaEnemigos() {
		NodoEnemigo* Aux = cabeza;
		while (Aux != NULL) {
			NodoEnemigo* borrar = Aux;
			Aux = Aux->sig;
			delete borrar;
		}
	}
	ListaEnemigos(const ListaEnemigos&) = delete;
	ListaEnemigos& operator=(const ListaEnemigos&) = delete;

	// la siguiente función es para insertar un enemigo al final de la lista y devuelve el nodo creado.
	//Si la lista estaba vacia, el nodo nuevo es a la vez la cabeza y la cola.
	NodoEnemigo* insertarFinal(const Enemigo& e) {
		NodoEnemigo* nuevo = new(NodoEnemigo);
		nuevo->dato = e;
		nuevo->sig = NULL;
		nuevo->ant = cola;
		if (cola == NULL) {
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
	// Si n era la cola, el nodo nuevo pasa a ser el último nodo y Si n es NULL no se inserta nada.
	NodoEnemigo* insertarDespues(NodoEnemigo* n, const Enemigo& e) {
		if (n == NULL) {
			return NULL;
		}
		NodoEnemigo* nuevo = new(NodoEnemigo);
		nuevo->dato = e;
		nuevo->ant = n;
		nuevo->sig = n->sig;
		if (n->sig != NULL) {
			n->sig->ant = nuevo;
		}
		else {
			cola = nuevo;
		}
		n->sig = nuevo;
		tam++;
		return nuevo;
	}

	// En esta parte se elimina el nodo n y devuelve el nodo siguiente para poder seguir el recorrido de la lista. Si n es NULL no se realiza nada y se devuelve el NULL.
	NodoEnemigo* eliminar(NodoEnemigo* n) {
		if (n == NULL) {
			return NULL;
		}
		NodoEnemigo* siguiente = n->sig;
		if (n->ant != NULL) {
			n->ant->sig = n->sig;
		}
		else {
			cabeza = n->sig;
		}
		if (n->sig != NULL) {
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

// Como en la estructura anterior, se configura la estructura bala que guarda los datos de cada bala
// disparada por la nave(usuario). Como en enemigo, x y y son su posicion y vx y vy como se mueve en su ángulo por medio de tick, el color se le asigna igualmente que a enemigo
// y duenio es el jugador.
struct Bala {
	double x, y, vx, vy;
	int color;
	int duenio;
};

// Este struct establece el nodo de la lista simple de balas.
struct NodoBala {
	Bala dato;
	NodoBala* sig;
};

// Esta clase utiliza la misma estructura como la de enemigos, pero este solo guarda los datos de cabeza por que las balas siempre se agregan al inicio.
class ListaBalas {
	NodoBala* cabeza;
	int tam;
public:
	// Esta parte deja la lista vacía y el tamaño en cero como constructor de la lista.
	ListaBalas() {
		cabeza = NULL;
		tam = 0;
	}

	//  En cambio, esta se puede tomar como la que la elimina ya que recorre la lista y libera todos los nodos.
	~ListaBalas() {
		NodoBala* Aux = cabeza;
		while (Aux != NULL) {
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

	int eliminarSi(bool(*debeEliminarse)(const Bala&)) {
		int borradas = 0;
		NodoBala** p = &cabeza;
		while (*p != NULL) {
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

	// Como en enemigo, se usan como consulta y Se usan const para que no se pueda modificar la lista y devolver la cabeza(primero), cola(último), el tamaño o indicar si esta vacía.
	NodoBala* primero() const { return cabeza; }
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};

// Este bool es para cumplir la condición de eliminarSi el cual recibe una bala y si devuelve true es por que el duenio es par y se debe borrar la bala. 
bool condicionpar(const Bala& b) {
	return b.duenio % 2 == 0;
}

// Este bool es una condición para la prueba del resultado "P04", donde establece la cabeza que es el 7 y dos grupos de balas consecutivas.
bool marcabalas(const Bala& b) {
	return b.duenio == 7 || b.duenio == 6 || b.duenio == 5 || b.duenio == 3 || b.duenio == 2;
}

// Se realiza esta función para crear un enemigo para probarlo y obtener los resultados esperados, se establece su tipo, color, ángulo, disntancia con el núcleo etc.
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

// Para la prueba P01 se ingresan 1000 enemigos se eliminan los id que sean par y se verifica que el tamaño sea 500. Además. el recorrido en la lista hacia adelante y atrás ya que deben dar los mismos id en orden inverso.
bool PruebaP01() {
	ListaEnemigos lista;
	for (int i = 1; i <= 1000; i++) {
		lista.insertarFinal(crearEnemigo(i));
	}
	NodoEnemigo* n = lista.primero();
	while (n != NULL) {
		if (n->dato.id % 2 == 0) {
			n = lista.eliminar(n);
		}
		else {
			n = n->sig;
		}
	}
	if (lista.tamano() != 500) {
		return false;
	}
	int adelante[500];
	int atras[500];
	int cant = 0;
	for (NodoEnemigo* p = lista.primero(); p != NULL; p = p->sig) {
		if (cant >= 500) {
			return false;
		}
		adelante[cant] = p->dato.id;
		cant++;
	}
	if (cant != 500) {
		return false;
	}
	cant = 0;
	for (NodoEnemigo* p = lista.ultimo(); p != NULL; p = p->ant) {
		if (cant >= 500) {
			return false;
		}
		atras[cant] = p->dato.id;
		cant++;
	}
	if (cant != 500) {
		return false;
	}
	for (int i = 0; i < 500; i++) {
		if (adelante[i] != atras[499 - i]) {
			return false;
		}
		if (adelante[i] % 2 == 0) {
			return false;
		}
	}
	return true;
}

// Para la prueba P02, se crean 3 enemigos y se elimina primero la cabeza después la cola y por último el nodo que queda. Al final, la lista debería quedar vacía y cabeza y cola en NULL.
bool PruebaP02() {
	ListaEnemigos lista;
	for (int i = 1; i <= 3; i++) {
		lista.insertarFinal(crearEnemigo(i));
	}
	lista.eliminar(lista.primero());
	if (lista.primero()->dato.id != 2) {
		return false;
	}
	if (lista.primero()->ant != NULL) {
		return false;
	}
	lista.eliminar(lista.ultimo());
	if (lista.ultimo()->dato.id != 2) {
		return false;
	}
	if (lista.ultimo()->sig != NULL) {
		return false;
	}
	if (lista.primero() != lista.ultimo()) {
		return false;
	}
	if (lista.tamano() != 1) {
		return false;
	}
	lista.eliminar(lista.primero());
	if (lista.vacia() == true && lista.primero() == NULL && lista.ultimo() == NULL) {
		return true;
	}
	return false;
}

// Para prueba P03 se debe buscar que insertarDespues se cumpla, por lo que se inserta un enemigo después del ultimo nodo y se verifica que la cola se actualiza al último enemigo en ser insertado.
bool PruebaP03() {
	ListaEnemigos lista;
	lista.insertarFinal(crearEnemigo(1));
	lista.insertarFinal(crearEnemigo(2));
	NodoEnemigo* nuevo = lista.insertarDespues(lista.ultimo(), crearEnemigo(3));
	if (nuevo == NULL) {
		return false;
	}
	if (lista.ultimo() != nuevo) {
		return false;
	}
	if (lista.ultimo()->dato.id != 3) {
		return false;
	}
	if (lista.ultimo()->ant->dato.id != 2) {
		return false;
	}
	if (lista.ultimo()->sig != NULL) {
		return false;
	}
	return true;
}

// Para la última prueba "P04", se insertan 8 balas ( de 0 a 7) y se borran por medio de marcabalas 7,6,5,3 y 2. Se deben de haber quitado esas 5 y solo quedar 4,1 y 0
bool PruebaP04() {
	ListaBalas balas;
	for (int i = 0; i < 8; i++) {
		balas.insertarInicio(crearBala(i));
	}
	int borradas = balas.eliminarSi(marcabalas);
	if (borradas != 5) {
		return false;
	}
	if (balas.tamano() != 3) {
		return false;
	}
	int esperado[3] = { 4, 1, 0 };
	int pos = 0;
	for (NodoBala* p = balas.primero(); p != NULL; p = p->sig) {
		if (marcabalas(p->dato) == true) {
			return false;
		}
		if (pos >= 3 || p->dato.duenio != esperado[pos]) {
			return false;
		}
		pos++;
	}
	if (pos != 3) {
		return false;
	}
	return true;
}

// Esta es la función principal del programa, se encarga de ejecutar las cuatro pruebas y mostrar los resultados.
int main()
{
	reportar("P01", "ListaEnemigos 1000 nodos, borrar ids pares", PruebaP01());
	reportar("P02", "ListaEnemigos eliminar cabeza, cola y unico", PruebaP02());
	reportar("P03", "ListaEnemigos insertarDespues sobre la cola", PruebaP03());
	reportar("P04", "ListaBalas eliminar consecutivas con NodoBala**", PruebaP04());
	return 0;
}