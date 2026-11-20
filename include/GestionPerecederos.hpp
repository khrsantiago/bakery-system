#ifndef GESTION_PERECEDEROS_HPP
#define GESTION_PERECEDEROS_HPP

#include <cstddef>
#include <heap.hpp> // DeSiGNAR: DynHeap
#include "ProductoPanaderia.hpp"

// Criterio de ordenación para el Min-Heap:
// El producto con fecha de vencimiento más temprana tiene mayor prioridad
struct ComparadorVencimiento
{
    bool operator()(const ProductoPanaderia& a, const ProductoPanaderia& b) const;
};

class GestionPerecederos
{
private:
    Designar::DynHeap<ProductoPanaderia, ComparadorVencimiento> heapPerecederos;

public:
    GestionPerecederos();

    // Inserta un producto o materia prima al montículo
    void registrarPerecedero(const ProductoPanaderia& prod);

    // Consulta el próximo a vencer sin extraerlo
    ProductoPanaderia consultarProximoAVencer() const;

    // Extrae y despacha el producto más próximo a caducar (estrategia FEFO)
    ProductoPanaderia despacharProximoAVencer();

    bool tienePerecederos() const;

    size_t totalPerecederos() const;
};

#endif // GESTION_PERECEDEROS_HPP