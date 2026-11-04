# FaseOrbital

## 📋 Tabla de Contenidos
1. [Descripción General](#descripción-general)
2. [Arquitectura del Proyecto](#arquitectura-del-proyecto)
3. [Módulos y Componentes](#módulos-y-componentes)
4. [Estructuras de Datos](#estructuras-de-datos)
5. [Flujo de Juego](#flujo-de-juego)
6. [Sistemas Principales](#sistemas-principales)
7. [Configuración y Compilación](#configuración-y-compilación)
8. [Dependencias Externas](#dependencias-externas)

---

## Descripción General

**Fase Orbital** es un videojuego de estrategia y acción desarrollado en C++ utilizando la librería gráfica Allegro 5. El juego presenta un sistema de combate en tiempo real con múltiples jugadores que controlan naves ubicadas en órbitas circulares alrededor de un núcleo central.

### Características Principales
- **Sistema de múltiples jugadores**: Hasta 3 jugadores (CÍAN, MAGENTA, ÁMBAR)
- **Mecánica orbital**: Los jugadores se encuentran posicionados en órbitas alrededor de un núcleo central
- **Sistema de defensa con colores**: Mecánica de coincidencia de colores (color matching)
- **Oleadas de enemigos**: Progresión por oleadas con enemigos de diferentes tipos
- **Poder especial "Pulso"**: Habilidad especial que daña enemigos en un radio específico
- **Estadísticas y ranking**: Sistema de puntuación y tracking de estadísticas

---

## Arquitectura del Proyecto

### Patrón de Diseño
El proyecto divide la lógica en tres capas principales:

```
┌─────────────────────────────────┐
│     VISTA (VistaJuego)          │
│  (Lectura y Renderización)      │
└────────────┬────────────────────┘
			 │ (Lee estado)
┌────────────▼────────────────────┐
│     MOTOR (MotorJuego)          │
│  (Lógica de Juego)              │
└────────────┬────────────────────┘
			 │ (Modifica estado)
┌────────────▼────────────────────┐
│  DATOS (Jugador, Enemigos, etc.)│
│  (Estado del Juego)             │
└─────────────────────────────────┘
```

### Principio de Separación
- **Vista**: Solo Lee el estado del motor, no aplica reglas
- **Motor**: Aplica lógica de juego y modifica el estado
- **Datos**: Estructuras que mantienen el estado del juego

---

## Módulos y Componentes

### 1. **MotorJuego.h**
**Responsabilidad**: Lógica de juego principal

**Funciones Principales**:
- `ejecutarronda()`: Ejecuta una ronda de juego con los jugadores
- `generarOleada()`: Genera una nueva oleada de enemigos basada en el número de oleada
- Control de turnos y progresión de rondas

**Características**:
- Manejo de dirección de movimiento (horario/antihorario) según si la ronda es par o impar
- Generación procedural de enemigos con dificultad progresiva
- Sistema de oleadas dinámicas

### 2. **Jugador.h**
**Responsabilidad**: Def inición y gestión del estado del jugador

**Estructura Jugador**:
```cpp
class Jugador {
	std::string nombre;
	int identificador;
	float posicion_angular;      // Ángulo en la órbita (0-360°)
	Color color_actual;          // Cián, Magenta, Ámbar
	float carga_pulse;           // Carga del poder especial (0-100)
	float puntaje;
	int combo_actual;            // Combo actual en juego
	int combo_maximo;            // Máximo combo alcanzado
	bool eliminado;
	int vida_nucleo;             // Cantidad de golpes restantes
};
```

**Colores Disponibles**:
- `CIAN = 0`
- `MAGENTA = 1`
- `AMBAR = 2`

**Métodos**:
- Normalización de ángulos
- Manipulación de estado del jugador (vida, carga, combo, etc.)

### 3. **Enemigos.h**
**Responsabilidad**: Gestión de enemigos mediante lista doblemente enlazada

**Tipos de Enemigos**:
1. **Dron**: Enemigo básico
2. **Girador**: Enemigo que rota
3. **Divisor**: Enemigo que se divide
4. **Mini**: Versión pequeña de otros enemigos
5. **Tanque**: Enemigo resistente
6. **Camaleón**: Causa cambios de color

**Estructura Enemigo**:
```cpp
struct Enemigo {
	int id;                    // Identificador único
	TipoEnemigo tipo;         // Tipo del enemigo
	int color;                // Color (0, 1, 2)
	double r;                 // Distancia radial al núcleo
	double theta;             // Ángulo angular (posición en órbita)
	double omega;             // Velocidad angular
	int vida;                 // Golpes restantes
	int radio;                // Radio de colisión/movimiento
	int ticksColor;           // Contador para camaleones
	int sentido;              // 0=horario, 1=antihorario (para giradores)
};
```

**Lista Doblemente Enlazada**:
- **NodoEnemigo**: Contiene enemigo + punteros anterior/siguiente
- **ListaEnemigos**: Contenedor con cabeza, cola y tamaño
- Operaciones: inserción, eliminación, recorrido bidireccional

### 4. **Balas.h**
**Responsabilidad**: Gestión de proyectiles disparados por jugadores

**Estructura Bala**:
```cpp
struct Bala {
	double x, y;      // Posición cartesiana
	double vx, vy;    // Velocidad en cada eje
	int color;        // Color del disparo (debe coincidir con enemigo)
	int duenio;       // Identificador del jugador propietario
	bool marcada;     // Marcada para eliminación
};
```

**Lista de Balas**:
- **NodoBala**: Nodo de lista simple enlazada
- **ListaBalas**: Lista simple (balas se agregan al inicio)
- Operaciones: inserción rápida, eliminación, recorrido

### 5. **Pulso.h**
**Responsabilidad**: Mecánica del poder especial "Pulso"

**Estructura Pulso**:
```cpp
struct Pulso {
	bool activo;           // Estado activo/inactivo
	double radio_actual;   // Radio de expansión actual
};
```

**Funciones**:
- `activarPulso()`: Activa un pulso si tiene carga >= 100
- `avanzarPulso()`: Expande el pulso y detecta enemigos en el radio

**Mecánica**:
- Requiere 100 de carga de pulso para activar
- Se expande a razón de 12 unidades por tick
- Daña enemigos que estén en el rango de radio

### 6. **Generador.h**
**Responsabilidad**: Generación de números pseudoaleatorios

**Clase GeneradorPseudoaleatorio**:
- Algoritmo LCG (Linear Congruential Generator)
- Fórmula: `(1103515245 * x + 12345) & 0x7FFFFFFF`
- Determinista (mismo resultado con misma semilla)

**Uso**:
- Generación procedural de oleadas
- Garantiza reproducibilidad de juegos

### 7. **VistaJuego.h**
**Responsabilidad**: Renderización del juego

**Función Principal**:
- `dibujarMundo()`: Dibuja el estado actual del motor

**Principio**:
- Solo lee estado, no lo modifica
- Usa datos del motor para renderizar

### 8. **Acciones.h**
**Responsabilidad**: Interpretación de acciones del jugador

**Contenido**:
- Mapeo de inputs a acciones
- Procesamiento de comandos del jugador

### 9. **Estadísticas&Ranking.h**
**Responsabilidad**: Tracking de estadísticas y ranking

**Funciones**:
- Registro de puntuaciones
- Cálculo de estadísticas del jugador
- Gestión de rankings

### 10. **Bitacora.h**
**Responsabilidad**: Logging y debugging

**Funciones**:
- Registro de eventos del juego
- Trazabilidad de ejecución

### 11. **Helpers.h**
**Responsabilidad**: Funciones auxiliares y utilidades

**Contenido**:
- Funciones matemáticas comunes
- Utilidades generales del proyecto

### 12. **Scripts.h**
**Responsabilidad**: Secuencias scripted y eventos especiales

**Contenido**:
- Comportamientos programados
- Eventos especiales del juego

### 13. **Pruebas.h**
**Responsabilidad**: Suite de pruebas unitarias

**Contenido**:
- Validación de componentes
- Testing de funcionalidades

---

## Estructuras de Datos

### Lista Doblemente Enlazada (Enemigos)
```
	 ┌─────────────┐
	 │  NodoEnemigo │
	 │  ┌─────────┐ │
   ┌─┤  ant      │ │
   │ │ Enemigo   │ │
   │ │ sig    ┌──│─┐
   │ └─────────┘ │ │
   │             │ │
   ▼             ▼ ▼
┌─────────┐  ┌─────────┐
│  Nodo1  │──│  Nodo2  │
└─────────┘  └─────────┘
```

**Ventajas**:
- Inserción y eliminación O(1)
- Recorrido bidireccional
- Gestión dinámica de memoria

### Lista Simple Enlazada (Balas)
```
┌────────┐     ┌────────┐     ┌────────┐
│ Nodo1  ├────→│ Nodo2  ├────→│ Nodo3  ├────→ NULL
└────────┘     └────────┘     └────────┘
   ↑
cabeza
```

**Ventajas**:
- Inserción al inicio O(1)
- Menor uso de memoria que doblemente enlazada

---

## Flujo de Juego

### Ciclo Principal del Juego
```
1. Inicialización
   ├─ Crear jugadores
   ├─ Inicializar oleadas
   └─ Configurar vista

2. Bucle de Juego
   ├─ Ejecutar comunicación Ronda
   │  ├─ Turno Jugador 1
   │  ├─ Turno Jugador 2
   │  └─ Turno Jugador 3
   │
   ├─ Procesar Aceiones de entrada
   │  ├─ Leer controles
   │  └─ Aplicar acciones
   │
   ├─ Actualizar Lógica de Juego
   │  ├─ Mover enemigos
   │  ├─ Mover balas
   │  ├─ Detectar colisiones
   │  ├─ Actualizar pulso
   │  └─ Actualizar estado jugadores
   │
   ├─ Renderizar
   │  ├─ Dibujar órbita
   │  ├─ Dibujar núcleo
   │  ├─ Dibujar jugadores
   │  ├─ Dibujar enemigos
   │  ├─ Dibujar balas
   │  └─ Dibujar UI
   │
   └─ Condición de Término
	  ├─ Todos jugadores eliminados → Game Over
	  ├─ Última oleada completada → Win
	  └─ Evento especial → Acción

3. Finalización
   ├─ Guardar estadísticas
   ├─ Mostrar ranking
   └─ Limpiar memoria
```

### Ronda de Juego
**Dirección**:
- Ronda IMPAR → Sentido HORARIO
- Ronda PAR → Sentido ANTIHORARIO

**Tabla de Oleadas**:
```
Oleada 1: Drones básicos
Oleada 2: Drones + Giradores
Oleada 3: Drones + Giradores + Divisores
Oleada 4: Drones + Giradores + Ampliados
...
```

---

## Sistemas Principales

### 1. Sistema de Posicionamiento Orbital

**Parámetros**:
```cpp
constexpr float CENTRO_X = 400.0f;      // Centro X de la pantalla
constexpr float CENTRO_Y = 300.0f;      // Centro Y de la pantalla
constexpr float RADIO_ORBITA = 100.0f;  // Radio de la órbita
constexpr float RADIO_NUCLEO = 20.0f;   // Radio del núcleo central
constexpr float TAMANO_NAVE = 6.0f;     // Tamaño de la nave del jugador
```

**Cálculo de Posición**:
```
x = CENTRO_X + RADIO_ORBITA * cos(posicion_angular)
y = CENTRO_Y + RADIO_ORBITA * sin(posicion_angular)
```

### 2. Sistema de Coincidencia de Colores

**Mecánica**:
- Los enemigos tienen un color asignado
- Las balas solo dañan enemigos del mismo color
- Los jugadores pueden cambiar de color durante la partida

**Efecto de Daño**:
- Bala ROJA + Enemigo ROJO = Daño
- Bala ROJA + Enemigo AZUL = Sin daño (puede ser ignorado)

### 3. Sistema de Pulso (Poder Especial)

**Activación**:
1. Jugador acumula 100 de carga
2. Player activa pulso (tecla específica)
3. Pulso se expande desde el área del jugador

**Expansión**:
- Radio inicial: 20 unidades
- Velocidad: +12 unidades por tick
- Crecimiento continuo hasta destruir todos los enemigos en rango

**Efecto**:
- Daña enemigos dentro del radio
- Ignora coincidencia de colores
- Consume toda la carga del pulso

### 4. Sistema de Combo

**Incremento**:
- +1 combo por enemigo derrotado sin perder vida del núcleo
- Se reinicia al recibir daño

**Beneficios**:
- Multiplicador de puntos
- Bonus de carga de pulso

### 5. Sistema de Puntuación

**Cálculo**:
```
Puntos = (Tipo_enemigo_valor) * (1 + Combo * Multiplicador)
```

**Valores por tipo**:
- Dron: 10 puntos
- Girador: 20 puntos
- Divisor: 30 puntos
- Mini: 5 puntos
- Tanque: 50 puntos
- Camaleón: 25 puntos

---

## Configuración y Compilación

### Archivo de Proyecto
**FaseOrbital.vcxproj**: Proyecto de Visual Studio para C++

**Versión de C++**: C++11 o superior

### Estructura de Directorios
```
FaseOrbital/
│
├── FaseOrbital.cpp              (Archivo principal)
├── FaseOrbital.vcxproj          (Archivo de proyecto)
├── FaseOrbital.vcxproj.filters  (Filtros de VS)
│
├── Headers:
│   ├── MotorJuego.h
│   ├── Jugador.h
│   ├── Enemigos.h
│   ├── Balas.h
│   ├── Pulso.h
│   ├── Generador.h
│   ├── Generador.h
│   ├── Acciones.h
│   ├── Estadísticas&Ranking.h
│   ├── Bitacora.h
│   ├── Helpers.h
│   ├── Scripts.h
│   ├── Pruebas.h
│   └── VistaJuego.h
│
└── packages.config              (Dependencias NuGet)
```

### Compilación

**Requisitos**:
- Visual Studio 2015 o superior
- C++ version: C++11 o posterior

**Pasos**:
1. Abrir FaseOrbital.vcxproj en Visual Studio
2. Instalar dependencias (ver sección Dependencias)
3. Configurar propiedades del proyecto (ver configuración de Allegro)
4. Compilar (Build → Build Solution)
5. Ejecutar

---

## Dependencias Externas

### Librería Gráfica: Allegro 5

**Módulos Utilizados**:
```cpp
#include <allegro5/allegro.h>              // Core
#include <allegro5/allegro_primitives.h>   // Dibujo de formas
#include <allegro5/allegro_font.h>         // Tipografía
#include <allegro5/allegro_ttf.h>          // Fuentes TTF
#include <allegro5/allegro_audio.h>        // Audio
#include <allegro5/allegro_acodec.h>       // Codecs de audio
```

**Características Utilizadas**:
- Renderización gráfica 2D
- Dibujo de formas geométricas (círculos, líneas)
- Gestión de texto y fuentes
- Reproducción de audio
- Manejo de eventos

### Librerías Estándar C++
```cpp
#include <iostream>      // E/S estándar
#include <string>        // Cadenas de caracteres
#include <cmath>         // Funciones matemáticas
#include <fstream>       // E/S de archivos
#include <iomanip>       // Manipuladores de entrada/salida
#include <cstdio>        // E/S tipo C
#include <sstream>       // Streams de cadenas
#include <cstdint>       // Tipos enteros específicos
#include <cstring>       // Funciones de cadena tipo C
```

### Instalación de Dependencias

**Windows (usar NuGet en Visual Studio)**:
```
Package Manager Console:
Install-Package allegro -Version [VERSION]
```

**Alternativamente**:
1. Descargar Allegro 5 desde https://liballeg.org/
2. Compilar o descargar binarios precompilados
3. Configurar rutas en Visual Studio:
   - Include paths
   - Library paths
   - Linkers

---

## Notas Técnicas Importantes

### 1. Gestión de Memoria
- Se utiliza **new/delete** explícitamente en listas enlazadas
- Los destructores de ListaEnemigos y ListaBalas liberan memoria
- **Potencial mejora**: Usar smart pointers (unique_ptr, shared_ptr)

### 2. Normalizacion de Ángulos
```cpp
static float normalizarAngulo(float angulo) {
	while (angulo < 0.0f) {
		angulo += 360.0f;
	}
	while (angulo >= 360.0f) {
		angulo -= 360.0f;
	}
	return angulo;
}
```

### 3. Generación Pseudoaleatoria
- Algoritmo: Linear Congruential Generator (LCG)
- Determinista: Reproducible con misma semilla
- Fórmula: `(1103515245 * x + 12345) & 0x7FFFFFFF`

### 4. Detectores de Colisión
- Distancia euclidiana para círculos
- Comparación de ángulos para órbitas

### 5. Arquitectura MVC (Model-View-Controller)
- **Model**: Jugador, Enemigos, Balas, Estado del juego
- **View**: VistaJuego.h (renderización)
- **Controller**: MotorJuego.h (lógica de negocios)

---

## Patrones de Diseño Utilizados

### 1. **Patrón Template Method**
```cpp
template<class Operacion>
void ejecutarronda(RondaJugadores& jugadores, int rondaactual, Operacion Jugarturno)
```

### 2. **Patrón Iterator (Implícito)**
Recorrido de listas doblemente enlazadas mediante punteros

### 3. **Separación de Responsabilidades**
- Vista: Solo lectura
- Motor: Lógica
- Datos: Estado

---

## Extensiones Futuras Posibles

1. **Multiplayer en Red**: Sincronización de estado entre clientes
2. **Efectos Visuales**: Partículas, transiciones, animaciones
3. **Sistema de Poder-ups**: Bonificadores temporales
4. **Guardar/Cargar Juego**: Persistencia de estado
5. **Editor de Oleadas**: Herramienta para diseñar oleadas customizadas
6. **IA Enemiga Adaptativa**: Patrones dinámicos según rendimiento
7. **Leaderboards Online**: Ranking global

---

## Conclusión

Fase Orbital es un proyecto bien estructurado que implementa un juego de estrategia orbital con múltiples jugadores. Su arquitectura separa claramente la lógica de juego (Motor) de la presentación (Vista), facilitando el mantenimiento y extensión. El uso de estructuras de datos personalizadas (listas enlazadas) demuestra comprensión de algoritmos fundamentales, aunque podría beneficiarse del uso de contenedores STL o smart pointers para modernizar el código.

---

## Glosario de Términos

| Término | Definición |
|---------|-----------|
| **Órbita** | Posición circular de los jugadores alrededor del núcleo |
| **Núcleo** | Centro del mapa, vital para los jugadores |
| **Pulso** | Poder especial que expande en onda desde el jugador |
| **Oleada** | Conjunto de enemigos que aparecen en una ronda |
| **Combo** | Serie consecutiva de enemigos derrotados sin recibir daño |
| **Girador** | Tipo de enemigo que rota alrededor de una órbita |
| **Camaleón** | Enemigo que cambia de color periódicamente |
| **Divisor** | Enemigo que se divide en múltiples enemigos menores |
| **Tanque** | Enemigo resistente con múltiples puntos de vida |

---

**Versión**: 1.0  
**Última actualización**: 2024  
**Autor**: Equipo de Desarrollo Fase Orbital  
**Lenguaje**: C++  
**Engine Gráfico**: Allegro 5
