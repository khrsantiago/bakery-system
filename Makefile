# ==============================================================
# Makefile optimizado para Bakery System con DeSiGNAR
# Compatible con Linux y Windows
# ==============================================================

CXX         ?= g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -O2

# Rutas de Inclusión:
# Las estructuras de DeSiGNAR (DynArray, MTreeNode, SLList, etc.)
# son plantillas (templates) definidas totalmente en sus cabeceras (.hpp).
# Por ende, solo se requiere -IDESIGNAR/include y NO compilar DESIGNAR/src.
INCLUDEPATH := -Iinclude -IDESIGNAR/include

FLAGS       := -g $(CXXFLAGS)

# Directorios del Proyecto
SRCDIR      := src
INCDIR      := include
BUILDDIR    := build
BINDIR      := bin

TARGET      := $(BINDIR)/bakery_system
TEST_TARGET := $(BINDIR)/test_system

# Archivos fuente y de cabecera
INCLUDES       := $(wildcard $(INCDIR)/*.hpp)
SOURCES        := $(wildcard $(SRCDIR)/*.cpp)

# Separación de módulos comunes frente a los puntos de entrada (main y test)
COMMON_SOURCES := $(filter-out $(SRCDIR)/main.cpp $(SRCDIR)/test.cpp, $(SOURCES))
COMMON_OBJECTS := $(COMMON_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
MAIN_OBJECT    := $(BUILDDIR)/main.o
TEST_OBJECT    := $(BUILDDIR)/test.o

# Detección de Sistema Operativo (Windows vs Linux/macOS)
ifeq ($(OS),Windows_NT)
    MKDIR_BUILD = if not exist $(subst /,\,$(BUILDDIR)) mkdir $(subst /,\,$(BUILDDIR))
    MKDIR_BIN   = if not exist $(subst /,\,$(BINDIR)) mkdir $(subst /,\,$(BINDIR))
    CLEAN_CMD   = rmdir /S /Q $(subst /,\,$(subst /,\,$(BUILDDIR))) $(subst /,\,$(BINDIR)) 2>nul || exit 0
    EXEC        = $(subst /,\,$(TARGET))
    EXEC_TEST   = $(subst /,\,$(TEST_TARGET))
else
    MKDIR_BUILD = mkdir -p $(BUILDDIR)
    MKDIR_BIN   = mkdir -p $(BINDIR)
    CLEAN_CMD   = rm -rf $(BUILDDIR) $(BINDIR)
    EXEC        = ./$(TARGET)
    EXEC_TEST   = ./$(TEST_TARGET)
endif

all: $(TARGET)

# Compilación de los archivos .cpp
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDES)
	@$(MKDIR_BUILD)
	@echo "Compilando: $<"
	$(CXX) $(FLAGS) $(INCLUDEPATH) -c $< -o $@

# Enlace del ejecutable principal (CLI Interactivo)
$(TARGET): $(MAIN_OBJECT) $(COMMON_OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable principal: $@"
	$(CXX) $(FLAGS) $(MAIN_OBJECT) $(COMMON_OBJECTS) -o $@

# Enlace del ejecutable de pruebas automatizadas
$(TEST_TARGET): $(TEST_OBJECT) $(COMMON_OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable de pruebas: $@"
	$(CXX) $(FLAGS) $(TEST_OBJECT) $(COMMON_OBJECTS) -o $@

# Ejecutar el sistema principal interactivo
run: $(TARGET)
	$(EXEC)

# Compilar y ejecutar las pruebas secuenciales
test: $(TEST_TARGET)
	$(EXEC_TEST)

clean:
	@echo "Limpiando archivos de compilacion..."
	@$(CLEAN_CMD)

.PHONY: all clean run test
