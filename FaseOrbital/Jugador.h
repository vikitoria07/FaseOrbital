#pragma once

#include <string>
#include <cmath> 

enum Color {
    CIAN = 0,
    MAGENTA = 1,
    AMBAR = 2
};

class Jugador {
private:
    std::string nombre;
    int identificador;
    float posicion_angular;
    Color color_actual;
    float carga_pulse;
    float puntaje;
    int combo_actual;
    int combo_maximo;
    bool eliminado;
    int vida_nucleo;

    static float normalizarAngulo(float angulo) {
        while (angulo < 0.0f) {
            angulo += 360.0f;
        }

        while (angulo >= 360.0f) {
            angulo -= 360.0f;
        }

        return angulo;
    }

public:
    Jugador()
        : nombre("nombreJugador"),
        identificador(0),
        posicion_angular(270.0f),
        color_actual(MAGENTA),
        carga_pulse(0.0f),
        puntaje(0.0f),
        combo_actual(0),
        combo_maximo(0),
        eliminado(false),
        vida_nucleo(100) {
    }

    int obtenerIdentificador() const {
        return identificador;
    }

    const std::string& obtenerNombre() const {
        return nombre;
    }

    float obtenerPosicionAngular() const {
        return posicion_angular;
    }

    int obtenerVidaNucleo() const {
        return vida_nucleo;
    }

    Color obtenerColorActual() const {
        return color_actual;
    }

    float obtenerCargaPulse() const {
        return carga_pulse;
    }

    float obtenerPuntaje() const {
        return puntaje;
    }

    int obtenerComboActual() const {
        return combo_actual;
    }

    int obtenerComboMaximo() const {
        return combo_maximo;
    }

    bool estaEliminado() const {
        return eliminado;
    }

    void ponerNombre(const std::string& nuevo_nombre) {
        nombre = nuevo_nombre;
    }

    void ponerIdentificador(int nuevo_identificador) {
        identificador = nuevo_identificador;
    }

    void cambiarPosicionAngular(float nueva_posicion) {
        posicion_angular = normalizarAngulo(nueva_posicion);
    }

    void cambiarColorActual(Color nuevo_color) {
        color_actual = nuevo_color;
    }

    void aumentarPulso(float incremento) {
        carga_pulse += incremento;

        if (carga_pulse > 100.0f) {
            carga_pulse = 100.0f;
        }
    }

    void resetearPulso() {
        carga_pulse = 0.0f;
    }

    void aumentarPuntaje(float puntos) {
        puntaje += puntos;
    }

    void incrementarCombo() {
        ++combo_actual;

        if (combo_actual > combo_maximo) {
            combo_maximo = combo_actual;
        }
    }

    void resetearCombo() {
        combo_actual = 0;
    }

    void reiniciarJugador() {
        posicion_angular = 270.0f;
        color_actual = MAGENTA;
        carga_pulse = 0.0f;
        puntaje = 0.0f;
        combo_actual = 0;
        combo_maximo = 0;
        eliminado = false;
        vida_nucleo = 100;
    }

    void cambiarComboMaximo(int nuevo_combo_maximo) {
        combo_maximo = nuevo_combo_maximo;
    }

    void eliminarJugador() {
        eliminado = true;
    }

    void girarHacia(float angulo) {
        angulo = normalizarAngulo(angulo);

        float diferencia = angulo - posicion_angular;

        if (diferencia > 180.0f) {
            diferencia -= 360.0f;
        }
        else if (diferencia < -180.0f) {
            diferencia += 360.0f;
        }

        if (std::fabs(diferencia) <= 6.0f) {
            posicion_angular = angulo;
        }
        else if (diferencia > 0.0f) {
            posicion_angular += 6.0f;
        }
        else {
            posicion_angular -= 6.0f;
        }

        posicion_angular = normalizarAngulo(posicion_angular);
    }

    void recibirDanno(int danno) {
        vida_nucleo -= danno;

        if (vida_nucleo < 0) {
            vida_nucleo = 0;
        }
    }

    void reiniciarNucleo() {
        vida_nucleo = 100;
    }

    void repararNucleo(int cantidad) {
        vida_nucleo += cantidad;

        if (vida_nucleo > 100) {
            vida_nucleo = 100;
        }
    }
};


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

Jugador crearJugador(std::string nombre, bool Bot) {
    Jugador jug;
    jug.ponerNombre(nombre);
    jug.aumentarPuntaje(100);
    jug.cambiarComboMaximo(100);
    jug.cambiarColorActual(MAGENTA);
    jug.aumentarPulso(100);
    return jug;
}

