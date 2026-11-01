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


// La siguiente función va a ejecutar una ronda con un jugador a la vez. Como antes mencionado,
// la ronda es recorrida en sentido horario si es impar y sentido antihorario si es par.
// Por último, el jugador inicia la ronda en donde quedo actual.
template<class Operacion>
void ejecutarronda(RondaJugadores& jugadores, int rondaactual, Operacion Jugarturno) {
	bool direccionhorario = rondaimpar(rondaactual);
	int totalturnos = jugadores.tamano();
	for (int posicion = 0; posicion < totalturnos; posicion++) {
		Jugarturno(jugadores.verActual());
		if (posicion < totalturnos - 1) {
			jugadores.avanzar(direccionhorario);
		}
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

		int intervaloReal = (intervaloBase * (60 + numeroIntervalo % 81)) / 100;

		// Desde la oleada 7 puede haber aparicion  doble

		if (oleada >= 7 && numeroIntervalo % 4 == 0) {
			intervaloReal = 0;
		}

		tickActual += intervaloReal;
	}

	return nuevaOleada;
}

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

					if (balaActual->dato.color == enemigoActual->dato.color) {
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
	void eliminarJugador() {
		Jugador* jugadorRemovido = ronda.verActual();
		if (jugadorRemovido == nullptr) {
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
		else if (ronda.tamano() == 1) {
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