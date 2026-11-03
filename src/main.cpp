#include <iostream>
#include <vector>
#include <string>

// Clase simple para demostrar la orientación a objetos
class GestorMensajes {
private:
    std::vector<std::string> mensajes;

public:
    GestorMensajes() {
        mensajes.push_back("¡Hola desde C++!");
        mensajes.push_back("El Makefile ha compilado todo correctamente.");
    }

    void mostrarMensajes() const {
        for (const auto& mensaje : mensajes) {
            std::cout << "[INFO] " << mensaje << "\n";
        }
    }
};

int main() {
    // Configurar salida estándar rápida
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cout << "========================================\n";
    std::cout << "   Proyecto Inicializado con Éxito\n";
    std::cout << "========================================\n";

    GestorMensajes gestor;
    gestor.mostrarMensajes();

    return 0;
}
