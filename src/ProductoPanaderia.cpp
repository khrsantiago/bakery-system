#include "ProductoPanaderia.hpp"

ProductoPanaderia::ProductoPanaderia(): tipoMasaBase(TipoMasaBase::BLANDA_ENRIQUECIDA), porcentajeHidratacion(0.0), tiempoFermentacion(0), temperaturaFermentacion(0.0),
      tiempoHorneado(0), temperaturaHorneado(0), rendimientoMasa(1.0),
      stockActual(0), stockMinimo(0), fechaVencimiento{0, 0, 0}, costoUnitario(0.0) {}

ProductoPanaderia::ProductoPanaderia(std::string sku, std::string nom, TipoMasaBase masa,
                                     double hidratacion, int tFerm, double tempFerm,
                                     int tHorn, int tempHorn, double rendimiento,
                                     int stockAct, int stockMin, fecha fVenc, double precio)
    : codigoSKU(sku), nombre(nom), tipoMasaBase(masa),
      porcentajeHidratacion(hidratacion), tiempoFermentacion(tFerm),
      temperaturaFermentacion(tempFerm), tiempoHorneado(tHorn),
      temperaturaHorneado(tempHorn), rendimientoMasa(rendimiento),
      stockActual(stockAct), stockMinimo(stockMin),
      fechaVencimiento(fVenc), costoUnitario(precio) {}

void ProductoPanaderia::reducirStock(int cantidad)
{
    if (cantidad <= stockActual)
        stockActual -= cantidad;
    else
        stockActual = 0;
}

void ProductoPanaderia::aumentarStock(int cantidad)
{
    if (cantidad > 0)
        stockActual += cantidad;
}

void ProductoPanaderia::agregarAlergeno(const std::string &alergeno)
{
    if (!alergeno.empty())
    {
        alergenos.append(alergeno);
    }
}

bool ProductoPanaderia::requiereReabastecimiento() const
{
    return stockActual <= stockMinimo;
}

bool ProductoPanaderia::estaVencido(const fecha &fechaHoy) const
{
    return fechaVencimiento < fechaHoy;
}

bool ProductoPanaderia::operator==(const ProductoPanaderia &otro) const
{
    return codigoSKU == otro.codigoSKU;
}

void ProductoPanaderia::mostrarInfo() const
{
    std::cout << "========================================\n"
              << "SKU: " << codigoSKU << " | Nombre: " << nombre << "\n"
              << "Tipo Masa: " << getTipoMasaNombre() << " | Hidratacion: " << porcentajeHidratacion << "%\n"
              << "Fermentacion: " << tiempoFermentacion << " min a " << temperaturaFermentacion << "C\n"
              << "Horneado: " << tiempoHorneado << " min a " << temperaturaHorneado << "C\n"
              << "Rendimiento Masa: " << rendimientoMasa << "\n"
              << "Stock: " << stockActual << " (Min: " << stockMinimo << ")\n"
              << "Vencimiento: " << fechaVencimiento.dia << "/"
              << fechaVencimiento.mes << "/" << fechaVencimiento.anio << "\n"
              << "Alergenos: ";

    if (alergenos.is_empty())
    {
        std::cout << "Ninguno";
    }
    else
    {
        for (size_t i = 0; i < alergenos.size(); ++i)
        {
            std::cout << alergenos[i] << (i + 1 < alergenos.size() ? ", " : "");
        }
    }
    std::cout << "\n========================================\n";
}