#include <iostream>
using namespace std;

#include <iostream>
#include "Generador.h"
using namespace std;

// En esta clase se define un generador de números pseudoaleatorios utilizando el método del Congruencial Lineal (LCG).
class GeneradorPseudoaleatorio {
private:
	// estado actual del generador, la inicial será la semilla (seed) proporcionada al constructor.
	long long estado_actual; // Guarda el valor actual de la secuencia

	// valores de la fórmula del LCG: X_{k+1} = (a * X_k + c) % m
	const long long multiplicador = 1103515245;
	const long long incremento = 12345;
	const long long modulo = 1ULL << 31; // 2^31

public:
	// Constructor: Recibe la semilla inicial (seed)
	GeneradorPseudoaleatorio(int seed) : estado_actual(seed) {}

	// Genera y devuelve el siguiente número pseudoaleatorio
	int siguiente() {
		// Fórmula X_{k+1} = (a * X_k + c) % m
		estado_actual = (multiplicador * estado_actual + incremento) % modulo;
		return estado_actual;
	}
};