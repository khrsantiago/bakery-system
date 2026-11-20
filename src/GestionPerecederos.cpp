#include "GestionPerecederos.hpp"

bool ComparadorVencimiento::operator()(const ProductoPanaderia& a, const ProductoPanaderia& b) const
{
    return a.getFechaVencimiento() < b.getFechaVencimiento();
}

GestionPerecederos::GestionPerecederos() = default;

void GestionPerecederos::registrarPerecedero(const ProductoPanaderia& prod)
{
    heapPerecederos.insert(prod);
}

ProductoPanaderia GestionPerecederos::consultarProximoAVencer() const
{
    return heapPerecederos.top();
}

ProductoPanaderia GestionPerecederos::despacharProximoAVencer()
{
    return heapPerecederos.get();
}

bool GestionPerecederos::tienePerecederos() const
{
    return !heapPerecederos.is_empty();
}

size_t GestionPerecederos::totalPerecederos() const
{
    return heapPerecederos.size();
}
