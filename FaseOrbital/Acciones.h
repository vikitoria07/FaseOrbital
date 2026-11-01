#pragma once

// ============================================================
// ACCIONES
// ============================================================

enum class TipoAccion {
	NONE,
	GIRAR_A,
	CAMBIAR_COLOR,
	ACTIVAR_PULSO
};

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
