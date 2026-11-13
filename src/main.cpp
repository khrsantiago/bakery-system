#include <iostream>
#include <limits>
#include "SistemaPanaderia.hpp"

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

// Función auxiliar para mostrar los SKUs precargados como referencia rápida en consola
void mostrarSkusDisponibles()
{
    std::cout << " SKUs precargados:\n";
    std::cout << "   - PAN-JAM-001 : Pan de Jamon Tradicional\n";
    std::cout << "   - PAN-CAN-001 : Pan Canilla Venezolano\n";
    std::cout << "   - PAN-CAM-001 : Pan Campesino Masa Madre\n";
}

int main()
{
    // =========================================================================
    // CONFIGURACIÓN INICIAL DEL CATÁLOGO Y MÓDULOS DEL SISTEMA
    // =========================================================================
    SistemaPanaderia sistema;
    Designar::DynArray<ProductoPanaderia*> productosDinamicos;

    // 1. Producto y Receta 1: Pan de Jamón Tradicional
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
        {15, 11, 2026},
        12.50
    );

    Receta recetaPanJamon(panDeJamon);
    NodoReceta* raizJam = recetaPanJamon.buscarElemento("PAN-JAM-001");
    if (raizJam != nullptr)
    {
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("SUB-REL-001", 0.45, "kg"));
        NodoReceta* nodoRelleno = recetaPanJamon.buscarElemento("SUB-REL-001");
        if (nodoRelleno != nullptr)
        {
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-JAM-001", 0.70, "kg")); // Jamon
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-TOC-001", 0.15, "kg")); // Tocineta
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-PAS-001", 0.10, "kg")); // Pasitas
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-ACE-001", 0.05, "kg")); // Aceitunas
        }

        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-HAR-001", 0.55, "kg")); // Harina Panadera
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-MAR-001", 0.06, "kg")); // Margarina
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-AZU-001", 0.05, "kg")); // Azucar
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-AGU-001", 0.20, "kg")); // Agua / Liquidos
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-LEV-001", 0.02, "kg")); // Levadura Fresca
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-PAP-001", 0.05, "kg")); // Papelon
    }

    // 2. Producto y Receta 2: Pan Canilla Venezolano (con subproducto Prefermento)
    ProductoPanaderia panCanilla(
        "PAN-CAN-001",
        "Pan Canilla Venezolano",
        TipoMasaBase::CRUJIENTE_RUSTICA,
        62.0,
        90,
        28.0,
        22,
        210,
        0.88, // Rendimiento: 88% (merma del 12%)
        120,
        30,
        {14, 11, 2026},
        1.50
    );

    Receta recetaCanilla(panCanilla);
    NodoReceta* raizCan = recetaCanilla.buscarElemento("PAN-CAN-001");
    if (raizCan != nullptr)
    {
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("SUB-PRE-001", 0.30, "kg"));
        NodoReceta* nodoPrefermento = recetaCanilla.buscarElemento("SUB-PRE-001");
        if (nodoPrefermento != nullptr)
        {
            recetaCanilla.agregarElemento(nodoPrefermento, ElementoReceta("MP-HAR-001", 0.60, "kg")); // Harina en esponja
            recetaCanilla.agregarElemento(nodoPrefermento, ElementoReceta("MP-AGU-001", 0.38, "kg")); // Agua en esponja
            recetaCanilla.agregarElemento(nodoPrefermento, ElementoReceta("MP-LEV-001", 0.02, "kg")); // Levadura
        }

        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-HAR-001", 0.45, "kg")); // Harina directa
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-AGU-001", 0.20, "kg")); // Agua directa
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-SAL-001", 0.02, "kg")); // Sal
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-MAN-001", 0.03, "kg")); // Manteca vegetal
    }

    // 3. Producto y Receta 3: Pan Campesino Masa Madre
    ProductoPanaderia panCampesino(
        "PAN-CAM-001",
        "Pan Campesino Masa Madre",
        TipoMasaBase::MADRE_ACIDA,
        70.0,
        240,
        26.0,
        40,
        220,
        0.85, // Rendimiento: 85% (merma del 15%)
        35,
        10,
        {16, 11, 2026},
        3.50
    );

    Receta recetaCampesino(panCampesino);
    NodoReceta* raizCam = recetaCampesino.buscarElemento("PAN-CAM-001");
    if (raizCam != nullptr)
    {
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("SUB-MMA-001", 0.25, "kg"));
        NodoReceta* nodoMasaMadre = recetaCampesino.buscarElemento("SUB-MMA-001");
        if (nodoMasaMadre != nullptr)
        {
            recetaCampesino.agregarElemento(nodoMasaMadre, ElementoReceta("MP-HAR-001", 0.50, "kg")); // Harina en masa madre
            recetaCampesino.agregarElemento(nodoMasaMadre, ElementoReceta("MP-AGU-001", 0.50, "kg")); // Agua en masa madre
        }

        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-HAR-001", 0.50, "kg")); // Harina directa
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-AGU-001", 0.23, "kg")); // Agua directa
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-SAL-001", 0.02, "kg")); // Sal
    }

    // Registro en las Tablas Hash O(1) de SistemaPanaderia
    sistema.registrarProducto(&panDeJamon);
    sistema.registrarReceta(&recetaPanJamon);

    sistema.registrarProducto(&panCanilla);
    sistema.registrarReceta(&recetaCanilla);

    sistema.registrarProducto(&panCampesino);
    sistema.registrarReceta(&recetaCampesino);

    // 4. Lote de Producción con Trazabilidad (SLList)
    LoteProduccion lote1("LOT-20261106-001", panDeJamon.getSKU(), 10.0, {6, 11, 2026});
    lote1.registrarEvento("Pesaje", "Pesaje exacto de harina, liquidos y relleno", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Amasado", "Amasado durante 12 min hasta desarrollar gluten", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Armado y Sobado", "Laminado e incorporacion de jamon, tocineta y pasas", "Khristopher Santiago", {6, 11, 2026});
    lote1.registrarEvento("Fermentacion Final", "Camara a 30C durante 180 min", "Luis Acevedo", {6, 11, 2026});
    lote1.registrarEvento("Horneado", "Horneado a 180C durante 35 min", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Control de Calidad", "Inspeccion de corteza, peso y empaque", "Khristopher Santiago", {6, 11, 2026});

    // 5. Almacén de Perecederos (DynHeap / Min-Heap FEFO)
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
    // BUCLE DEL MENÚ INTERACTIVO (CLI - POS & PRODUCCIÓN)
    // =========================================================================
    int opcion = 0;
    while (opcion != 6)
    {
        std::cout << "\n==================================================\n";
        std::cout << "         BAKERY SYSTEM - MENU PRINCIPAL          \n";
        std::cout << "  Programacion III - ULA | Preparaduria / Lab    \n";
        std::cout << "  Autores: Luis Acevedo, Khristopher Santiago,   \n";
        std::cout << "           Juan Rincon                           \n";
        std::cout << "==================================================\n";
        std::cout << " 1. Busqueda Rapida en POS por SKU (Tabla Hash O(1))\n";
        std::cout << " 2. Orden de Produccion para Cocina (Hash O(1) -> Arbol DFS)\n";
        std::cout << " 3. Gestion de Catalogo Hash (Registrar / Eliminar O(1))\n";
        std::cout << " 4. Trazabilidad del Lote (Designar::SLList)\n";
        std::cout << " 5. Gestion de Perecederos FEFO (Min-Heap)\n";
        std::cout << " 6. Salir\n";
        std::cout << "==================================================\n";
        std::cout << "Seleccione una opcion [1-6]: ";

        if (!(std::cin >> opcion))
        {
            std::cin.clear();
            limpiarBuffer();
            std::cout << "[!] Entrada invalida. Ingrese un numero del 1 al 6.\n";
            continue;
        }
        limpiarBuffer();

        switch (opcion)
        {
            case 1:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 1. CONSULTA RAPIDA EN POS POR SKU (Hash O(1))\n";
                std::cout << "--------------------------------------------------\n";
                mostrarSkusDisponibles();

                std::string sku;
                std::cout << "\nIngrese el SKU a consultar [Por defecto: PAN-JAM-001]: ";
                std::getline(std::cin, sku);
                if (sku.empty())
                {
                    sku = "PAN-JAM-001";
                }

                ProductoPanaderia* productoEncontrado = sistema.buscarProducto(sku);
                if (productoEncontrado != nullptr)
                {
                    std::cout << "\n[+] Producto localizado en O(1):\n";
                    productoEncontrado->mostrarInfo();
                }
                else
                {
                    std::cout << "\n[!] No existe ningun producto registrado con el SKU: " << sku << "\n";
                }
                pausar();
                break;
            }

            case 2:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 2. ORDEN DE PRODUCCION (Hash O(1) -> DFS O(n))\n";
                std::cout << "--------------------------------------------------\n";
                mostrarSkusDisponibles();

                std::string sku;
                std::cout << "\nIngrese el SKU del producto a producir [Por defecto: PAN-JAM-001]: ";
                std::getline(std::cin, sku);
                if (sku.empty())
                {
                    sku = "PAN-JAM-001";
                }

                double cantidadDeseada = 10.0;
                std::cout << "Ingrese la cantidad a producir en kg [Por defecto: 10.0]: ";
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

                sistema.generarReporteCocina(sku, cantidadDeseada);
                pausar();
                break;
            }

            case 3:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 3. GESTION DE CATALOGO HASH (Tabla Hash O(1))\n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Registrar nuevo producto en Tabla Hash\n";
                std::cout << " 2. Eliminar producto del catalogo por SKU\n";
                std::cout << " 3. Regresar al menu principal\n";
                std::cout << " Seleccione una accion [1-3]: ";

                int subopc = 3;
                if (!(std::cin >> subopc))
                {
                    std::cin.clear();
                    limpiarBuffer();
                    std::cout << "[!] Entrada invalida.\n";
                    pausar();
                    break;
                }
                limpiarBuffer();

                if (subopc == 1)
                {
                    std::string sku, nombre;
                    int stockAct = 0;
                    int stockMin = 0;
                    double costo = 0.0;

                    std::cout << "\n Ingrese el SKU (ej. PAN-GOL-001): ";
                    std::getline(std::cin, sku);
                    std::cout << " Ingrese el nombre del producto: ";
                    std::getline(std::cin, nombre);
                    std::cout << " Stock inicial (unidades): ";
                    std::cin >> stockAct;
                    std::cout << " Stock minimo (unidades): ";
                    std::cin >> stockMin;
                    std::cout << " Costo unitario ($): ";
                    std::cin >> costo;
                    limpiarBuffer();

                    ProductoPanaderia* nuevo = new ProductoPanaderia(
                        sku,
                        nombre,
                        TipoMasaBase::BLANDA_ENRIQUECIDA,
                        55.0,
                        120,
                        28.0,
                        25,
                        180,
                        0.90,
                        stockAct,
                        stockMin,
                        {20, 11, 2026},
                        costo
                    );

                    if (sistema.registrarProducto(nuevo))
                    {
                        productosDinamicos.append(nuevo);
                        std::cout << "\n[+] Producto '" << nombre << "' (" << sku
                                  << ") insertado en la Tabla Hash en O(1).\n";
                    }
                    else
                    {
                        delete nuevo;
                        std::cout << "\n[!] No se pudo registrar: SKU vacio o duplicado.\n";
                    }
                }
                else if (subopc == 2)
                {
                    std::string sku;
                    std::cout << "\n Ingrese el SKU a eliminar: ";
                    std::getline(std::cin, sku);

                    if (sistema.eliminarProducto(sku))
                    {
                        std::cout << "\n[+] Producto con SKU '" << sku << "' eliminado de la Tabla Hash.\n";
                    }
                    else
                    {
                        std::cout << "\n[!] No se encontro ningun producto con el SKU '" << sku << "'.\n";
                    }
                }
                pausar();
                break;
            }

            case 4:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 4. TRAZABILIDAD DEL LOTE DE PRODUCCION (SLList)  \n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Mostrar historial cronologico completo\n";
                std::cout << " 2. Registrar nuevo evento de auditoria\n";
                std::cout << " Seleccione una accion [1-2]: ";

                int subopc = 1;
                if (!(std::cin >> subopc))
                {
                    std::cin.clear();
                    limpiarBuffer();
                    subopc = 1;
                }
                else
                {
                    limpiarBuffer();
                }

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

            case 5:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 5. GESTION DE PERECEDEROS (Min-Heap / FEFO)      \n";
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
                if (!(std::cin >> subopc))
                {
                    std::cin.clear();
                    limpiarBuffer();
                    subopc = 3;
                }
                else
                {
                    limpiarBuffer();
                }

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

            case 6:
                std::cout << "\nSaliendo del Bakery System. Hasta luego!\n";
                break;

            default:
                std::cout << "[!] Opcion desconocida. Seleccione entre 1 y 6.\n";
                pausar();
                break;
        }
    }

    // Liberar los productos creados dinámicamente desde la CLI
    for (size_t i = 0; i < productosDinamicos.size(); ++i)
    {
        delete productosDinamicos[i];
    }

    return 0;
}
