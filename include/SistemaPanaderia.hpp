#ifndef SISTEMA_PANADERIA_HPP
#define SISTEMA_PANADERIA_HPP

#include <map.hpp>      // Cabecera de mapas de DeSiGNAR
#include "receta.hpp"
#include "LoteProduccion.hpp"
#include "GestionPerecederos.hpp"
#include "KardexInventario.hpp"
#include <ctime> // Para obtener fecha actual

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
    // Tabla Hash para diccionario SKU -> Nombre descriptivo (materias primas y subproductos)
    Designar::HashMap<std::string, std::string> catalogoNombresSKU;
    // Índice ordenado de productos para paginación y selección por número en CLI/GUI
    Designar::DynArray<ProductoPanaderia*> listaProductos;
    // Propiedad RAII de productos y recetas creados dinámicamente o cargados desde JSON
    Designar::DynArray<ProductoPanaderia*> productosPropios;
    Designar::DynArray<Receta*> recetasPropias;
    // Lotes de producción con trazabilidad (SLList) unificados en el sistema
    Designar::DynArray<LoteProduccion> lotesProduccion;
    // Árbol AVL para Kardex de inventario indexado por fecha
    KardexInventario kardex;
    // Min-Heap para gestión FEFO de insumos perecederos
    GestionPerecederos almacenPerecederos;

    void limpiarMemoriaPropia();
    
public:
    SistemaPanaderia();
    ~SistemaPanaderia();

    // =========================================================================
    // REGISTRO Y GESTIÓN DE CATÁLOGO (Integración POS y Nombres por SKU)
    // =========================================================================
    
    bool registrarProducto(ProductoPanaderia* producto, bool tomarPropiedad = false);
    bool registrarReceta(Receta* receta, bool tomarPropiedad = false);
    void registrarNombreSKU(const std::string& sku, const std::string& nombre);
    std::string obtenerNombreSKU(const std::string& sku) const;
    bool existeProducto(const std::string& sku) const;
    bool existeReceta(const std::string& sku) const;
    const Designar::DynArray<ProductoPanaderia*>& obtenerListaProductos() const;
    const Designar::HashMap<std::string, std::string>& obtenerCatalogoNombresSKU() const;

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

    bool procesarPedidoProduccion(const std::string& sku,
                                  double cantidadSolicitadaKg,
                                  const fecha& fechaHoy,
                                  Designar::DynArray<ElementoReceta>& insumosConsolidados);

    bool procesarPedidoProduccion(const std::string& sku,
                                  double cantidadSolicitadaKg,
                                  Designar::DynArray<ElementoReceta>& insumosConsolidados);

    void generarReporteCocina(const std::string& sku, double cantidadSolicitadaKg);
    void mostrarArbolReceta(const std::string& sku) const;

    // =========================================================================
    // PUNTO DE VENTA (POS) VINCULADO A STOCK Y KARDEX
    // =========================================================================

    bool registrarVentaPOS(const std::string& sku, int unidadesVendidas, const fecha& fechaVenta);
    bool registrarVentaPOS(const std::string& sku, int unidadesVendidas);

    // =========================================================================
    // TRAZABILIDAD DE LOTES DE PRODUCCIÓN (SLList)
    // =========================================================================

    void registrarLote(const LoteProduccion& lote);
    Designar::DynArray<LoteProduccion>& getLotes();
    const Designar::DynArray<LoteProduccion>& getLotes() const;
    void mostrarHistorialLote(const LoteProduccion& lote) const;

    // =========================================================================
    // KARDEX DE INVENTARIO (Árbol AVL) Y PERECEDEROS FEFO (Min-Heap)
    // =========================================================================

    void registrarMovimientoInventario(const MovimientoKardex& mov);
    void consultarKardexPorRangoFechas(const fecha& fechaInicio, const fecha& fechaFin) const;
    KardexInventario& getKardex();
    const KardexInventario& getKardex() const;

    void registrarPerecedero(const ProductoPanaderia& prod);
    ProductoPanaderia consultarProximoAVencer() const;
    ProductoPanaderia despacharProximoAVencer();
    bool tienePerecederos() const;
    size_t totalPerecederos() const;
    GestionPerecederos& getAlmacenPerecederos();
    const GestionPerecederos& getAlmacenPerecederos() const;

    // =========================================================================
    // MEMORIA PERSISTENTE (JSON COMPARTIDO ENTRE CLI Y GUI)
    // =========================================================================

    void cargarDatosPorDefecto();
    bool guardarDatosJSON(const std::string& rutaArchivo = "data/bakery_data.json") const;
    bool cargarDatosJSON(const std::string& rutaArchivo = "data/bakery_data.json");
    void inicializarConPersistencia(const std::string& rutaArchivo = "data/bakery_data.json");
};

fecha obtenerFechaActual();

#endif // SISTEMA_PANADERIA_HPP