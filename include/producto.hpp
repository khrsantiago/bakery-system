#ifndef PRODUCTO_HPP
#define PRODUCTO_HPP
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
class ProductoPanaderia
{
private:
    std::string codigoSKU;
    std::string nombre;
    std::string tipoMasa;
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
    ProductoPanaderia(std::string sku, std::string nom, std::string masa, double hidratacion, int tFerm, double tempFerm, int tHorn, int tempHorn, double rendimiento, int stockAct, int stockMin, fecha fVenc, double precio = 0.0);
    
    std::string getSKU() const { return codigoSKU; }
    std::string getNombre() const { return nombre; }
    std::string getTipoMasa() const { return tipoMasa; }
    double getPorcentajeHidratacion() const { return porcentajeHidratacion; }
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