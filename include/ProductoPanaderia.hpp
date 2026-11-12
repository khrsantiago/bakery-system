#ifndef PRODUCTO_PANADERIA_HPP
#define PRODUCTO_PANADERIA_HPP

#include <string>
#include <iostream>
#include <array.hpp>
struct fecha
{
    int dia,mes,anio;
    bool operator<(const fecha &f) const
    {
        if (anio != f.anio) return anio < f.anio;
        if (mes != f.mes)   return mes < f.mes;
        return dia < f.dia;
    }
    bool operator==(const fecha &f) const
    {
        return dia == f.dia && mes == f.mes && anio == f.anio;
    }
};

// Enum requerido para clasificar los tipos de masa panadera
enum class TipoMasaBase
{
    DULCE_SOBADA,       // Pan de jamón, piñitas, acemas
    HOJALDRADA,         // Croissants, cachitos hojaldrados
    MADRE_ACIDA,        // Pan rústico, sourdough / masa madre
    BLANDA_ENRIQUECIDA, // Pan de molde, brioche, hamburguesa
    CRUJIENTE_RUSTICA,  // Canilla venezolana, baguette, pan campesino
    INTEGRAL            // Panes con salvado o centeno
};

// Función auxiliar para convertir el enum a texto legible
inline std::string tipoMasaToString(TipoMasaBase tipo)
{
    switch (tipo)
    {
        case TipoMasaBase::DULCE_SOBADA:       return "Masa Dulce Sobada";
        case TipoMasaBase::HOJALDRADA:         return "Masa Hojaldrada";
        case TipoMasaBase::MADRE_ACIDA:        return "Masa Madre Acida";
        case TipoMasaBase::BLANDA_ENRIQUECIDA: return "Masa Blanda Enriquecida";
        case TipoMasaBase::CRUJIENTE_RUSTICA:  return "Masa Crujiente Rustica";
        case TipoMasaBase::INTEGRAL:           return "Masa Integral";
        default:                               return "Desconocida";
    }
}

class ProductoPanaderia
{
private:
    std::string codigoSKU;
    std::string nombre;
    TipoMasaBase tipoMasaBase;
    double porcentajeHidratacion;
    int tiempoFermentacion;
    double temperaturaFermentacion;
    int tiempoHorneado;
    int temperaturaHorneado;
    double rendimientoMasa;
    Designar::DynArray<std::string> alergenos;
    int stockActual;
    int stockMinimo;
    fecha fechaVencimiento;
    double costoUnitario;
    bool esPerecedero;
public:
    ProductoPanaderia();
    ProductoPanaderia(std::string sku, std::string nom, TipoMasaBase masa, double hidratacion, int tFerm, double tempFerm, int tHorn, int tempHorn, double rendimiento, int stockAct, int stockMin, fecha fVenc, double precio = 0.0);
    
    std::string getSKU() const { return codigoSKU; }
    std::string getNombre() const { return nombre; }
    TipoMasaBase getTipoMasa() const { return tipoMasaBase; }
    std::string getTipoMasaNombre() const { return tipoMasaToString(tipoMasaBase); }
    double getPorcentajeHidratacion() const { return porcentajeHidratacion; }
    int getTiempoFermentacion() const { return tiempoFermentacion; }
    double getTemperaturaFermentacion() const { return temperaturaFermentacion; }
    int getTiempoHorneado() const { return tiempoHorneado; }
    int getTemperaturaHorneado() const { return temperaturaHorneado; }
    double getRendimientoMasa() const { return rendimientoMasa; }
    int getStockActual() const { return stockActual; }
    int getStockMinimo() const { return stockMinimo; }
    fecha getFechaVencimiento() const { return fechaVencimiento; }
    double getCostoUnitario() const { return costoUnitario; }
    const Designar::DynArray<std::string>& getAlergenos() const { return alergenos; }

    void setStockActual(int nuevoStock) { stockActual = nuevoStock; }
    
    void reducirStock(int cantidad);
    void aumentarStock(int cantidad);
    void agregarAlergeno(const std::string &alergeno);
    bool requiereReabastecimiento() const;
    bool estaVencido(const fecha &fechaHoy) const;
    bool operator==(const ProductoPanaderia &otro) const;
    void mostrarInfo() const;
};

#endif