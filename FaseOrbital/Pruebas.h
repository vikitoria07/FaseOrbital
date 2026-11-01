#pragma once

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


void reportarPrueba(const char* nombre, bool resultado) {
	if (resultado == true) {
		cout << "´Ronda de prueba" << nombre << ": Correcta" << endl;
	}
	else {
		cout << "Ronda de prueba" << nombre << ": Incorrecta" << endl;
	}

}

// El siguiente bool se utiliza para comprobar que la lista siga su recorrido hacia adelante y hacia atrás.
bool verificarRecorrido(RondaJugadores& r, Jugador* requeridos[], int j) {
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

