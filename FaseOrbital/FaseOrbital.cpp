// FaseOrbital.cpp : ListaEnemigos utiliza listas dobles y ListaBalas utiliza listas simples, además, incluye las pruebas solicitadas P01 a P04.
#include <iostream>
using namespace std;

// En esta parte se establecen los  seis tipos de enemigos del programa. Guarda los datos de cada enemigo en el cual id identifica al enemigo, tipo dice que clase es,
// el tipo de color se guarda por medio de números, r indica la distancia al núcleo, theta el ángulo, omega cuanto gira e ángulo por medio de un tick,
// vida indica la cantidad de golpes restantes para que muera, radio es el tamaño en el cual puede moverse y tickscolor para que el camaleón pueda cambiar de color.
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
	//Si la lista estaba vacía, el nodo nuevo es a la vez la cabeza y la cola.
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
	// Si n era la cola, el nodo nuevo pasa a ser el último nodo y si n es NULL no se inserta nada.
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

	// En esta parte se elimina el nodo n y devuelve el nodo siguiente para poder seguir el recorrido de la lista. Si n es NULL no se realiza nada y se devuelve NULL.
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
// disparada por la nave(usuario). Como en enemigo, x y y son su posición y vx y vy como se mueve en su ángulo por medio de tick, el color se le asigna igualmente que a enemigo
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

