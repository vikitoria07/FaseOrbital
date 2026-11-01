#pragma once

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

	int tamano() const {
		return total;
	}
	bool vacio() const {
		return total == 0;
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