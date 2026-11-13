# Bakery System — Sistema de Gestión de Producción y Recetas Panaderas

Proyecto académico de **Programación III** — **Universidad de Los Andes (ULA)**.

**Autores:**
- **Luis Acevedo**
- **Khristopher Santiago**
- **Juan Rincón**

---

## Descripción General

**Bakery System** es un sistema en **C++17** diseñado para gestionar las operaciones de una panadería artesanal e industrial, integrando estructuras de datos clásicas de la biblioteca **DeSiGNAR** (ULA) con dos interfaces de usuario: una **interfaz de consola (CLI)** y una **interfaz gráfica de escritorio multiplataforma (GUI)** construida con **Dear ImGui**.

### Funcionalidades Principales
- **Punto de Venta (POS) y Catálogo en $O(1)$:** Búsqueda, registro y eliminación instantánea de productos y recetas mediante su código `SKU` usando una Tabla Hash.
- **Explosión Recursiva de Recetas (Árbol N-Ario):** Desglose jerárquico de productos con sub-recetas anidadas (masa base, prefermentos, masa madre, rellenos, coberturas) ajustado automáticamente por factor de lote y porcentaje de merma.
- **Trazabilidad de Lotes de Producción:** Registro cronológico enlazado de cada etapa del proceso panadero (amasado, fermentación, horneado, enfriamiento).
- **Control de Insumos Perecederos (FEFO):** Cola de prioridad (*Min-Heap*) para despachar siempre primero la materia prima con fecha de vencimiento más próxima (*First Expired, First Out*).

---

## Estructuras de Datos Implementadas (`DeSiGNAR`)

| Módulo | Archivo(s) | Estructura `DeSiGNAR` | Complejidad | Propósito |
| :--- | :--- | :--- | :--- | :--- |
| **POS y Catálogo SKU** | `SistemaPanaderia.hpp` / `.cpp` | `Designar::HashMap` (`SeparateChainingHashTable`) + hash `hashSKU` (FNV-1a 64 bits) | $O(1)$ promedio | Indexación y búsqueda rápida de `ProductoPanaderia*` y `Receta*` por `codigoSKU`. |
| **Árbol de Recetas y DFS** | `receta.hpp` / `.cpp` | `Designar::MTreeNode<ElementoReceta>` | $O(n)$ por receta | Modelado multinivel de ingredientes/sub-recetas y explosión recursiva (`calcularMateriasPrimas`). |
| **Trazabilidad de Lotes** | `LoteProduccion.hpp` / `.cpp` | `Designar::SLList<EventoTrazabilidad>` | $O(1)$ inserción | Bitácora cronológica de eventos de producción por lote. |
| **Insumos Perecederos (FEFO)** | `GestionPerecederos.hpp` | `Designar::DynHeap` (Min-Heap) | $O(1)$ consulta / $O(\log n)$ extracción | Priorización automática de insumos próximos a vencer. |

---

## Modos de Ejecución (`Makefile`)

El proyecto dispone de tres ejecutables independientes compilables desde el mismo `Makefile`:

| Comando | Ejecutable generado | Descripción | ¿Requiere librerías gráficas? |
| :--- | :--- | :--- | :--- |
| `make run` | `bin/bakery_system` | **Interfaz de Consola (CLI):** Menú interactivo de terminal para POS, pedidos de producción y gestión de catálogo. | **No** (solo `g++` y `make`) |
| `make gui` | `bin/bakery_gui` | **Interfaz Gráfica de Escritorio (GUI):** Ventana nativa con **Dear ImGui** (4 pestañas: POS $O(1)$, Catálogo Hash, Cocina/Árbol DFS y Trazabilidad + FEFO). | **Linux/WSL/macOS:** SDL2 + OpenGL<br>**Windows nativo:** Ninguna (usa Win32 nativo) |
| `make test` | `bin/test_system` | **Pruebas Automatizadas:** Suite unitaria que valida todas las estructuras y flujos con aserciones (`assert`). | **No** (solo `g++` y `make`) |
| `make clean` | — | Elimina los archivos compilados en `build/` y `bin/`. | — |

---

## Requisitos e Instrucciones por Sistema Operativo

Clona el repositorio y entra en la carpeta del proyecto antes de ejecutar cualquiera de los pasos:

```bash
git clone <URL_DEL_REPOSITORIO>
cd bakery-system
```

---

### 1. Linux (Ubuntu / Debian / Linux Mint / Fedora)

- **Para consola (`make run`) y pruebas (`make test`):** solo necesitas el compilador `g++` (C++17) y `make`.
- **Para la interfaz gráfica (`make gui`):** necesitas los encabezados de desarrollo de **SDL2** y **OpenGL**.

**Ubuntu / Debian / Mint:**
```bash
sudo apt update
sudo apt install -y build-essential libsdl2-dev libgl1-mesa-dev
```

**Fedora:**
```bash
sudo dnf install -y gcc-c++ make SDL2-devel mesa-libGL-devel
```

**Ejecución:**
```bash
make run    # Interfaz CLI en terminal
make gui    # Interfaz gráfica de escritorio (Dear ImGui)
make test   # Suite de pruebas automatizadas
```

---

### 2. Windows con WSL / WSL2 (Windows Subsystem for Linux)

> **Importante para usuarios de WSL:**
> Aunque WSL se ejecute dentro de Windows, **para el compilador y el `Makefile`, WSL es un entorno Linux completo** (`uname -s` devuelve `Linux`, no `Windows_NT`). Por lo tanto, dentro de la terminal de WSL el proyecto utiliza el flujo de Linux (no el de Windows nativo).

