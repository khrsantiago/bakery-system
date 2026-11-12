#include "LoteProduccion.hpp"

LoteProduccion::LoteProduccion()
    : codigoLote(""), codigoSKU(""), cantidadPlanificada(0.0), fechaElaboracion{0, 0, 0} {}

LoteProduccion::LoteProduccion(std::string codLote, std::string sku, double cantidad, fecha fElab)
    : codigoLote(codLote), codigoSKU(sku), cantidadPlanificada(cantidad), fechaElaboracion(fElab) {}

void LoteProduccion::registrarEvento(const std::string& etapa, const std::string& desc,
                                     const std::string& resp, const fecha& f)
{
    EventoTrazabilidad nuevoEvento{etapa, desc, resp, f};
    historialEventos.append(nuevoEvento);
}

void LoteProduccion::registrarEvento(const EventoTrazabilidad& evento)
{
    historialEventos.append(evento);
}

void LoteProduccion::mostrarHistorialTrazabilidad() const
{
    std::cout << "==================================================\n";
    std::cout << " HISTORIAL DE TRAZABILIDAD DEL LOTE: " << codigoLote << "\n";
    std::cout << " Producto (SKU): " << codigoSKU 
              << " | Cantidad: " << cantidadPlanificada << " kg\n";
    std::cout << "==================================================\n";

    if (historialEventos.is_empty())
    {
        std::cout << "  (No se han registrado eventos para este lote)\n";
    }
    else
    {
        for (const auto& evento : historialEventos)
        {
            evento.mostrarInfo();
        }
    }
    std::cout << "==================================================\n\n";
}