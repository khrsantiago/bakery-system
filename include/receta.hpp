#ifndef RECETA_HPP
#define RECETA_HPP
#include <mtreenode.hpp>
#include "producto.hpp"
class ElementoReceta
{
private:
    std::string codigoSKU;
    double cantidad;
    std::string unidadMedida;
public:
    ElementoReceta();
    ElementoReceta(std::string cSKU, double cantidad, std::string uMedida);

    std::string getCodigoSKU() const { return codigoSKU; }
    double getCantidad() const { return cantidad; }
    std::string getUnidadMedida() const { return unidadMedida; }

    void setCodigoSKU(const std::string &sku) { codigoSKU = sku; }
    void setCantidad(double c) { cantidad = c; }
    void setUnidadMedida(const std::string &um) { unidadMedida = um; }

    void escalarCantidad(double factor);
    double getCantidadEscalada(double factor) const;
    bool operator==(const ElementoReceta &otro) const;
    void mostrarInfo() const;
};

using NodoReceta = Designar::MTreeNode<ElementoReceta>;

class Receta
{
private:
    NodoReceta* raiz;
    std::string codigoSKU;
    std::string nombre;
    NodoReceta* buscarElemento(NodoReceta* r,std::string codigoSKU);
    void desglozarReceta(NodoReceta* r,double cantidadRequerida, Designar::DynArray<ElementoReceta>& insumos);
public:
    Receta(const ProductoPanaderia& producto);
    
    void agregarElemento(NodoReceta* padre,const ElementoReceta& elemento);
    NodoReceta* buscarElemento(std::string codigoSKU);
    void calcularMateriasPrimas(double cantidadRequerida, Designar::DynArray<ElementoReceta>& insumos);
    ~Receta();
};
#endif