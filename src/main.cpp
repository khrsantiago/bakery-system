#include <iostream>
#include <limits>
#include "ProductoPanaderia.hpp"
#include "receta.hpp"
#include "LoteProduccion.hpp"
#include "GestionPerecederos.hpp"

// Función auxiliar para pausar y esperar tecla Enter
void pausar()
{
    std::cout << "\nPresione ENTER para continuar...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Función auxiliar para limpiar el buffer de entrada tras leer números
void limpiarBuffer()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main()
{
    // =========================================================================
    // CONFIGURACIÓN INICIAL DEL ENTORNO DE PRUEBAS
    // =========================================================================

    // 1. Producto y Receta
    fecha fechaVencPan = {15, 11, 2026};
    ProductoPanaderia panDeJamon(
        "PAN-JAM-001",
        "Pan de Jamon Tradicional",
        TipoMasaBase::DULCE_SOBADA,
        55.0,
        180,
        30.0,
        35,
        180,
        0.90, // Rendimiento: 90% (merma del 10%)
        50,
        10,
        fechaVencPan,
        12.50
    );

    Receta receta(panDeJamon);
    NodoReceta* raiz = receta.buscarElemento("PAN-JAM-001");
    if (raiz != nullptr)
    {
        // Subproducto: Relleno tradicional
        receta.agregarElemento(raiz, ElementoReceta("SUB-REL-001", 0.45, "kg"));
        NodoReceta* nodoRelleno = receta.buscarElemento("SUB-REL-001");
        if (nodoRelleno != nullptr)
        {
            receta.agregarElemento(nodoRelleno, ElementoReceta("MP-JAM-001", 0.70, "kg")); // Jamón
            receta.agregarElemento(nodoRelleno, ElementoReceta("MP-TOC-001", 0.15, "kg")); // Tocineta
            receta.agregarElemento(nodoRelleno, ElementoReceta("MP-PAS-001", 0.10, "kg")); // Pasitas
            receta.agregarElemento(nodoRelleno, ElementoReceta("MP-ACE-001", 0.05, "kg")); // Aceitunas
        }

        // Ingredientes directos de masa
        receta.agregarElemento(raiz, ElementoReceta("MP-HAR-001", 0.55, "kg")); // Harina Panadera
        receta.agregarElemento(raiz, ElementoReceta("MP-MAR-001", 0.06, "kg")); // Margarina
        receta.agregarElemento(raiz, ElementoReceta("MP-AZU-001", 0.05, "kg")); // Azúcar
        receta.agregarElemento(raiz, ElementoReceta("MP-AGU-001", 0.20, "kg")); // Agua / Líquidos
        receta.agregarElemento(raiz, ElementoReceta("MP-LEV-001", 0.02, "kg")); // Levadura Fresca
        receta.agregarElemento(raiz, ElementoReceta("MP-PAP-001", 0.05, "kg")); // Papelón
    }

    // 2. Lote de Producción con Trazabilidad (SLList)
    LoteProduccion lote1("LOT-20261106-001", panDeJamon.getSKU(), 10.0, {6, 11, 2026});
    lote1.registrarEvento("Pesaje", "Pesaje exacto de harina, liquidos y relleno", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Amasado", "Amasado durante 12 min hasta desarrollar gluten", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Armado y Sobado", "Laminado e incorporacion de jamon, tocineta y pasas", "Khristopher Santiago", {6, 11, 2026});
    lote1.registrarEvento("Fermentacion Final", "Camara a 30C durante 180 min", "Luis Acevedo", {6, 11, 2026});
    lote1.registrarEvento("Horneado", "Horneado a 180C durante 35 min", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Control de Calidad", "Inspeccion de corteza, peso y empaque", "Khristopher Santiago", {6, 11, 2026});

    // 3. Almacén de Perecederos (DynHeap / Min-Heap FEFO)
    GestionPerecederos almacen;
    almacen.registrarPerecedero(ProductoPanaderia("MP-LEV-001", "Levadura Fresca en Pasta", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                                 0, 0, 0, 0, 0, 1.0, 20, 5, {12, 11, 2026}, 1.20));
    almacen.registrarPerecedero(ProductoPanaderia("MP-JAM-001", "Jamon Ahumado Especial", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                                 0, 0, 0, 0, 0, 1.0, 50, 10, {20, 11, 2026}, 8.50));
    almacen.registrarPerecedero(ProductoPanaderia("MP-LEC-001", "Leche Pasteurizada Completa", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                                 0, 0, 0, 0, 0, 1.0, 30, 5, {8, 11, 2026}, 1.80)); // Vence primero
    almacen.registrarPerecedero(ProductoPanaderia("MP-MAR-001", "Margarina con Sal", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                                 0, 0, 0, 0, 0, 1.0, 40, 10, {25, 12, 2026}, 3.00));

    // =========================================================================
    // BUCLE DEL MENÚ INTERACTIVO (CLI)
    // =========================================================================
    int opcion = 0;
    while (opcion != 4)
    {
        std::cout << "\n==================================================\n";
        std::cout << "         BAKERY SYSTEM - MENU PRINCIPAL          \n";
        std::cout << "  Programacion III - ULA | Preparaduria / Lab    \n";
        std::cout << "  Autores: Luis Acevedo, Khristopher Santiago,   \n";
        std::cout << "           Juan Rincon                           \n";
        std::cout << "==================================================\n";
        std::cout << " 1. Explosion Recursiva de Receta (Arbol DFS)\n";
        std::cout << " 2. Trazabilidad del Lote (Designar::SLList)\n";
        std::cout << " 3. Gestion de Perecederos FEFO (Min-Heap)\n";
        std::cout << " 4. Salir\n";
        std::cout << "==================================================\n";
        std::cout << "Seleccione una opcion [1-4]: ";

        if (!(std::cin >> opcion))
        {
            std::cin.clear();
            limpiarBuffer();
            std::cout << "[!] Entrada invalida. Ingrese un numero del 1 al 4.\n";
            continue;
        }
        limpiarBuffer();

        switch (opcion)
        {
            case 1:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 1. EXPLOSION DE RECETA Y CALCULO DE MATERIAS PRIMAS\n";
                std::cout << "--------------------------------------------------\n";
                panDeJamon.mostrarInfo();

                double cantidadDeseada = 10.0;
                std::cout << "\nIngrese la cantidad a producir en kg [Por defecto: 10.0]: ";
                std::string input;
                std::getline(std::cin, input);
                if (!input.empty())
                {
                    try {
                        cantidadDeseada = std::stod(input);
                    } catch (...) {
                        std::cout << "[!] Entrada no valida. Se usaran 10.0 kg por defecto.\n";
                        cantidadDeseada = 10.0;
                    }
                }

                Designar::DynArray<ElementoReceta> insumosTotales;
                std::cout << "\n--> Ejecutando recorrido DFS con rendimiento (" 
                          << (panDeJamon.getRendimientoMasa() * 100.0) << "%)...\n\n";

                receta.calcularMateriasPrimas(cantidadDeseada, insumosTotales);

                std::cout << "==================================================\n";
                std::cout << " REPORTE DE MATERIAS PRIMAS (Orden: " << cantidadDeseada << " kg)\n";
                std::cout << "==================================================\n";
                for (size_t i = 0; i < insumosTotales.size(); ++i)
                {
                    std::cout << " [" << (i + 1) << "] ";
                    insumosTotales[i].mostrarInfo();
                    std::cout << "\n";
                }
                std::cout << "--------------------------------------------------\n";
                std::cout << " Total de insumos elementales: " << insumosTotales.size() << "\n";
                std::cout << "==================================================\n";
                pausar();
                break;
            }

            case 2:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 2. TRAZABILIDAD DEL LOTE DE PRODUCCION (SLList)  \n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Mostrar historial cronologico completo\n";
                std::cout << " 2. Registrar nuevo evento de auditoria\n";
                std::cout << " Seleccione una accion [1-2]: ";

                int subopc = 1;
                std::cin >> subopc;
                limpiarBuffer();

                if (subopc == 2)
                {
                    std::string etapa, desc, resp;
                    std::cout << " Ingrese la etapa (ej. Enfriamiento, Despacho): ";
                    std::getline(std::cin, etapa);
                    std::cout << " Ingrese la descripcion: ";
                    std::getline(std::cin, desc);
                    std::cout << " Ingrese el responsable: ";
                    std::getline(std::cin, resp);

                    lote1.registrarEvento(etapa, desc, resp, {6, 11, 2026});
                    std::cout << "\n[+] Evento agregado a la lista simplemente enlazada.\n\n";
                }

                lote1.mostrarHistorialTrazabilidad();
                pausar();
                break;
            }

            case 3:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 3. GESTION DE PERECEDEROS (Min-Heap / FEFO)      \n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " Insumos perecederos en almacen: " << almacen.totalPerecederos() << "\n\n";

                if (almacen.tienePerecederos())
                {
                    ProductoPanaderia prox = almacen.consultarProximoAVencer();
                    fecha f = prox.getFechaVencimiento();
                    std::cout << "[!] Insumo MAS proximo a vencer (Cima del Min-Heap):\n"
                              << "    -> " << prox.getNombre() << " (SKU: " << prox.getSKU() << ")\n"
                              << "    -> Fecha de vencimiento: " << f.dia << "/" << f.mes << "/" << f.anio << "\n\n";
                }

                std::cout << " Opciones:\n";
                std::cout << " 1. Despachar el mas proximo a vencer (extraer del heap)\n";
                std::cout << " 2. Despachar TODOS los insumos en orden FEFO\n";
                std::cout << " 3. Regresar al menu principal\n";
                std::cout << " Seleccione una accion [1-3]: ";

                int subopc = 3;
                std::cin >> subopc;
                limpiarBuffer();

                if (subopc == 1)
                {
                    if (almacen.tienePerecederos())
                    {
                        ProductoPanaderia desp = almacen.despacharProximoAVencer();
                        fecha f = desp.getFechaVencimiento();
                        std::cout << "\n[+] DESPACHADO (FEFO): " << desp.getNombre() 
                                  << " (Vencimiento: " << f.dia << "/" << f.mes << "/" << f.anio << ")\n";
                    }
                    else
                    {
                        std::cout << "\n[!] No quedan insumos perecederos en el almacen.\n";
                    }
                }
                else if (subopc == 2)
                {
                    if (!almacen.tienePerecederos())
                    {
                        std::cout << "\n[!] El almacen esta vacio.\n";
                    }
                    else
                    {
                        std::cout << "\n--> Despachando todos los perecederos por orden FEFO:\n";
                        int num = 1;
                        while (almacen.tienePerecederos())
                        {
                            ProductoPanaderia desp = almacen.despacharProximoAVencer();
                            fecha f = desp.getFechaVencimiento();
                            std::cout << "  " << num++ << ". [" << f.dia << "/" << f.mes << "/" << f.anio << "] "
                                      << desp.getNombre() << " (SKU: " << desp.getSKU() << ")\n";
                        }
                    }
                }
                pausar();
                break;
            }

            case 4:
                std::cout << "\nSaliendo del Bakery System. Hasta luego!\n";
                break;

            default:
                std::cout << "[!] Opcion desconocida. Seleccione entre 1 y 4.\n";
                pausar();
                break;
        }
    }

    return 0;
}