1. **Si solo vas a usar la consola (`make run`) o correr pruebas (`make test`):**
   Solo necesitas tener compilador C++ y `make` instalados en tu distribución WSL (ej. Ubuntu):
   ```bash
   sudo apt update
   sudo apt install -y build-essential
   make run
   make test
   ```
   *(No necesitas instalar ninguna librería gráfica para CLI ni tests).*

2. **Si deseas abrir la interfaz gráfica (`make gui`) desde WSL:**
   Como WSL compila con la configuración de Linux, debes instalar `libsdl2-dev` dentro de WSL una sola vez:
   ```bash
   sudo apt update
   sudo apt install -y build-essential libsdl2-dev libgl1-mesa-dev
   make gui
   ```
   - **Windows 11 (y Windows 10 actualizado con WSL2):** Gracias a **WSLg** (incluido por defecto en WSL2 moderno), la ventana gráfica de Linux se abrirá directamente en tu escritorio de Windows sin configurar nada extra.
   - **Si aparece el error `Could not initialize SDL: No available video device`:** Significa que tu versión de WSL no tiene WSLg habilitado. Puedes actualizar WSL ejecutando `wsl --update` en PowerShell como administrador, compilar de forma nativa en Windows (ver sección 3), o usar `make run` para operar desde la interfaz de consola.

---

### 3. Windows Nativo (PowerShell / CMD con MinGW-w64 o MSYS2 — sin WSL)

En Windows nativo (`OS=Windows_NT`), el `Makefile` detecta el sistema automáticamente y compila `make gui` usando el backend **Win32 + OpenGL (`imgui_impl_win32`)** ya incluido en la carpeta `IMGUI/` del repositorio.

- **Ventaja:** **Cero instalaciones adicionales**. No necesitas descargar SDL2, DLLs ni paquetes externos; únicamente necesitas un compilador `g++` (MinGW-w64 / MSYS2) y `make` en tu `PATH`.
- **Requisitos:** MinGW-w64 (por ejemplo vía [MSYS2](https://www.msys2.org/) instalando `mingw-w64-x86_64-gcc` y `make`, o WinLibs).

**Ejecución desde PowerShell, CMD o terminal MSYS2:**
```powershell
make run    # Interfaz CLI en consola
make gui    # Interfaz gráfica nativa Win32 (sin instalar SDL2)
make test   # Suite de pruebas automatizadas
```

> **Nota:** Si tu distribución de MinGW trae el ejecutable con el nombre `mingw32-make` en lugar de `make`, ejecuta `mingw32-make run`, `mingw32-make gui` o `mingw32-make test`.

---

### 4. macOS

**Requisitos:**
1. Herramientas de línea de comandos de Xcode (`clang++`/`g++` y `make`):
   ```bash
   xcode-select --install
   ```
2. Para la **interfaz gráfica (`make gui`)**, instala **SDL2** mediante [Homebrew](https://brew.sh/):
   ```bash
   brew install sdl2
   ```
   *(Para `make run` y `make test` no es necesario instalar SDL2).*

**Ejecución:**
```bash
make run    # Interfaz CLI en terminal
make gui    # Interfaz gráfica de escritorio (Dear ImGui)
make test   # Suite de pruebas automatizadas
```

---

## Catálogo Precargado de Prueba

Tanto la interfaz CLI (`make run`) como la interfaz gráfica (`make gui`) inician con 3 productos y sus árboles de recetas multinivel precargados para pruebas inmediatas en el POS y en Cocina:

| SKU | Producto | Estructura de Receta |
| :--- | :--- | :--- |
| `PAN-JAM-001` | Pan de Jamón Tradicional | Masa Base Enriquecida + Relleno Tradicional y Barniz |
| `PAN-CAN-001` | Pan Canilla Venezolano | Prefermento (Esponja) + Masa Principal |
| `PAN-CAM-001` | Pan Campesino Masa Madre | Masa Madre Activa (Levain) + Masa Rústica |

---

## Estructura del Proyecto

```text
bakery-system/
├── include/                   # Archivos de cabecera (.hpp)
│   ├── ProductoPanaderia.hpp  # Entidad de producto (SKU, precios, masa, alérgenos)
│   ├── receta.hpp             # Árbol N-ario MTreeNode y explosión DFS de insumos
│   ├── LoteProduccion.hpp     # Lista enlazada SLList de trazabilidad por lote
│   ├── GestionPerecederos.hpp # Min-Heap DynHeap con política FEFO
│   └── SistemaPanaderia.hpp   # Integración POS con Tabla Hash (HashMap + FNV-1a)
├── src/                       # Implementación (.cpp) y puntos de entrada
│   ├── ProductoPanaderia.cpp
│   ├── receta.cpp
│   ├── LoteProduccion.cpp
│   ├── SistemaPanaderia.cpp
│   ├── main.cpp               # Punto de entrada CLI (make run)
│   ├── gui.cpp                # Punto de entrada GUI Dear ImGui (make gui)
│   └── test.cpp               # Suite de pruebas automatizadas (make test)
├── DESIGNAR/                  # Biblioteca de estructuras de datos ULA (header-only)
├── IMGUI/                     # Dear ImGui v1.91.8 + backends OpenGL3 / SDL2 / Win32
├── Makefile                   # Compilación multiplataforma (CLI, GUI y Tests)
├── build/                     # Archivos objeto generados (ignorado por Git)
└── bin/                       # Ejecutables compilados (ignorado por Git)
```
