#pragma once

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
#include "Pulso.h"
#include "Bitacora.h"
#include "Estadisticas&Ranking.h"
#include "Acciones.h"
#include "MotorJuego.h"

// PRUEBAS

#include "Pruebas.h"

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
