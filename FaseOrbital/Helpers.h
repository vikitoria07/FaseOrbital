float normalizarAngulo(float angulo) {
	while (angulo < 0.0f) {
		angulo += 360.0f;
	}

	while (angulo >= 360.0f) {
		angulo -= 360.0f;
	}

	return angulo;
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