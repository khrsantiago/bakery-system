#include "SistemaPanaderia.hpp"

Designar::nat_t hashSKU(const std::string& sku)
{
    Designar::nat_t hash = 14695981039346656037ULL; // Offset basis FNV-1a 64-bit
    for (unsigned char c : sku)
    {
        hash ^= static_cast<Designar::nat_t>(c);
        hash *= 1099511628211ULL; // FNV prime 64-bit
    }
    return hash;
}

SistemaPanaderia::SistemaPanaderia()
    : catalogoRecetas(std::equal_to<std::string>(), &hashSKU),
      catalogoProductos(std::equal_to<std::string>(), &hashSKU)
{
}

SistemaPanaderia::~SistemaPanaderia() = default;

bool SistemaPanaderia::registrarProducto(ProductoPanaderia* producto)
{
    if (producto == nullptr || producto->getSKU().empty())
    {
        return false;
    }

    if (catalogoProductos.has(producto->getSKU()))
    {
        return false;
    }

    return catalogoProductos.insert(producto->getSKU(), producto) != nullptr;
}

bool SistemaPanaderia::registrarReceta(Receta* receta)
{
    if (receta == nullptr || receta->getSKU().empty())
    {
        return false;
    }

    if (catalogoRecetas.has(receta->getSKU()))
    {
        return false;
    }

    return catalogoRecetas.insert(receta->getSKU(), receta) != nullptr;
}

ProductoPanaderia* SistemaPanaderia::buscarProducto(const std::string& sku)
{
    ProductoPanaderia** ptrProducto = catalogoProductos.search(sku);
    if (ptrProducto == nullptr)
    {
        return nullptr;
    }
    return *ptrProducto;
}

Receta* SistemaPanaderia::buscarReceta(const std::string& sku)
{
    Receta** ptrReceta = catalogoRecetas.search(sku);
    if (ptrReceta == nullptr)
    {
        return nullptr;
    }
    return *ptrReceta;
}

bool SistemaPanaderia::eliminarProducto(const std::string& sku)
{
    return catalogoProductos.remove(sku);
}

bool SistemaPanaderia::eliminarReceta(const std::string& sku)
{
    return catalogoRecetas.remove(sku);
}

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
    ProductoPanaderia* prod = buscarProducto(sku);
    if (prod != nullptr) {
        std::cout << " Producto             : " << prod->getNombre()
                  << " (Rendimiento: " << (prod->getRendimientoMasa() * 100.0) << "%)\n";
    }
    std::cout << " SKU del Producto     : " << sku << "\n";
    std::cout << " Tamano del Lote      : " << cantidadSolicitadaKg << " kg\n";
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