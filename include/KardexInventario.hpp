#ifndef KARDEX_INVENTARIO_HPP
#define KARDEX_INVENTARIO_HPP

#include <cstddef>
#include <string>
#include <avltree.hpp> // DeSiGNAR: AVLTree auto-balanceado
#include <array.hpp>   // DeSiGNAR: DynArray
#include "MovimientoKardex.hpp"

/**
 * @class KardexInventario
 * @brief Gestiona el historial cronológico de entradas, egresos y ajustes de
 *        inventario mediante un Árbol AVL (Designar::AVLTree) indexado por fecha.
 *
 * Garantiza inserciones auto-balanceadas en O(log n) sin degradación, y
 * consultas por rango de fechas mediante recorrido in-order acotado.
 */
class KardexInventario
{
private:
    Designar::AVLTree<MovimientoKardex> arbolMovimientos;
    size_t siguienteId; // Generador secuencial para orden total estricto en fechas iguales

public:
    KardexInventario();
    ~KardexInventario();

    // =========================================================================
    // REGISTRO DE MOVIMIENTOS EN EL AVL — Complejidad O(log n)
    // =========================================================================

    // Registra un movimiento individual asignándole su ID secuencial en el Kardex
    void registrarMovimiento(const MovimientoKardex& movimiento);

    // Sobrecarga conveniente para construir y registrar el movimiento directamente
    void registrarMovimiento(const fecha& fechaTransaccion,
                             const std::string& codigoSKU,
                             TipoMovimiento tipo,
                             double cantidad,
                             const std::string& descripcion);

    // Enlace directo con la salida consolidada de generarMovimientosEgreso() (DFS -> Kardex)
    void registrarMovimientos(const Designar::DynArray<MovimientoKardex>& movimientos);

    // =========================================================================
    // CONSULTAS POR RANGO DE FECHAS — Recorrido in-order acotado
    // =========================================================================

    // Retorna todos los movimientos ocurridos en el intervalo cerrado [fechaInicio, fechaFin]
    Designar::DynArray<MovimientoKardex> consultarPorRangoFechas(const fecha& fechaInicio,
                                                                 const fecha& fechaFin) const;

    // Filtra movimientos dentro de [fechaInicio, fechaFin] por tipo (ej. solo egresos o entradas)
    Designar::DynArray<MovimientoKardex> consultarPorRangoYTipo(const fecha& fechaInicio,
                                                                const fecha& fechaFin,
                                                                TipoMovimiento tipo) const;

    // Filtra movimientos dentro de [fechaInicio, fechaFin] para un SKU de insumo específico
    Designar::DynArray<MovimientoKardex> consultarPorRangoYSKU(const fecha& fechaInicio,
                                                               const fecha& fechaFin,
                                                               const std::string& codigoSKU) const;

    // =========================================================================
    // REPORTES EN CONSOLA
    // =========================================================================

    void mostrarMovimientosPorRango(const fecha& fechaInicio, const fecha& fechaFin) const;
    void mostrarHistorialCompleto() const;

    // =========================================================================
    // ESTADO Y VERIFICACIÓN DEL ÁRBOL AVL
    // =========================================================================

    size_t totalMovimientos() const;
    bool estaVacio() const;
    bool verificarBalanceoAVL() const;
};

#endif // KARDEX_INVENTARIO_HPP
