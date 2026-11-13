# ==============================================================
# Makefile optimizado para Bakery System (CLI + GUI + Tests)
# Compatible con Linux, macOS y Windows
# ==============================================================

CXX         ?= g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -O2

# Directorios del Proyecto
SRCDIR      := src
INCDIR      := include
IMGUI_DIR   := IMGUI
BUILDDIR    := build
BINDIR      := bin

TARGET      := $(BINDIR)/bakery_system
GUI_TARGET  := $(BINDIR)/bakery_gui
TEST_TARGET := $(BINDIR)/test_system

# Detección de Sistema Operativo (Windows vs macOS vs Linux)
ifeq ($(OS),Windows_NT)
    MKDIR_BUILD = if not exist $(subst /,\,$(BUILDDIR)) mkdir $(subst /,\,$(BUILDDIR))
    MKDIR_BIN   = if not exist $(subst /,\,$(BINDIR)) mkdir $(subst /,\,$(BINDIR))
    CLEAN_CMD   = rmdir /S /Q $(subst /,\,$(BUILDDIR)) $(subst /,\,$(BINDIR)) 2>nul || exit 0
    EXEC        = $(subst /,\,$(TARGET))
    EXEC_GUI    = $(subst /,\,$(GUI_TARGET))
    EXEC_TEST   = $(subst /,\,$(TEST_TARGET))
    # En Windows usa Win32 + WGL OpenGL nativos (cero dependencias externas)
    SDL2_CFLAGS           :=
    GUI_LIBS              := -lopengl32 -lgdi32 -ldwmapi -limm32
    IMGUI_BACKEND_EXCLUDE := $(IMGUI_DIR)/imgui_impl_sdl2.cpp
else
    MKDIR_BUILD = mkdir -p $(BUILDDIR)
    MKDIR_BIN   = mkdir -p $(BINDIR)
    CLEAN_CMD   = rm -rf $(BUILDDIR) $(BINDIR)
    EXEC        = ./$(TARGET)
    EXEC_GUI    = ./$(GUI_TARGET)
    EXEC_TEST   = ./$(TEST_TARGET)
    IMGUI_BACKEND_EXCLUDE := $(IMGUI_DIR)/imgui_impl_win32.cpp
    UNAME_S     := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        SDL2_CFLAGS := $(shell sdl2-config --cflags)
        GUI_LIBS    := $(shell sdl2-config --libs) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        SDL2_CFLAGS := $(shell sdl2-config --cflags)
        GUI_LIBS    := $(shell sdl2-config --libs) -lGL -ldl
    endif
endif

# Rutas de Inclusión:
# - INCLUDEPATH: solo cabeceras propias + DeSiGNAR (CLI y Tests no requieren SDL2/ImGui)
# - GUI_INCLUDEPATH: agrega IMGUI y SDL2 únicamente para la interfaz gráfica
INCLUDEPATH     := -I$(INCDIR) -isystem DESIGNAR/include
GUI_INCLUDEPATH := $(INCLUDEPATH) -isystem $(IMGUI_DIR) $(SDL2_CFLAGS)

FLAGS           := -g $(CXXFLAGS)

# Archivos fuente y de cabecera
INCLUDES       := $(wildcard $(INCDIR)/*.hpp)
SOURCES        := $(wildcard $(SRCDIR)/*.cpp)
IMGUI_SOURCES  := $(filter-out $(IMGUI_BACKEND_EXCLUDE), $(wildcard $(IMGUI_DIR)/*.cpp))

# Separación de módulos comunes frente a los puntos de entrada (main CLI, gui, test)
COMMON_SOURCES := $(filter-out $(SRCDIR)/main.cpp $(SRCDIR)/gui.cpp $(SRCDIR)/test.cpp, $(SOURCES))
COMMON_OBJECTS := $(COMMON_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
IMGUI_OBJECTS  := $(IMGUI_SOURCES:$(IMGUI_DIR)/%.cpp=$(BUILDDIR)/imgui_%.o)
MAIN_OBJECT    := $(BUILDDIR)/main.o
GUI_OBJECT     := $(BUILDDIR)/gui.o
TEST_OBJECT    := $(BUILDDIR)/test.o

all: $(TARGET)

# Compilación del punto de entrada de la Interfaz Gráfica (usa cabeceras SDL2/ImGui)
$(GUI_OBJECT): $(SRCDIR)/gui.cpp $(INCLUDES)
	@$(MKDIR_BUILD)
	@echo "Compilando GUI: $<"
	$(CXX) $(FLAGS) $(GUI_INCLUDEPATH) -c $< -o $@

# Compilación de la biblioteca Dear ImGui + Backends (SDL2 / OpenGL3)
$(BUILDDIR)/imgui_%.o: $(IMGUI_DIR)/%.cpp
	@$(MKDIR_BUILD)
	@echo "Compilando ImGui: $<"
	$(CXX) -g -std=c++17 -O2 -w $(GUI_INCLUDEPATH) -c $< -o $@

# Compilación de los archivos .cpp propios del proyecto (CLI, dominio y pruebas)
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDES)
	@$(MKDIR_BUILD)
	@echo "Compilando: $<"
	$(CXX) $(FLAGS) $(INCLUDEPATH) -c $< -o $@

# Enlace del ejecutable CLI (Interfaz de consola - no requiere SDL2)
$(TARGET): $(MAIN_OBJECT) $(COMMON_OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable CLI: $@"
	$(CXX) $(FLAGS) $(MAIN_OBJECT) $(COMMON_OBJECTS) -o $@

# Enlace del ejecutable GUI (Interfaz gráfica de escritorio - Dear ImGui + SDL2)
$(GUI_TARGET): $(GUI_OBJECT) $(COMMON_OBJECTS) $(IMGUI_OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable GUI: $@"
	$(CXX) $(FLAGS) $(GUI_OBJECT) $(COMMON_OBJECTS) $(IMGUI_OBJECTS) $(GUI_LIBS) -o $@

# Enlace del ejecutable de pruebas automatizadas (Consola)
$(TEST_TARGET): $(TEST_OBJECT) $(COMMON_OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable de pruebas: $@"
	$(CXX) $(FLAGS) $(TEST_OBJECT) $(COMMON_OBJECTS) -o $@

# Ejecutar el sistema en modo CLI
run: $(TARGET)
	$(EXEC)

# Compilar y ejecutar el sistema en modo Interfaz Gráfica (GUI)
gui: $(GUI_TARGET)
	$(EXEC_GUI)

# Compilar y ejecutar las pruebas automatizadas
test: $(TEST_TARGET)
	$(EXEC_TEST)

clean:
	@echo "Limpiando archivos de compilacion..."
	@$(CLEAN_CMD)

.PHONY: all clean run gui test
