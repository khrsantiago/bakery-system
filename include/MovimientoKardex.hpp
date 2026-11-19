#ifndef MOVIMIENTO_KARDEX_HPP
#define MOVIMIENTO_KARDEX_HPP

#include <cstddef>
#include <string>
#include <iostream>
#include "ProductoPanaderia.hpp" // Provee la struct fecha

// Clasificación del tipo de transacción de inventario
enum class TipoMovimiento {
    ENTRADA_COMPRA,
    EGRESO_PRODUCCION,
    AJUSTE_INVENTARIO
};

// Función auxiliar para convertir el enum TipoMovimiento a texto legible
inline std::string tipoMovimientoToString(TipoMovimiento tipo)
{
    switch (tipo)
    {
        case TipoMovimiento::ENTRADA_COMPRA:    return "ENTRADA (Compra)";
        case TipoMovimiento::EGRESO_PRODUCCION: return "EGRESO (Produccion)";
        case TipoMovimiento::AJUSTE_INVENTARIO: return "AJUSTE (Inventario)";
        default:                                return "DESCONOCIDO";
    }
}

// Estructura que representa cada transacción en el Kardex
struct MovimientoKardex {
    fecha fechaTransaccion;
    std::string codigoSKU;     // SKU de la materia prima (ej. MP-HAR-001)
    TipoMovimiento tipo;       // EGRESO_PRODUCCION
    double cantidad;           // Cantidad desglosada y consolidada por el DFS
    std::string descripcion;   // Ej: "Salida por produccion de SKU: PAN-JAM-001"
    size_t idMovimiento = 0;   // Identificador secuencial para desempate estricto en el AVL

    // Sobrecarga del operador < requerida para el ordenamiento en el AVL (orden por fecha e ID)
    bool operator<(const MovimientoKardex& otro) const
    {
        if (!(fechaTransaccion == otro.fechaTransaccion))
        {
            return fechaTransaccion < otro.fechaTransaccion;
        }
        return idMovimiento < otro.idMovimiento;
    }

    // Método auxiliar para imprimir el movimiento en consola
    void mostrarInfo() const
    {
        std::cout << "[" << fechaTransaccion.dia << "/" << fechaTransaccion.mes << "/" << fechaTransaccion.anio << "] "
                  << "[" << tipoMovimientoToString(tipo) << "] "
                  << "SKU: " << codigoSKU
                  << " | Cantidad: " << cantidad
                  << " | Concepto: " << descripcion;
    }
};

#endif // MOVIMIENTO_KARDEX_HPP