// Esta clase utiliza la misma estructura como la de enemigos, pero esta solo guarda los datos de cabeza por que las balas siempre se agregan al inicio.
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
	template<class Pred>
	int eliminarSi(Pred debeEliminarse) {
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

	// Como en enemigo, se usan como consulta y se usan const para que no se pueda modificar la lista y devolver la cabeza(primero), cola(último), el tamaño o indicar si esta vacía.
	NodoBala* primero() const { return cabeza; }
	int tamano() const { return tam; }
	bool vacia() const { return tam == 0; }
};
// Continuando con la misma base de las estructuras anteriores, este struct guarda los datos de cada aparición pendiente de la oleada (guarda el enemigo que todavía no ha aparecido).
// El tipo en el struct establece que clase de enemigo va a aparecer, color indica el color con el que aparece, theta es el ángulo, tick indica en cuál tick de la oleada le toca aparecer 
// y sentido indica hacia donde gira el enemigo (tomando en cuenta que 0 es horario y 1 antihorario).
struct Aparicion {
	TipoEnemigo tipo;
	int color;
	double theta;
	int tick;
	int sentido;
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
		frente = NULL;
		final = NULL;
		tam = 0;
	}

	// Las siguientes líneas son las constructoras de copia, se hace esta copia para que cuando se recorra la cola original y por cada nodo se cree un nuevo nodo con los mismos datos.
	ColaSpawn(const ColaSpawn& otra) {
		frente = NULL;
		final = NULL;
		tam = 0;
		NodoAparicion* Aux = otra.frente;
		while (Aux != NULL) {
			encolar(Aux->dato);
			Aux = Aux->sig;
		}
	}

	// Este es el operador de asignación, realiza primero una revisión la cual verifica que no se esté asignando la cola a si misma y después de esta revisión
	// libera los nodos que ya tenía esta cola y por último copia uno por uno los nodos de la otra cola.
	ColaSpawn& operator=(const ColaSpawn& otra) {
		if (this != &otra) {
			NodoAparicion* Aux = frente;
			while (Aux != NULL) {
				NodoAparicion* borrar = Aux;
				Aux = Aux->sig;
				delete borrar;
			}
			frente = NULL;
			final = NULL;
			tam = 0;
			Aux = otra.frente;
			while (Aux != NULL) {
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
		while (Aux != NULL) {
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
		nuevo->sig = NULL;
		if (final == NULL) {
			frente = nuevo;
		}
		else {
			final->sig = nuevo;
		}
		final = nuevo;
		tam++;
	}

	// Esta función es para sacar la aparición que está en el frente de la cola. Guarda el dato en salida y devuelve true si se pudo sacar,
	// si la cola está vacía no hace nada y devuelve false. Si al sacarla la cola queda vacía se pone al final en NULL.
	bool desencolar(Aparicion& salida) {
		if (frente == NULL) {
			return false;
		}
		NodoAparicion* borrar = frente;
		salida = borrar->dato;
		frente = borrar->sig;
		if (frente == NULL) {
			final = NULL;
		}
		delete borrar;
		tam--;
		return true;
	}

	// Esta función es para ver la aparición que está en el frente sin sacarla de la cola. Devuelve un puntero const para que no se pueda modificar
	// y si la cola está vacía devuelve NULL.
	const Aparicion* verFrente() const {
		if (frente == NULL) {
			return NULL;
		}
		return &(frente->dato);
	}

	// Se usan const para que no se pueda modificar la cola y devolver el tamaño o indicar si esta vacía.
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
Aparicion crearAparicion(int n) {
	Aparicion a;
	a.tipo = TipoEnemigo::Dron;
	a.color = n % 3;
	a.theta = n * 10.0;
	a.tick = n;
	a.sentido = n % 2;
	return a;
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

// Para la prueba P01 se ingresan 1000 enemigos se eliminan los id que sean par y se verifica que el tamaño sea 500. Además, el recorrido en la lista hacia adelante y atrás ya que deben dar los mismos id en orden inverso.
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

// Para la prueba P02, se crean 3 enemigos y se elimina primero la cabeza, después la cola y por último el nodo que queda. Al final, la lista debería quedar vacía y cabeza y cola en NULL.
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

// Para prueba P03 se debe buscar que insertarDespues se cumpla, por lo que se inserta un enemigo después del último nodo y se verifica que la cola se actualiza al último enemigo en ser insertado.
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

// Para la prueba "P04", se insertan 8 balas ( de 0 a 7) y se borran por medio de marcabalas 7,6,5,3 y 2. Se deben de haber quitado esas 5 y solo quedar 4,1 y 0
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
// Para la prueba P05, se debe verificar que una nueva cola se encuentre vacía, que verFrente de NULL y desencolar devuelva false. Después, se encolan 5 apariciones y se comprueba que verFrente muestre la primera sin sacarla y que al desencolar salgan en el mismo orden en que entraron (1,2,3,4,5).
// Finalmente, la cola debe quedar vacía y volver a encolar una aparición para verificar que la cola siga funcionando.
bool PruebaP05() {
	ColaSpawn cola;
	Aparicion salida;
	if (cola.vacia() == false || cola.verFrente() != NULL || cola.desencolar(salida) == true) {
		return false;
	}
	for (int i = 1; i <= 5; i++) {
		cola.encolar(crearAparicion(i));
	}
	if (cola.tamano() != 5) {
		return false;
	}
	if (cola.verFrente() == NULL || cola.verFrente()->tick != 1) {
		return false;
	}
	if (cola.tamano() != 5) {
		return false;
	}
	for (int i = 1; i <= 5; i++) {
		if (cola.desencolar(salida) == false) {
			return false;
		}
		if (salida.tick != i || salida.theta != i * 10.0) {
			return false;
		}
	}
	if (cola.vacia() == false || cola.verFrente() != NULL || cola.tamano() != 0) {
		return false;
	}
	cola.encolar(crearAparicion(9));
	if (cola.tamano() != 1 || cola.verFrente()->tick != 9) {
		return false;
	}
	return true;
}

// En la prueba P06 Se crea una cola con 5 apariciones y se copia con el constructor de copia y con el operador =. Se sacan todas las apariciones de las copias y se encolan otras, la cola original debe quedar intacta con sus 5 apariciones
// Además en la prueba se da la asignación de una cola a si misma y la asignación sobre una cola que ya tenía datos.
bool PruebaP06() {
	ColaSpawn original;
	for (int i = 1; i <= 5; i++) {
		original.encolar(crearAparicion(i));
	}
	ColaSpawn copia(original);
	ColaSpawn asignada;
	asignada.encolar(crearAparicion(99));
	asignada.encolar(crearAparicion(98));
	asignada = original;
	Aparicion salida;
	for (int i = 1; i <= 5; i++) {
		if (copia.desencolar(salida) == false || salida.tick != i) {
			return false;
		}
	}
	for (int i = 1; i <= 5; i++) {
		if (asignada.desencolar(salida) == false || salida.tick != i) {
			return false;
		}
	}
	if (copia.vacia() == false || asignada.vacia() == false) {
		return false;
	}
	copia.encolar(crearAparicion(50));
	asignada.encolar(crearAparicion(60));
	if (original.tamano() != 5) {
		return false;
	}
	ColaSpawn& referencia = original;
	original = referencia;
	if (original.tamano() != 5) {
		return false;
	}
	for (int i = 1; i <= 5; i++) {
		if (original.desencolar(salida) == false || salida.tick != i || salida.sentido != i % 2) {
			return false;
		}
	}
	if (original.vacia() == false || original.verFrente() != NULL) {
		return false;
	}
	return true;
}

// Esta es la función principal del programa, se encarga de ejecutar las pruebas requeridas de que funcione el programa y mostrar los resultados.
int main()
{
	reportar("P01", "ListaEnemigos 1000 nodos, borrar ids pares", PruebaP01());
	reportar("P02", "ListaEnemigos eliminar cabeza, cola y unico", PruebaP02());
	reportar("P03", "ListaEnemigos insertarDespues sobre la cola", PruebaP03());
	reportar("P04", "ListaBalas eliminar consecutivas con NodoBala**", PruebaP04());
	reportar("P05", "ColaSpawn encolar, desencolar y verFrente en orden", PruebaP05());
	reportar("P06", "ColaSpawn copia profunda con constructor de copia y operator=", PruebaP06());
	return 0;
}