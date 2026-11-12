#include "receta.hpp"

ElementoReceta::ElementoReceta()
    : codigoSKU(""), cantidad(0.0), unidadMedida(""), factorRendimiento(1.0) {}

ElementoReceta::ElementoReceta(std::string cSKU, double cant, std::string uMed, double rendimiento)
    : codigoSKU(cSKU), cantidad(cant), unidadMedida(uMed), factorRendimiento(rendimiento) {}

void ElementoReceta::escalarCantidad(double factor)
{
    cantidad *= factor;
}

double ElementoReceta::getCantidadEscalada(double factor) const
{
    return cantidad * factor;
}

bool ElementoReceta::operator==(const ElementoReceta &otro) const
{
    return codigoSKU == otro.codigoSKU;
}

void ElementoReceta::mostrarInfo() const
{
    std::cout << "SKU: " << codigoSKU 
              << " | Cantidad: " << cantidad << " " << unidadMedida;
}

Receta::Receta(const ProductoPanaderia& producto):codigoSKU(producto.getSKU()),nombre(producto.getNombre())
{
    raiz=new NodoReceta(ElementoReceta(producto.getSKU(),1.0,"kg", producto.getRendimientoMasa()));
}

void Receta::agregarElemento(NodoReceta* padre,const ElementoReceta& elemento)
{
    NodoReceta* nuevoSubproducto=new NodoReceta(elemento);
    padre->append_child(nuevoSubproducto);
}

NodoReceta* Receta::buscarElemento(NodoReceta* r,std::string codigoSKU)
{
    if(r==nullptr) return nullptr;
    if(r->get_key().getCodigoSKU()==codigoSKU) return r;
    for(NodoReceta::ChildrenIterator it=NodoReceta::ChildrenIterator(*r);it.has_current();it.next())
    {
        NodoReceta* encontrado=buscarElemento(it.get_current(),codigoSKU);
        if (encontrado != nullptr)
        {
            return encontrado;
        }
    }
    return nullptr;
}

NodoReceta* Receta::buscarElemento(std::string codigoSKU)
{
    return buscarElemento(raiz,codigoSKU);
}

void Receta::desglozarReceta(NodoReceta* r, double cantidadRequerida, Designar::DynArray<ElementoReceta>& insumos)
{
    if (r == nullptr) return;
    ElementoReceta elementoActual = r->get_key();
    // Caso base: Nodo hoja (materia prima elemental)
    if (r->is_leaf())
    {
        for (auto& i : insumos)
        {
            if (elementoActual.getCodigoSKU() == i.getCodigoSKU())
            {
                i.setCantidad(i.getCantidad() + cantidadRequerida);
                return;
            }
        }
        elementoActual.setCantidad(cantidadRequerida);
        insumos.append(elementoActual);
        return;
    }
    // Caso recursivo: Nodo intermedio o raíz (producto o subproducto)
    // Se ajusta la cantidad requerida según la merma (factor de rendimiento):
    // Si rendimiento es 0.90 (90%), se formula para cantidadRequerida / 0.90
    double rendimiento = elementoActual.getFactorRendimiento();
    double cantidadAjustada = cantidadRequerida;
    if (rendimiento > 0.0 && rendimiento < 1.0)
    {
        cantidadAjustada = cantidadRequerida / rendimiento;
    }
    for (NodoReceta::ChildrenIterator it = NodoReceta::ChildrenIterator(*r); it.has_current(); it.next())
    {
        ElementoReceta subelemento = it.get_current()->get_key();
        double canReqSubelemento = cantidadAjustada * subelemento.getCantidad();
        desglozarReceta(it.get_current(), canReqSubelemento, insumos);
    }
}

void Receta::calcularMateriasPrimas(double cantidadRequerida, Designar::DynArray<ElementoReceta>& insumos)
{
    desglozarReceta(raiz,cantidadRequerida,insumos);
}

Receta::~Receta()
{
    NodoReceta::destroy_tree(raiz);
}