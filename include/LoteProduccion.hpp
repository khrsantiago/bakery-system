#ifndef LOTE_PRODUCCION_HPP
#define LOTE_PRODUCCION_HPP

#include <string>
#include <iostream>
#include <list.hpp> // DeSiGNAR: SLList
#include "ProductoPanaderia.hpp"

// Evento individual dentro de la línea de tiempo del lote
struct EventoTrazabilidad
{
    std::string etapa;       // Ej. "Pesaje", "Amasado", "Fermentación", "Horneado"
    std::string descripcion; // Detalles (temperatura, tiempo, observaciones)
    std::string responsable; // Nombre del maestro panadero u operario
    fecha fechaEvento;

    void mostrarInfo() const
    {
        std::cout << "  [" << fechaEvento.dia << "/" << fechaEvento.mes << "/" << fechaEvento.anio << "] "
                  << etapa << " | " << descripcion 
                  << " (Responsable: " << responsable << ")\n";
    }
};

class LoteProduccion
{
private:
    std::string codigoLote;
    std::string codigoSKU;
    double cantidadPlanificada; // Cantidad en kg o unidades
    fecha fechaElaboracion;
    Designar::SLList<EventoTrazabilidad> historialEventos; // Lista simplemente enlazada

public:
    LoteProduccion();
    LoteProduccion(std::string codLote, std::string sku, double cantidad, fecha fElab);

    std::string getCodigoLote() const { return codigoLote; }
    std::string getCodigoSKU() const { return codigoSKU; }
    double getCantidadPlanificada() const { return cantidadPlanificada; }
    fecha getFechaElaboracion() const { return fechaElaboracion; }

    // Registra un evento al final de la lista cronológica
    void registrarEvento(const std::string& etapa, const std::string& desc,
                         const std::string& resp, const fecha& f);
    void registrarEvento(const EventoTrazabilidad& evento);

    // Muestra toda la línea de tiempo de auditoría
    void mostrarHistorialTrazabilidad() const;
    size_t getNumeroEventos() const { return historialEventos.size(); }
    const Designar::SLList<EventoTrazabilidad>& getEventos() const { return historialEventos; }
};

#endif // LOTE_PRODUCCION_HPP