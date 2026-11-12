#ifndef GESTION_PERECEDEROS_HPP
#define GESTION_PERECEDEROS_HPP

#include <iostream>
#include <heap.hpp> // DeSiGNAR: DynHeap
#include "ProductoPanaderia.hpp"

// Criterio de ordenación para el Min-Heap:
// El producto con fecha de vencimiento más temprana tiene mayor prioridad
struct ComparadorVencimiento
{
    bool operator()(const ProductoPanaderia& a, const ProductoPanaderia& b) const
    {
        return a.getFechaVencimiento() < b.getFechaVencimiento();
    }
};

class GestionPerecederos
{
private:
    Designar::DynHeap<ProductoPanaderia, ComparadorVencimiento> heapPerecederos;

public:
    GestionPerecederos() {}

    // Inserta un producto o materia prima al montículo
    void registrarPerecedero(const ProductoPanaderia& prod)
    {
        heapPerecederos.insert(prod);
    }

    // Consulta el próximo a vencer sin extraerlo
    ProductoPanaderia consultarProximoAVencer() const
    {
        return heapPerecederos.top();
    }

    // Extrae y despacha el producto más próximo a caducar (estrategia FEFO)
    ProductoPanaderia despacharProximoAVencer()
    {
        return heapPerecederos.get();
    }

    bool tienePerecederos() const
    {
        return !heapPerecederos.is_empty();
    }

    size_t totalPerecederos() const
    {
        return heapPerecederos.size();
    }
};

#endif // GESTION_PERECEDEROS_HPP