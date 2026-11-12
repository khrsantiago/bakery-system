#ifndef RECETA_HPP
#define RECETA_HPP
#include <mtreenode.hpp>
#include "ProductoPanaderia.hpp"

class ElementoReceta
{
private:
    std::string codigoSKU;
    double cantidad;
    std::string unidadMedida;
    double factorRendimiento;
public:
    ElementoReceta();
    ElementoReceta(std::string cSKU, double cantidad, std::string uMedida, double rendimiento = 1.0);

    std::string getCodigoSKU() const { return codigoSKU; }
    double getCantidad() const { return cantidad; }
    std::string getUnidadMedida() const { return unidadMedida; }
    double getFactorRendimiento() const { return factorRendimiento; }

    void setCodigoSKU(const std::string &sku) { codigoSKU = sku; }
    void setCantidad(double c) { cantidad = c; }
    void setUnidadMedida(const std::string &um) { unidadMedida = um; }
    void setFactorRendimiento(double r) { factorRendimiento = r; }

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