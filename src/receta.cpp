#include "receta.hpp"

ElementoReceta::ElementoReceta()
    : codigoSKU(""), cantidad(0.0), unidadMedida("") {}

ElementoReceta::ElementoReceta(std::string cSKU, double cant, std::string uMed)
    : codigoSKU(cSKU), cantidad(cant), unidadMedida(uMed) {}

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
    raiz=new NodoReceta(ElementoReceta(producto.getSKU(),0,"kg"));
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

void Receta::desglozarReceta(NodoReceta* r,double cantidadRequerida, Designar::DynArray<ElementoReceta>& insumos)
{
    if(r==nullptr) return;
    ElementoReceta elementoActual=r->get_key();
    if(r->is_leaf())
    {
        for(auto& i: insumos)
        {
            if(elementoActual.getCodigoSKU()==i.getCodigoSKU())
            {
                i.setCantidad(i.getCantidad()+cantidadRequerida);
                return;
            }
        }
        elementoActual.setCantidad(cantidadRequerida);
        insumos.append(elementoActual);
        return;
    }
    for(NodoReceta::ChildrenIterator it=NodoReceta::ChildrenIterator(*r);it.has_current();it.next())
    {
        ElementoReceta subelemento=it.get_current()->get_key();
        // TODO: Falta por recalcular la cantidad requerida del padre en funcion de su rendimiento de masa.
        double canReqSubelemento=cantidadRequerida*subelemento.getCantidad();
        desglozarReceta(it.get_current(),canReqSubelemento,insumos);
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