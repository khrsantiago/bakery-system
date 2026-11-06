#include <iostream>
#include "ProductoPanaderia.hpp"
#include "receta.hpp"
#include "LoteProduccion.hpp"
#include "GestionPerecederos.hpp"

int main()
{
    std::cout << "==================================================\n";
    std::cout << "   PRUEBA DE EXPLOSION DE RECETA Y MATERIAS PRIMAS\n";
    std::cout << "==================================================\n\n";

    // 1. Instanciación del Producto Panadería Principal (Pan de Jamón Tradicional)
    fecha fechaVenc = {15, 11, 2026};
    ProductoPanaderia panDeJamon(
        "PAN-JAM-001",           // SKU
        "Pan de Jamon Tradicional", // Nombre adaptado a la panadería venezolana
        TipoMasaBase::DULCE_SOBADA,
        //"Masa Dulce Sobada",     // Tipo de Masa (Muy común en Venezuela)
        55.0,                    // % Hidratación
        180,                     // Tiempo Fermentación (min)
        30.0,                    // Temp Fermentación (°C) -> Temperatura ambiente promedio en Venezuela
        35,                      // Tiempo Horneado (min)
        180,                     // Temp Horneado (°C)
        0.90,                    // Rendimiento de Masa (90%)
        50,                      // Stock Actual
        10,                      // Stock Mínimo
        fechaVenc,               // Fecha de Vencimiento
        12.50                    // Costo Unitario (Estimación referencial)
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
    // Se cambia "Harina de Fuerza" por Harina Todo Uso/Panadera común y se añaden ingredientes de masa dulce criolla
    receta.agregarElemento(raiz, ElementoReceta("MP-HAR-001", 0.55, "kg")); // Harina de Trigo Panadera (o Todo Uso)
    receta.agregarElemento(raiz, ElementoReceta("MP-MAR-001", 0.06, "kg")); // Margarina con sal (más común que la mantequilla)
    receta.agregarElemento(raiz, ElementoReceta("MP-AZU-001", 0.05, "kg")); // Azúcar Blanquilla
    receta.agregarElemento(raiz, ElementoReceta("MP-AGU-001", 0.20, "kg")); // Agua fría o Leche líquida
    receta.agregarElemento(raiz, ElementoReceta("MP-LEV-001", 0.02, "kg")); // Levadura Fresca (en pasta)
    receta.agregarElemento(raiz, ElementoReceta("MP-PAP-001", 0.05, "kg")); // Papelón (para el melao de la corteza)

    std::cout << "[+] Arbol jerárquico ensamblado correctamente con subproductos e insumos.\n\n";

    // 4. Ejecución del Algoritmo de Explosión de Receta (DFS)
    double ordenProduccion = 10.0; // Se solicitan 10 kg de Pan de Jamón (Aprox. 10 a 12 panes medianos)
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
    std::cout << "==================================================\n";

    return 0;
}
