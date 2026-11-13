#include <iostream>
#include "SistemaPanaderia.hpp"

int main()
{
    std::cout << "==================================================\n";
    std::cout << "   PRUEBA DE EXPLOSION DE RECETA Y MATERIAS PRIMAS\n";
    std::cout << "==================================================\n\n";

    // 1. Instanciación del Producto Panadería Principal (Pan de Jamón Tradicional)
    fecha fechaVenc = {15, 11, 2026};
    ProductoPanaderia panDeJamon(
        "PAN-JAM-001",              // SKU
        "Pan de Jamon Tradicional", // Nombre adaptado a la panadería venezolana
        TipoMasaBase::DULCE_SOBADA, // Masa Dulce Sobada
        55.0,                       // % Hidratación
        180,                        // Tiempo Fermentación (min)
        30.0,                       // Temp Fermentación (°C)
        35,                         // Tiempo Horneado (min)
        180,                        // Temp Horneado (°C)
        0.90,                       // Rendimiento de Masa (90%)
        50,                         // Stock Actual
        10,                         // Stock Mínimo
        fechaVenc,                  // Fecha de Vencimiento
        12.50                       // Costo Unitario
    );

    // 2. Creación de la Receta (El constructor asigna el producto como Nodo Raíz)
    Receta receta(panDeJamon);

    // Obtener la raíz a partir de su SKU para comenzar a agregar componentes
    NodoReceta* raiz = receta.buscarElemento("PAN-JAM-001");
    if (raiz == nullptr)
    {
        std::cerr << "[ERROR] No se pudo localizar la raíz de la receta.\n";
        return 1;
    }

    std::cout << "[+] Receta creada con exito para: " << panDeJamon.getNombre() << "\n\n";

    // 3. Construcción del Árbol de la Receta (Subproductos/Rellenos e Ingredientes)

    // A. Agregar Subproducto Intermedio: El Relleno del Pan de Jamón (0.45 kg por kg de pan)
    receta.agregarElemento(raiz, ElementoReceta("SUB-REL-001", 0.45, "kg"));

    // B. Buscar el nodo del Relleno para insertarle sus componentes específicos (Hojas)
    NodoReceta* nodoRelleno = receta.buscarElemento("SUB-REL-001");
    if (nodoRelleno != nullptr)
    {
        // Ingredientes típicos del relleno del pan de jamón venezolano:
        receta.agregarElemento(nodoRelleno, ElementoReceta("MP-JAM-001", 0.70, "kg")); // Jamón Ahumado
        receta.agregarElemento(nodoRelleno, ElementoReceta("MP-TOC-001", 0.15, "kg")); // Tocinetas
        receta.agregarElemento(nodoRelleno, ElementoReceta("MP-PAS-001", 0.10, "kg")); // Pasitas
        receta.agregarElemento(nodoRelleno, ElementoReceta("MP-ACE-001", 0.05, "kg")); // Aceitunas Rellenas
    }

    // C. Agregar los ingredientes directos a la masa del Pan de Jamón (Nodo Raíz):
    receta.agregarElemento(raiz, ElementoReceta("MP-HAR-001", 0.55, "kg")); // Harina Panadera
    receta.agregarElemento(raiz, ElementoReceta("MP-MAR-001", 0.06, "kg")); // Margarina con sal
    receta.agregarElemento(raiz, ElementoReceta("MP-AZU-001", 0.05, "kg")); // Azúcar Blanquilla
    receta.agregarElemento(raiz, ElementoReceta("MP-AGU-001", 0.20, "kg")); // Agua fría o Leche
    receta.agregarElemento(raiz, ElementoReceta("MP-LEV-001", 0.02, "kg")); // Levadura Fresca
    receta.agregarElemento(raiz, ElementoReceta("MP-PAP-001", 0.05, "kg")); // Papelón (melao)

    std::cout << "[+] Arbol jerárquico ensamblado correctamente con subproductos e insumos.\n\n";

    // 4. Ejecución del Algoritmo de Explosión de Receta (DFS)
    double ordenProduccion = 10.0; // Se solicitan 10 kg de Pan de Jamón
    Designar::DynArray<ElementoReceta> insumosTotales;

    std::cout << "--> Ejecutando explosión de receta para un lote de " 
              << ordenProduccion << " kg...\n\n";

    receta.calcularMateriasPrimas(ordenProduccion, insumosTotales);

    // 5. Despliegue de los Resultados Acumulados
    std::cout << "==================================================\n";
    std::cout << "       REPORTE FINAL DE MATERIAS PRIMAS          \n";
    std::cout << "==================================================\n";

    for (size_t i = 0; i < insumosTotales.size(); ++i)
    {
        std::cout << " Insumo [" << (i + 1) << "]: ";
        insumosTotales[i].mostrarInfo();
        std::cout << "\n";
    }

    std::cout << "==================================================\n";
    std::cout << " Total de tipos de insumos elementales: " << insumosTotales.size() << "\n";
    std::cout << "==================================================\n\n";

    // 6. Prueba de Trazabilidad de Lotes (Designar::SLList)
    std::cout << "==================================================\n";
    std::cout << "      PRUEBA DE TRAZABILIDAD POR LOTE (SLList)    \n";
    std::cout << "==================================================\n\n";

    LoteProduccion lote1("LOT-20261106-001", panDeJamon.getSKU(), ordenProduccion, {6, 11, 2026});

    lote1.registrarEvento("Pesaje", "Pesaje exacto de harina, liquidos y relleno", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Amasado", "Amasado durante 12 min hasta desarrollar gluten", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Armado y Sobado", "Laminado e incorporacion de jamon, tocineta y pasas", "Khristopher Santiago", {6, 11, 2026});
    lote1.registrarEvento("Fermentacion Final", "Camara a 30C durante 180 min", "Luis Acevedo", {6, 11, 2026});
    lote1.registrarEvento("Horneado", "Horneado a 180C durante 35 min", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Control de Calidad", "Inspeccion de corteza, peso y empaque", "Khristopher Santiago", {6, 11, 2026});

    lote1.mostrarHistorialTrazabilidad();

    // 7. Prueba de Gestión de Perecederos (Designar::DynHeap / Min-Heap)
    std::cout << "==================================================\n";
    std::cout << "  PRUEBA DE GESTION FEFO DE PERECEDEROS (Min-Heap)\n";
    std::cout << "==================================================\n\n";

    GestionPerecederos almacen;

    // Se ingresan insumos con fechas de vencimiento desordenadas
    ProductoPanaderia levadura("MP-LEV-001", "Levadura Fresca en Pasta", TipoMasaBase::BLANDA_ENRIQUECIDA,
                               0, 0, 0, 0, 0, 1.0, 20, 5, {12, 11, 2026}, 1.20);

    ProductoPanaderia jamon("MP-JAM-001", "Jamon Ahumado Especial", TipoMasaBase::BLANDA_ENRIQUECIDA,
                            0, 0, 0, 0, 0, 1.0, 50, 10, {20, 11, 2026}, 8.50);

    ProductoPanaderia leche("MP-LEC-001", "Leche Pasteurizada Completa", TipoMasaBase::BLANDA_ENRIQUECIDA,
                            0, 0, 0, 0, 0, 1.0, 30, 5, {8, 11, 2026}, 1.80); // Esta vence primero!

    ProductoPanaderia margarina("MP-MAR-001", "Margarina con Sal", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                0, 0, 0, 0, 0, 1.0, 40, 10, {25, 12, 2026}, 3.00);

    almacen.registrarPerecedero(levadura);
    almacen.registrarPerecedero(jamon);
    almacen.registrarPerecedero(leche);
    almacen.registrarPerecedero(margarina);

    std::cout << "[+] Insumos registrados en el Min-Heap: " << almacen.totalPerecederos() << "\n";
    std::cout << "[+] Proximo a vencer en el top(): " 
              << almacen.consultarProximoAVencer().getNombre() << " (Vence: "
              << almacen.consultarProximoAVencer().getFechaVencimiento().dia << "/"
              << almacen.consultarProximoAVencer().getFechaVencimiento().mes << "/"
              << almacen.consultarProximoAVencer().getFechaVencimiento().anio << ")\n\n";

    std::cout << "--> Despachando insumos segun vencimiento (orden FEFO):\n";
    int ordenDespacho = 1;
    while (almacen.tienePerecederos())
    {
        ProductoPanaderia despachado = almacen.despacharProximoAVencer();
        fecha f = despachado.getFechaVencimiento();
        std::cout << "  " << ordenDespacho++ << ". [" << f.dia << "/" << f.mes << "/" << f.anio << "] "
                  << despachado.getNombre() << " (SKU: " << despachado.getSKU() << ")\n";
    }
    std::cout << "==================================================\n\n";

    // 8. Prueba de Tabla Hash por SKU y Flujo Completo POS -> Cocina (Semana 12)
    std::cout << "==================================================\n";
    std::cout << "  PRUEBA DE TABLA HASH SKU Y FLUJO POS (Semana 12)\n";
    std::cout << "==================================================\n\n";

    SistemaPanaderia sistema;

    ProductoPanaderia panCanilla(
        "PAN-CAN-001",
        "Pan Canilla Venezolano",
        TipoMasaBase::CRUJIENTE_RUSTICA,
        62.0, 90, 28.0, 22, 210, 0.88,
        120, 30, {14, 11, 2026}, 1.50
    );

    Receta recetaCanilla(panCanilla);
    NodoReceta* raizCanilla = recetaCanilla.buscarElemento("PAN-CAN-001");
    if (raizCanilla != nullptr)
    {
        recetaCanilla.agregarElemento(raizCanilla, ElementoReceta("SUB-PRE-001", 0.30, "kg"));
        NodoReceta* nodoPref = recetaCanilla.buscarElemento("SUB-PRE-001");
        if (nodoPref != nullptr)
        {
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-HAR-001", 0.60, "kg"));
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-AGU-001", 0.38, "kg"));
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-LEV-001", 0.02, "kg"));
        }
        recetaCanilla.agregarElemento(raizCanilla, ElementoReceta("MP-HAR-001", 0.45, "kg"));
        recetaCanilla.agregarElemento(raizCanilla, ElementoReceta("MP-AGU-001", 0.20, "kg"));
        recetaCanilla.agregarElemento(raizCanilla, ElementoReceta("MP-SAL-001", 0.02, "kg"));
        recetaCanilla.agregarElemento(raizCanilla, ElementoReceta("MP-MAN-001", 0.03, "kg"));
    }

    bool regProd1 = sistema.registrarProducto(&panDeJamon);
    bool regRec1  = sistema.registrarReceta(&receta);
    bool regProd2 = sistema.registrarProducto(&panCanilla);
    bool regRec2  = sistema.registrarReceta(&recetaCanilla);

    std::cout << "[+] Registro en Tabla Hash (esperado true): "
              << (regProd1 && regRec1 && regProd2 && regRec2 ? "OK" : "FALLO") << "\n";

    bool regDuplicado = sistema.registrarProducto(&panDeJamon);
    std::cout << "[+] Rechazo de SKU duplicado (esperado false): "
              << (!regDuplicado ? "OK" : "FALLO") << "\n";

    ProductoPanaderia* encontrado = sistema.buscarProducto("PAN-JAM-001");
    if (encontrado != nullptr)
    {
        std::cout << "[+] Busqueda O(1) exitosa: " << encontrado->getNombre()
                  << " (SKU: " << encontrado->getSKU() << ")\n";
    }
    else
    {
        std::cerr << "[ERROR] No se encontro PAN-JAM-001 en la Tabla Hash.\n";
        return 1;
    }

    ProductoPanaderia* noEncontrado = sistema.buscarProducto("SKU-NO-EXISTE");
    std::cout << "[+] Busqueda O(1) de SKU inexistente (esperado nullptr): "
              << (noEncontrado == nullptr ? "OK" : "FALLO") << "\n";

    std::cout << "\n--> Ejecutando flujo acoplado Hash O(1) -> Explosion DFS O(n) para PAN-CAN-001:\n";
    sistema.generarReporteCocina("PAN-CAN-001", 10.0);

    bool eliminado = sistema.eliminarProducto("PAN-CAN-001");
    bool yaNoEsta = (sistema.buscarProducto("PAN-CAN-001") == nullptr);
    std::cout << "[+] Eliminacion O(1) de PAN-CAN-001: "
              << (eliminado && yaNoEsta ? "OK" : "FALLO") << "\n";
    std::cout << "==================================================\n";

    return 0;
}

