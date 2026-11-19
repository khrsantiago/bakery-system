#ifndef MOVIMIENTO_KARDEX_HPP
#define MOVIMIENTO_KARDEX_HPP

#include <string>
#include <iostream>
#include "ProductoPanaderia.hpp" // Provee la struct fecha

// Clasificación del tipo de transacción de inventario
enum class TipoMovimiento {
    ENTRADA_COMPRA,
    EGRESO_PRODUCCION,
    AJUSTE_INVENTARIO
};

// Estructura que representa cada transacción en el Kardex
struct MovimientoKardex {
    fecha fechaTransaccion;
    std::string codigoSKU;     // SKU de la materia prima (ej. MP-HAR-001)
    TipoMovimiento tipo;       // EGRESO_PRODUCCION
    double cantidad;           // Cantidad desglosada y consolidada por el DFS
    std::string descripcion;   // Ej: "Salida por produccion de SKU: PAN-JAM-001"

    // Sobrecarga del operador < requerida para el ordenamiento en el AVL (orden por fecha)
    bool operator<(const MovimientoKardex& otro) const {
        return fechaTransaccion < otro.fechaTransaccion;
    }

    // Método auxiliar para imprimir el movimiento en consola
    void mostrarInfo() const {
        std::cout << "[" << fechaTransaccion.dia << "/" << fechaTransaccion.mes << "/" << fechaTransaccion.anio << "] "
                  << "SKU Insumo: " << codigoSKU 
                  << " | Cantidad Egreso: " << cantidad 
                  << " | Concepto: " << descripcion;
    }
};

#endif // MOVIMIENTO_KARDEX_HPP