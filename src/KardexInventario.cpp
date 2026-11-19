#include "KardexInventario.hpp"
#include <iostream>

namespace
{
    /**
     * Ejecuta un recorrido in-order acotado sobre el Árbol AVL de movimientos,
     * filtrando el intervalo cerrado [fechaInicio, fechaFin] con corte temprano
     * una vez superada la fecha final.
     */
    template <typename Pred>
    Designar::DynArray<MovimientoKardex> filtrarRangoInorder(
        const Designar::AVLTree<MovimientoKardex>& arbol,
        const fecha& fechaInicio,
        const fecha& fechaFin,
        Pred cumpleFiltro)
    {
        Designar::DynArray<MovimientoKardex> resultado;

        if (arbol.is_empty() || fechaFin < fechaInicio)
        {
            return resultado;
        }

        for (Designar::AVLTree<MovimientoKardex>::InorderIterator it(arbol);
             it.has_current();
             it.next())
        {
            const MovimientoKardex& mov = it.get_current();

            if (mov.fechaTransaccion < fechaInicio)
            {
                continue;
            }

            // El recorrido in-order visita en orden cronológico ascendente:
            // al superar fechaFin ningún nodo posterior pertenecerá al rango.
            if (fechaFin < mov.fechaTransaccion)
            {
                break;
            }

            if (cumpleFiltro(mov))
            {
                resultado.append(mov);
            }
        }

        return resultado;
    }
} // namespace

KardexInventario::KardexInventario()
    : arbolMovimientos(), siguienteId(1)
{
}

KardexInventario::~KardexInventario() = default;

void KardexInventario::registrarMovimiento(const MovimientoKardex& movimiento)
{
    MovimientoKardex registro = movimiento;
    registro.idMovimiento = siguienteId++;
    arbolMovimientos.insert(registro);
}

void KardexInventario::registrarMovimiento(const fecha& fechaTransaccion,
                                           const std::string& codigoSKU,
                                           TipoMovimiento tipo,
                                           double cantidad,
                                           const std::string& descripcion)
{
    MovimientoKardex nuevoMovimiento{
        fechaTransaccion,
        codigoSKU,
        tipo,
        cantidad,
        descripcion,
        0
    };
    registrarMovimiento(nuevoMovimiento);
}

void KardexInventario::registrarMovimientos(const Designar::DynArray<MovimientoKardex>& movimientos)
{
    for (size_t i = 0; i < movimientos.size(); ++i)
    {
        registrarMovimiento(movimientos[i]);
    }
}

Designar::DynArray<MovimientoKardex> KardexInventario::consultarPorRangoFechas(
    const fecha& fechaInicio,
    const fecha& fechaFin) const
{
    return filtrarRangoInorder(
        arbolMovimientos,
        fechaInicio,
        fechaFin,
        [](const MovimientoKardex&) { return true; }
    );
}

Designar::DynArray<MovimientoKardex> KardexInventario::consultarPorRangoYTipo(
    const fecha& fechaInicio,
    const fecha& fechaFin,
    TipoMovimiento tipo) const
{
    return filtrarRangoInorder(
        arbolMovimientos,
        fechaInicio,
        fechaFin,
        [tipo](const MovimientoKardex& mov) { return mov.tipo == tipo; }
    );
}

Designar::DynArray<MovimientoKardex> KardexInventario::consultarPorRangoYSKU(
    const fecha& fechaInicio,
    const fecha& fechaFin,
    const std::string& codigoSKU) const
{
    return filtrarRangoInorder(
        arbolMovimientos,
        fechaInicio,
        fechaFin,
        [&codigoSKU](const MovimientoKardex& mov) { return mov.codigoSKU == codigoSKU; }
    );
}

void KardexInventario::mostrarMovimientosPorRango(const fecha& fechaInicio,
                                                  const fecha& fechaFin) const
{
    std::cout << "=======================================================\n";
    std::cout << " KARDEX DE INVENTARIO (RANGO: "
              << fechaInicio.dia << "/" << fechaInicio.mes << "/" << fechaInicio.anio
              << " - "
              << fechaFin.dia << "/" << fechaFin.mes << "/" << fechaFin.anio << ")\n";
    std::cout << "=======================================================\n";

    if (fechaFin < fechaInicio)
    {
        std::cout << " [!] Rango de fechas invalido: la fecha final es anterior a la inicial.\n";
        std::cout << "=======================================================\n\n";
        return;
    }

    Designar::DynArray<MovimientoKardex> movimientos =
        consultarPorRangoFechas(fechaInicio, fechaFin);

    if (movimientos.is_empty())
    {
        std::cout << "  (No se registraron movimientos en este rango de fechas)\n";
    }
    else
    {
        for (size_t i = 0; i < movimientos.size(); ++i)
        {
            std::cout << "  [" << (i + 1) << "] ";
            movimientos[i].mostrarInfo();
            std::cout << "\n";
        }
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << " Movimientos en rango : " << movimientos.size() << "\n";
    std::cout << " Total en Kardex AVL  : " << arbolMovimientos.size() << "\n";
    std::cout << "=======================================================\n\n";
}

void KardexInventario::mostrarHistorialCompleto() const
{
    std::cout << "=======================================================\n";
    std::cout << "      HISTORIAL COMPLETO DEL KARDEX (ARBOL AVL)        \n";
    std::cout << "=======================================================\n";

    if (arbolMovimientos.is_empty())
    {
        std::cout << "  (El Kardex de inventario esta vacio)\n";
    }
    else
    {
        size_t indice = 1;
        for (Designar::AVLTree<MovimientoKardex>::InorderIterator it(arbolMovimientos);
             it.has_current();
             it.next())
        {
            std::cout << "  [" << indice++ << "] ";
            it.get_current().mostrarInfo();
            std::cout << "\n";
        }
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << " Total de registros : " << arbolMovimientos.size() << "\n";
    std::cout << " Estado del AVL     : "
              << (arbolMovimientos.verify() ? "BALANCEADO (OK)" : "DESBALANCEADO (ERROR)")
              << "\n";
    std::cout << "=======================================================\n\n";
}

size_t KardexInventario::totalMovimientos() const
{
    return arbolMovimientos.size();
}

bool KardexInventario::estaVacio() const
{
    return arbolMovimientos.is_empty();
}

bool KardexInventario::verificarBalanceoAVL() const
{
    return arbolMovimientos.verify();
}
