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

# Archivos fuente y objetos de TU proyecto únicamente (Ignora DESIGNAR/src)
INCLUDES    := $(wildcard $(INCDIR)/*.hpp)
SOURCES     := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS     := $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Detección de Sistema Operativo (Windows vs Linux/macOS)
ifeq ($(OS),Windows_NT)
    MKDIR_BUILD = if not exist $(subst /,\,$(BUILDDIR)) mkdir $(subst /,\,$(BUILDDIR))
    MKDIR_BIN   = if not exist $(subst /,\,$(BINDIR)) mkdir $(subst /,\,$(BINDIR))
    CLEAN_CMD   = rmdir /S /Q $(subst /,\,$(subst /,\,$(BUILDDIR))) $(subst /,\,$(BINDIR)) 2>nul || exit 0
    EXEC        = $(subst /,\,$(TARGET))
else
    MKDIR_BUILD = mkdir -p $(BUILDDIR)
    MKDIR_BIN   = mkdir -p $(BINDIR)
    CLEAN_CMD   = rm -rf $(BUILDDIR) $(BINDIR)
    EXEC        = ./$(TARGET)
endif

all: $(TARGET)

# Compilación de los archivos .cpp de TU proyecto únicamente
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDES)
	@$(MKDIR_BUILD)
	@echo "Compilando: $<"
	$(CXX) $(FLAGS) $(INCLUDEPATH) -c $< -o $@

# Enlace del ejecutable final
$(TARGET): $(OBJECTS)
	@$(MKDIR_BIN)
	@echo "Enlazando ejecutable: $@"
	$(CXX) $(FLAGS) $(OBJECTS) -o $@

run: all
	$(EXEC)

clean:
	@echo "Limpiando archivos de compilacion..."
	@$(CLEAN_CMD)

.PHONY: all clean run
