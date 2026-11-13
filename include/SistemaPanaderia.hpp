#ifndef SISTEMA_PANADERIA_HPP
#define SISTEMA_PANADERIA_HPP

#include <map.hpp>      // Cabecera de mapas de DeSiGNAR
#include "receta.hpp"
#include "LoteProduccion.hpp"
#include "GestionPerecederos.hpp"

/**
 * Función de dispersión (FNV-1a 64-bit) optimizada para códigos SKU alfanuméricos.
 * Se define en el módulo del proyecto para mantener DeSiGNAR en modo header-only.
 */
Designar::nat_t hashSKU(const std::string& sku);

class SistemaPanaderia
{
private:
    // Tabla Hash para catálogo de recetas por SKU (Búsqueda promedio O(1))
    Designar::HashMap<std::string, Receta*> catalogoRecetas;
    // Tabla Hash para catálogo de productos por SKU (Búsqueda promedio O(1))
    Designar::HashMap<std::string, ProductoPanaderia*> catalogoProductos;

public:
    SistemaPanaderia();
    ~SistemaPanaderia();

    // =========================================================================
    // REGISTRO Y GESTIÓN DE CATÁLOGO (Integración POS)
    // =========================================================================
    
    bool registrarProducto(ProductoPanaderia* producto);
    bool registrarReceta(Receta* receta);

    // =========================================================================
    // BÚSQUEDAS EN TIEMPO CONSTANTE O(1)
    // =========================================================================

    ProductoPanaderia* buscarProducto(const std::string& sku);

    Receta* buscarReceta(const std::string& sku);

    bool eliminarProducto(const std::string& sku);

    bool eliminarReceta(const std::string& sku);

    // =========================================================================
    // ORQUESTACIÓN DE PRODUCCIÓN (Conector Hash O(1) -> Árbol DFS O(n))
    // =========================================================================

    bool procesarPedidoProduccion(const std::string& sku, double cantidadSolicitadaKg, Designar::DynArray<ElementoReceta>& insumosConsolidados);

    void generarReporteCocina(const std::string& sku, double cantidadSolicitadaKg);
};

#endif // SISTEMA_PANADERIA_HPP