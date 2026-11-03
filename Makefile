# ==========================================
# Makefile genérico para proyectos en C++
# ==========================================

# Compilador y banderas
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -O2
LDFLAGS   := 

# Directorios
SRCDIR    := src
INCDIR    := include
BUILDDIR  := build
BINDIR    := .
TARGET    := programa

# Archivos fuente y objetos
SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCS))

# Regla principal por defecto
all: $(BUILDDIR) $(TARGET)

# Crear el ejecutable
$(TARGET): $(OBJS)
	@echo "Enlazando: $@"
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compilar archivos .cpp en archivos objeto .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	@echo "Compilando: $<"
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Crear carpeta build si no existe
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos..."
	@rm -rf $(BUILDDIR) $(TARGET)

# Marcar objetivos que no son archivos reales
.PHONY: all clean
