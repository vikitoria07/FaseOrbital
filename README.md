# FaseOrbital

Videojuego desarrollado en C++ utilizando Allegro 5.

## Requisitos

- Windows 10 o Windows 11.
- Visual Studio 2026.
- Carga de trabajo **Desarrollo para el escritorio con C++**.
- MSVC v145 y Windows SDK.
- Git.

> Se necesita Visual Studio porque el proyecto utiliza MSVC y las dependencias de Allegro se administran mediante NuGet.

## Instalación y ejecución

1. Clonar el repositorio desde GitHub:

   ```bash
   git clone URL_DEL_REPOSITORIO
   ```

2. Abrir **Developer PowerShell for Visual Studio**.

3. Entrar en la carpeta raíz del proyecto:

   ```powershell
   cd FaseOrbital
   ```

4. Ejecutar el juego:

   ```powershell
   nmake run
   ```

El Makefile se encarga automáticamente de restaurar las dependencias de NuGet, compilar el proyecto y ejecutar el videojuego.

**Nota:** La primera ejecución requiere conexión a Internet para descargar las dependencias de Allegro.

## Comandos disponibles

| Comando | Descripción |
|---|---|
| `nmake` | Compila el proyecto. |
| `nmake run` | Compila y ejecuta el juego. |
| `nmake clean` | Elimina los archivos generados de compilación. |
| `nmake restore` | Restaura las dependencias de NuGet. |

## Tecnologías utilizadas

- C++20
- Allegro 5.2.11.3
- Visual Studio / MSVC
- NuGet
