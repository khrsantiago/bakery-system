#include "SistemaPanaderia.hpp"

bool SistemaPanaderia::procesarPedidoProduccion(const std::string& sku, double cantidadSolicitadaKg, Designar::DynArray<ElementoReceta>& insumosConsolidados)
{
    // 1. Búsqueda en O(1) sobre la Tabla Hash
    Receta** ptrReceta = catalogoRecetas.search(sku);

    if (ptrReceta == nullptr || *ptrReceta == nullptr)
    {
        std::cerr << "[!] Error: No se encontró receta para el SKU: " << sku << "\n";
        return false;
    }

    Receta* recetaElegida = *ptrReceta;

    // 2. Desglose recursivo DFS en O(n) sobre el Árbol General
    recetaElegida->calcularMateriasPrimas(cantidadSolicitadaKg, insumosConsolidados);

    return true;
}

void SistemaPanaderia::generarReporteCocina(const std::string& sku, double cantidadSolicitadaKg)
{
    Designar::DynArray<ElementoReceta> insumosConsolidados;

    std::cout << "\n=======================================================\n";
    std::cout << "           ORDEN DE PRODUCCIÓN PARA COCINA             \n";
    std::cout << "=======================================================\n";

    // 1. Ejecuta la búsqueda O(1) en la Tabla Hash y la explosión DFS O(n)
    if (!procesarPedidoProduccion(sku, cantidadSolicitadaKg, insumosConsolidados)) {
        std::cout << "[!] No se pudo generar el reporte. Verifique el SKU ingresado.\n";
        std::cout << "=======================================================\n\n";
        return;
    }

    // 2. Encabezado del reporte
    std::cout << " SKU del Producto     : " << sku << "\n";
    std::cout << " Tama\313o del Lote     : " << cantidadSolicitadaKg << " kg\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << " MATERIAS PRIMAS CONSOLIDADAS:\n";
    std::cout << "-------------------------------------------------------\n";

    // 3. Recorrido del DynArray resultante para mostrar cada ingrediente
    if (insumosConsolidados.is_empty()) {
        std::cout << " (No se registraron materias primas para esta receta)\n";
    } else {
        for (size_t i = 0; i < insumosConsolidados.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] ";
            insumosConsolidados[i].mostrarInfo();
            std::cout << "\n";
        }
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << " Total de Insumos Domiciliados : " << insumosConsolidados.size() << "\n";
    std::cout << " Estado                        : LISTO PARA PREPARACIÓN\n";
    std::cout << "=======================================================\n\n";
}