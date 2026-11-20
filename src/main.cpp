#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include "SistemaPanaderia.hpp"

namespace
{
    const std::string RUTA_PERSISTENCIA_JSON = "data/bakery_data.json";

    void pausar()
    {
        std::cout << "\nPresione ENTER para continuar...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string recortarEspacios(const std::string& texto)
    {
        size_t inicio = texto.find_first_not_of(" \t\r\n");
        if (inicio == std::string::npos)
        {
            return "";
        }
        size_t fin = texto.find_last_not_of(" \t\r\n");
        return texto.substr(inicio, fin - inicio + 1);
    }

    std::string leerLinea(const std::string& prompt, const std::string& porDefecto = "")
    {
        std::cout << prompt;
        std::string linea;
        std::getline(std::cin, linea);
        linea = recortarEspacios(linea);
        return linea.empty() ? porDefecto : linea;
    }

    int leerEntero(const std::string& prompt, int porDefecto)
    {
        std::cout << prompt;
        std::string linea;
        std::getline(std::cin, linea);
        linea = recortarEspacios(linea);
        if (linea.empty())
        {
            return porDefecto;
        }
        try
        {
            size_t idx = 0;
            int val = std::stoi(linea, &idx);
            if (idx == linea.size())
            {
                return val;
            }
        }
        catch (...)
        {
        }
        std::cout << "[!] Entrada no valida. Se usara " << porDefecto << " por defecto.\n";
        return porDefecto;
    }

    double leerDouble(const std::string& prompt, double porDefecto)
    {
        std::cout << prompt;
        std::string linea;
        std::getline(std::cin, linea);
        linea = recortarEspacios(linea);
        if (linea.empty())
        {
            return porDefecto;
        }
        try
        {
            size_t idx = 0;
            double val = std::stod(linea, &idx);
            if (idx == linea.size())
            {
                return val;
            }
        }
        catch (...)
        {
        }
        std::cout << "[!] Entrada no valida. Se usara " << porDefecto << " por defecto.\n";
        return porDefecto;
    }

    fecha leerFecha(const std::string& prompt, const fecha& porDefecto)
    {
        std::cout << prompt;
        std::string linea;
        std::getline(std::cin, linea);
        linea = recortarEspacios(linea);
        if (linea.empty())
        {
            return porDefecto;
        }
        std::istringstream iss(linea);
        int d = 0, m = 0, a = 0;
        if (iss >> d >> m >> a && d >= 1 && d <= 31 && m >= 1 && m <= 12 && a >= 1900)
        {
            return {d, m, a};
        }
        std::cout << "[!] Fecha no valida. Se usara "
                  << porDefecto.dia << "/" << porDefecto.mes << "/" << porDefecto.anio << ".\n";
        return porDefecto;
    }

    bool confirmarSiNo(const std::string& prompt)
    {
        std::string resp = leerLinea(prompt, "n");
        return !resp.empty() && (resp[0] == 's' || resp[0] == 'S');
    }

    bool tieneNombreRegistrado(const SistemaPanaderia& sistema, const std::string& sku)
    {
        return sistema.existeProducto(sku) ||
               sistema.existeReceta(sku) ||
               sistema.obtenerCatalogoNombresSKU().has(sku);
    }

    void asegurarNombreSKU(SistemaPanaderia& sistema, const std::string& sku)
    {
        if (sku.empty() || tieneNombreRegistrado(sistema, sku))
        {
            return;
        }
        std::string nombre = leerLinea(" El SKU '" + sku + "' es nuevo. Ingrese su nombre descriptivo: ");
        if (!nombre.empty())
        {
            sistema.registrarNombreSKU(sku, nombre);
        }
    }

    void mostrarInsumosConocidos(const SistemaPanaderia& sistema)
    {
        std::cout << "\n Insumos y subproductos registrados en el sistema (SKU -> Nombre):\n";
        const auto& diccionario = sistema.obtenerCatalogoNombresSKU();
        size_t cont = 0;
        for (const auto& par : diccionario)
        {
            if (!sistema.existeProducto(par.first))
            {
                std::cout << "   - " << std::left << std::setw(12) << par.first << " : " << par.second << "\n";
                ++cont;
            }
        }
        if (cont == 0)
        {
            std::cout << "   (Sin insumos adicionales registrados)\n";
        }
    }

    ProductoPanaderia* mostrarYSeleccionarProductoPaginado(SistemaPanaderia& sistema,
                                                           bool soloExplorar = false)
    {
        const Designar::DynArray<ProductoPanaderia*>& productos = sistema.obtenerListaProductos();
        if (productos.is_empty())
        {
            std::cout << "\n[!] No hay productos registrados en el catalogo.\n";
            return nullptr;
        }

        const size_t POR_PAGINA = 5;
        const size_t total = productos.size();
        const size_t totalPaginas = (total + POR_PAGINA - 1) / POR_PAGINA;
        size_t paginaActual = 0;

        while (true)
        {
            size_t inicio = paginaActual * POR_PAGINA;
            size_t fin = inicio + POR_PAGINA;
            if (fin > total)
            {
                fin = total;
            }

            std::cout << "\n--------------------------------------------------------------------------\n";
            std::cout << " CATALOGO DE PRODUCTOS (Pagina " << (paginaActual + 1)
                      << " de " << totalPaginas << " | Total: " << total << ")\n";
            std::cout << "--------------------------------------------------------------------------\n";
            for (size_t i = inicio; i < fin; ++i)
            {
                const ProductoPanaderia* p = productos[i];
                if (p == nullptr)
                {
                    continue;
                }
                std::string tieneReceta = sistema.existeReceta(p->getSKU()) ? "SI" : "NO";
                std::cout << "  [" << (i + 1) << "] "
                          << p->getSKU() << " | " << p->getNombre()
                          << " | Stock: " << p->getStockActual()
                          << " | $" << std::fixed << std::setprecision(2) << p->getCostoUnitario()
                          << " | Receta: " << tieneReceta << "\n";
            }
            std::cout << std::defaultfloat << std::setprecision(6);
            std::cout << "--------------------------------------------------------------------------\n";
            std::cout << " Comandos: [n] Pag. siguiente | [p] Pag. anterior | [0] Cancelar\n";

            std::string prompt = soloExplorar
                                     ? " Ingrese comando (n/p/0) o numero/SKU para ver detalle: "
                                     : " Seleccione por numero [1-" + std::to_string(total) +
                                           "] o SKU (ENTER = #" + std::to_string(inicio + 1) + "): ";

            std::string entrada = leerLinea(prompt);

            if (entrada == "n" || entrada == "N")
            {
                if (paginaActual + 1 < totalPaginas)
                {
                    ++paginaActual;
                }
                else
                {
                    std::cout << " [i] Ya se encuentra en la ultima pagina.\n";
                }
                continue;
            }
            if (entrada == "p" || entrada == "P")
            {
                if (paginaActual > 0)
                {
                    --paginaActual;
                }
                else
                {
                    std::cout << " [i] Ya se encuentra en la primera pagina.\n";
                }
                continue;
            }
            if (entrada == "0" || entrada == "q" || entrada == "Q")
            {
                return nullptr;
            }
            if (entrada.empty())
            {
                return soloExplorar ? nullptr : productos[inicio];
            }

            // Intentar interpretar como número de índice [1..total]
            bool esNumero = true;
            for (char c : entrada)
            {
                if (!std::isdigit(static_cast<unsigned char>(c)))
                {
                    esNumero = false;
                    break;
                }
            }

            if (esNumero)
            {
                try
                {
                    size_t num = static_cast<size_t>(std::stoul(entrada));
                    if (num >= 1 && num <= total)
                    {
                        return productos[num - 1];
                    }
                    std::cout << " [!] Numero fuera de rango [1-" << total << "].\n";
                    continue;
                }
                catch (...)
                {
                }
            }

            // Intentar buscar por SKU en O(1)
            ProductoPanaderia* encontrado = sistema.buscarProducto(entrada);
            if (encontrado != nullptr)
            {
                return encontrado;
            }
            std::cout << " [!] No se encontro ningun producto con numero o SKU '" << entrada << "'.\n";
        }
    }

    bool construirYRegistrarReceta(SistemaPanaderia& sistema, ProductoPanaderia* producto)
    {
        if (producto == nullptr)
        {
            return false;
        }

        if (sistema.existeReceta(producto->getSKU()))
        {
            std::cout << "\n[!] El producto '" << producto->getNombre() << "' ("
                      << producto->getSKU() << ") ya tiene una receta registrada.\n";
            if (!confirmarSiNo(" ¿Desea reemplazar la receta existente? (s/n): "))
            {
                return false;
            }
        }

        Receta* nuevaReceta = new Receta(*producto);
        NodoReceta* raiz = nuevaReceta->getRaiz();
        int componentesAgregados = 0;

        std::cout << "\n==================================================\n";
        std::cout << " CREACION DE RECETA PARA: " << producto->getNombre()
                  << " (" << producto->getSKU() << ")\n";
        std::cout << "==================================================\n";
        mostrarInsumosConocidos(sistema);

        bool editando = true;
        while (editando)
        {
            std::cout << "\n Acciones de armado de receta:\n";
            std::cout << "  1. Agregar ingrediente directo a la masa principal\n";
            std::cout << "  2. Agregar subproducto (ej. relleno/prefermento) con ingredientes\n";
            std::cout << "  3. Ver arbol actual de la receta\n";
            std::cout << "  4. Guardar receta y finalizar\n";
            std::cout << "  5. Cancelar\n";

            int acc = leerEntero(" Seleccione una opcion [1-5]: ", 4);
            if (acc == 1)
            {
                std::string skuIng = leerLinea("   SKU del ingrediente (ej. MP-HAR-001): ");
                if (skuIng.empty())
                {
                    std::cout << "   [!] El SKU del ingrediente no puede estar vacio.\n";
                    continue;
                }
                asegurarNombreSKU(sistema, skuIng);
                double cant = leerDouble("   Cantidad por kg de producto (ej. 0.55): ", 0.10);
                std::string um = leerLinea("   Unidad de medida [kg]: ", "kg");

                if (cant <= 0.0)
                {
                    std::cout << "   [!] La cantidad debe ser mayor que 0.\n";
                    continue;
                }
                nuevaReceta->agregarElemento(raiz, ElementoReceta(skuIng, cant, um));
                ++componentesAgregados;
                std::cout << "   [+] Agregado: " << skuIng << " ("
                          << sistema.obtenerNombreSKU(skuIng) << ") = " << cant << " " << um << "\n";
            }
            else if (acc == 2)
            {
                std::string skuSub = leerLinea("   SKU del subproducto (ej. SUB-REL-002): ");
                if (skuSub.empty())
                {
                    std::cout << "   [!] El SKU del subproducto no puede estar vacio.\n";
                    continue;
                }
                asegurarNombreSKU(sistema, skuSub);
                double cantSub = leerDouble("   Cantidad del subproducto por kg de producto (ej. 0.30): ", 0.25);
                double rendSub = leerDouble("   Factor de rendimiento del subproducto [1.0]: ", 1.0);
                std::string umSub = leerLinea("   Unidad de medida [kg]: ", "kg");

                if (cantSub <= 0.0)
                {
                    std::cout << "   [!] La cantidad debe ser mayor que 0.\n";
                    continue;
                }

                nuevaReceta->agregarElemento(raiz, ElementoReceta(skuSub, cantSub, umSub, rendSub));
                NodoReceta* nodoSub = nuevaReceta->buscarElemento(skuSub);
                if (nodoSub != nullptr)
                {
                    while (true)
                    {
                        std::string skuHijo = leerLinea("     SKU ingrediente del subproducto (ENTER para terminar): ");
                        if (skuHijo.empty())
                        {
                            break;
                        }
                        asegurarNombreSKU(sistema, skuHijo);
                        double cantHijo = leerDouble("     Cantidad por kg de subproducto: ", 0.10);
                        std::string umHijo = leerLinea("     Unidad de medida [kg]: ", "kg");
                        if (cantHijo > 0.0)
                        {
                            nuevaReceta->agregarElemento(nodoSub, ElementoReceta(skuHijo, cantHijo, umHijo));
                            ++componentesAgregados;
                            std::cout << "     [+] Sub-ingrediente agregado: " << skuHijo
                                      << " (" << sistema.obtenerNombreSKU(skuHijo) << ")\n";
                        }
                    }
                }
                ++componentesAgregados;
            }
            else if (acc == 3)
            {
                std::cout << "\n Vista previa de '" << producto->getNombre() << "':\n";
                Designar::DynArray<ElementoReceta> preview;
                nuevaReceta->calcularMateriasPrimas(1.0, preview);
                if (preview.is_empty())
                {
                    std::cout << "  (Sin ingredientes agregados aun)\n";
                }
                else
                {
                    for (size_t i = 0; i < preview.size(); ++i)
                    {
                        std::cout << "  [" << (i + 1) << "] " << preview[i].getCodigoSKU()
                                  << " (" << sistema.obtenerNombreSKU(preview[i].getCodigoSKU()) << ") : "
                                  << preview[i].getCantidad() << " " << preview[i].getUnidadMedida() << "\n";
                    }
                }
            }
            else if (acc == 4)
            {
                if (componentesAgregados == 0)
                {
                    std::cout << " [!] Agregue al menos un ingrediente antes de guardar la receta.\n";
                    continue;
                }
                if (sistema.existeReceta(producto->getSKU()))
                {
                    sistema.eliminarReceta(producto->getSKU());
                }
                if (sistema.registrarReceta(nuevaReceta, true))
                {
                    std::cout << "\n[+] Receta registrada exitosamente para " << producto->getNombre()
                              << " (" << producto->getSKU() << ").\n";
                    sistema.mostrarArbolReceta(producto->getSKU());
                    return true;
                }
                delete nuevaReceta;
                std::cout << "\n[!] Error al registrar la receta.\n";
                return false;
            }
            else if (acc == 5)
            {
                delete nuevaReceta;
                std::cout << " [i] Creacion de receta cancelada.\n";
                return false;
            }
        }

        delete nuevaReceta;
        return false;
    }

    TipoMasaBase leerTipoMasa()
    {
        std::cout << "   Tipos de masa disponibles:\n";
        std::cout << "     1. Masa Dulce Sobada\n";
        std::cout << "     2. Masa Hojaldrada\n";
        std::cout << "     3. Masa Madre Acida\n";
        std::cout << "     4. Masa Blanda Enriquecida\n";
        std::cout << "     5. Masa Crujiente Rustica\n";
        std::cout << "     6. Masa Integral\n";
        int op = leerEntero("   Seleccione tipo de masa [1-6, por defecto 4]: ", 4);
        if (op < 1 || op > 6)
        {
            op = 4;
        }
        return static_cast<TipoMasaBase>(op - 1);
    }
} // namespace

int main()
{
    SistemaPanaderia sistema;
    sistema.inicializarConPersistencia(RUTA_PERSISTENCIA_JSON);

    int opcion = 0;
    while (opcion != 7)
    {
        std::cout << "\n==================================================\n";
        std::cout << "         BAKERY SYSTEM - MENU PRINCIPAL          \n";
        std::cout << "  Programacion III - ULA | Preparaduria / Lab    \n";
        std::cout << "  Autores: Luis Acevedo, Khristopher Santiago,   \n";
        std::cout << "           Juan Rincon                           \n";
        std::cout << "==================================================\n";
        std::cout << " 1. Punto de Venta (POS) y Consulta Rapida (Hash O(1))\n";
        std::cout << " 2. Orden de Produccion para Cocina (Hash -> DFS -> Kardex AVL)\n";
        std::cout << " 3. Gestion de Catalogo y Recetas (Validacion SKU + Arbol)\n";
        std::cout << " 4. Trazabilidad de Lotes (Designar::SLList)\n";
        std::cout << " 5. Gestion de Perecederos FEFO (Min-Heap)\n";
        std::cout << " 6. Kardex de Inventario (Consultas / Movimientos en Arbol AVL)\n";
        std::cout << " 7. Salir\n";
        std::cout << "==================================================\n";

        opcion = leerEntero("Seleccione una opcion [1-7]: ", 0);
        if (opcion < 1 || opcion > 7)
        {
            std::cout << "[!] Entrada invalida. Ingrese un numero del 1 al 7.\n";
            continue;
        }

        switch (opcion)
        {
            case 1:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 1. PUNTO DE VENTA (POS) Y CONSULTA EN O(1)\n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Registrar Venta en POS (Descuenta Stock + Kardex AVL)\n";
                std::cout << " 2. Consultar ficha tecnica y receta por numero o SKU\n";
                std::cout << " 3. Explorar catalogo paginado de productos\n";
                std::cout << " 4. Regresar al menu principal\n";
                int subopc = leerEntero(" Seleccione una accion [1-4]: ", 4);

                if (subopc == 1)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr)
                    {
                        std::cout << "\n Producto seleccionado: " << prod->getNombre()
                                  << " (" << prod->getSKU() << ")\n";
                        std::cout << " Stock disponible: " << prod->getStockActual()
                                  << " | Precio unitario: $" << std::fixed << std::setprecision(2)
                                  << prod->getCostoUnitario() << std::defaultfloat << std::setprecision(6) << "\n";

                        if (prod->getStockActual() <= 0)
                        {
                            std::cout << " [!] No hay stock disponible. Genere una orden en la Opcion 2.\n";
                        }
                        else
                        {
                            int cant = leerEntero(" Unidades a vender [1]: ", 1);
                            if (sistema.registrarVentaPOS(prod->getSKU(), cant))
                            {
                                double total = static_cast<double>(cant) * prod->getCostoUnitario();
                                std::cout << "\n[+] VENTA REGISTRADA EN POS Y KARDEX AVL\n";
                                std::cout << "    Producto       : " << prod->getNombre() << " (" << prod->getSKU() << ")\n";
                                std::cout << "    Cantidad       : " << cant << "\n";
                                std::cout << "    Total cobrado  : $" << std::fixed << std::setprecision(2)
                                          << total << std::defaultfloat << std::setprecision(6) << "\n";
                                std::cout << "    Stock restante : " << prod->getStockActual() << "\n";
                                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                            }
                            else
                            {
                                std::cout << "\n[!] Venta rechazada: cantidad invalida o superior al stock actual.\n";
                            }
                        }
                    }
                }
                else if (subopc == 2)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr)
                    {
                        std::cout << "\n[+] Producto localizado en O(1):\n";
                        prod->mostrarInfo();
                        if (sistema.existeReceta(prod->getSKU()))
                        {
                            sistema.mostrarArbolReceta(prod->getSKU());
                        }
                        else
                        {
                            std::cout << "[i] Este producto aun no tiene receta asociada.\n";
                        }
                    }
                }
                else if (subopc == 3)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema, true);
                    if (prod != nullptr)
                    {
                        prod->mostrarInfo();
                    }
                }
                pausar();
                break;
            }

            case 2:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 2. ORDEN DE PRODUCCION (Hash O(1) -> DFS -> Kardex AVL)\n";
                std::cout << "--------------------------------------------------\n";

                ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                if (prod == nullptr)
                {
                    pausar();
                    break;
                }

                if (!sistema.existeReceta(prod->getSKU()))
                {
                    std::cout << "\n[!] El producto '" << prod->getNombre() << "' ("
                              << prod->getSKU() << ") no tiene una receta asignada.\n";
                    if (confirmarSiNo(" ¿Desea crearle una receta ahora? (s/n): "))
                    {
                        if (construirYRegistrarReceta(sistema, prod))
                        {
                            sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        }
                    }
                    if (!sistema.existeReceta(prod->getSKU()))
                    {
                        pausar();
                        break;
                    }
                }

                double cantidadDeseada = leerDouble(" Ingrese la cantidad a producir en kg [10.0]: ", 10.0);
                if (cantidadDeseada <= 0.0)
                {
                    std::cout << "[!] La cantidad debe ser mayor que 0.\n";
                    pausar();
                    break;
                }

                sistema.generarReporteCocina(prod->getSKU(), cantidadDeseada);

                int unidadesProducidas = static_cast<int>(std::llround(cantidadDeseada));
                if (unidadesProducidas < 1)
                {
                    unidadesProducidas = 1;
                }
                prod->aumentarStock(unidadesProducidas);

                fecha hoy = obtenerFechaActual();
                std::ostringstream codLote;
                codLote << "LOT-" << hoy.anio
                        << std::setw(2) << std::setfill('0') << hoy.mes
                        << std::setw(2) << std::setfill('0') << hoy.dia
                        << "-" << std::setw(3) << std::setfill('0') << (sistema.getLotes().size() + 1);

                LoteProduccion nuevoLote(codLote.str(), prod->getSKU(), cantidadDeseada, hoy);
                nuevoLote.registrarEvento("Orden de Produccion",
                                          "Explosion DFS y egreso de materias primas al Kardex",
                                          "Sistema / Cocina", hoy);
                sistema.registrarLote(nuevoLote);
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);

                std::cout << "[+] Egresos registrados en Kardex AVL.\n";
                std::cout << "[+] Stock actualizado de '" << prod->getNombre()
                          << "': " << prod->getStockActual() << " unidades.\n";
                std::cout << "[+] Lote de trazabilidad creado: " << codLote.str() << "\n";

                pausar();
                break;
            }

            case 3:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 3. GESTION DE CATALOGO Y RECETAS\n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Registrar nuevo producto (validacion temprana de SKU)\n";
                std::cout << " 2. Agregar o modificar receta de un producto\n";
                std::cout << " 3. Ver arbol de receta de un producto\n";
                std::cout << " 4. Eliminar producto del catalogo\n";
                std::cout << " 5. Regresar al menu principal\n";

                int subopc = leerEntero(" Seleccione una accion [1-5]: ", 5);

                if (subopc == 1)
                {
                    std::string sku = leerLinea("\n Ingrese el SKU del nuevo producto (ej. PAN-GOL-001): ");
                    if (sku.empty())
                    {
                        std::cout << "[!] El codigo SKU no puede estar vacio.\n";
                    }
                    else if (sistema.existeProducto(sku))
                    {
                        std::cout << "[!] ERROR: Ya existe un producto registrado con el SKU '" << sku
                                  << "' (" << sistema.obtenerNombreSKU(sku) << "). Registro cancelado.\n";
                    }
                    else
                    {
                        std::string nombre = leerLinea(" Ingrese el nombre del producto: ");
                        if (nombre.empty())
                        {
                            nombre = sku;
                        }
                        TipoMasaBase masa = leerTipoMasa();
                        double hidratacion = leerDouble(" Porcentaje de hidratacion (%) [55.0]: ", 55.0);
                        int tFerm = leerEntero(" Tiempo de fermentacion (min) [120]: ", 120);
                        double tempFerm = leerDouble(" Temperatura de fermentacion (C) [28.0]: ", 28.0);
                        int tHorn = leerEntero(" Tiempo de horneado (min) [25]: ", 25);
                        int tempHorn = leerEntero(" Temperatura de horneado (C) [180]: ", 180);
                        double rend = leerDouble(" Rendimiento de masa (0.1 - 1.0) [0.90]: ", 0.90);
                        int stockAct = leerEntero(" Stock inicial (unidades) [20]: ", 20);
                        int stockMin = leerEntero(" Stock minimo (unidades) [5]: ", 5);
                        fecha fVenc = leerFecha(" Fecha de vencimiento (Dia Mes Anio) [30 11 2026]: ", {30, 11, 2026});
                        double costo = leerDouble(" Precio / Costo unitario ($) [5.00]: ", 5.00);

                        ProductoPanaderia* nuevo = new ProductoPanaderia(
                            sku, nombre, masa, hidratacion, tFerm, tempFerm,
                            tHorn, tempHorn, rend, stockAct, stockMin, fVenc, costo
                        );

                        if (sistema.registrarProducto(nuevo, true))
                        {
                            std::cout << "\n[+] Producto '" << nombre << "' (" << sku
                                      << ") registrado en Tabla Hash O(1).\n";
                            if (confirmarSiNo(" ¿Desea agregar una receta para este producto ahora? (s/n): "))
                            {
                                construirYRegistrarReceta(sistema, nuevo);
                            }
                            sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        }
                        else
                        {
                            delete nuevo;
                            std::cout << "\n[!] No se pudo registrar el producto.\n";
                        }
                    }
                }
                else if (subopc == 2)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr && construirYRegistrarReceta(sistema, prod))
                    {
                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    }
                }
                else if (subopc == 3)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr)
                    {
                        sistema.mostrarArbolReceta(prod->getSKU());
                    }
                }
                else if (subopc == 4)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr)
                    {
                        std::string skuElim = prod->getSKU();
                        std::string nomElim = prod->getNombre();
                        if (confirmarSiNo(" ¿Confirma eliminar '" + nomElim + "' (" + skuElim + ")? (s/n): "))
                        {
                            sistema.eliminarProducto(skuElim);
                            sistema.eliminarReceta(skuElim);
                            sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                            std::cout << "\n[+] Producto '" << nomElim << "' (" << skuElim << ") eliminado.\n";
                        }
                    }
                }
                pausar();
                break;
            }

            case 4:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 4. TRAZABILIDAD DE LOTES DE PRODUCCION (SLList)  \n";
                std::cout << "--------------------------------------------------\n";

                Designar::DynArray<LoteProduccion>& lotes = sistema.getLotes();
                if (lotes.is_empty())
                {
                    std::cout << " (No hay lotes registrados)\n";
                }
                else
                {
                    std::cout << " Lotes registrados:\n";
                    for (size_t i = 0; i < lotes.size(); ++i)
                    {
                        const LoteProduccion& l = lotes[i];
                        std::cout << "  [" << (i + 1) << "] " << l.getCodigoLote()
                                  << " | " << l.getCodigoSKU() << " (" << sistema.obtenerNombreSKU(l.getCodigoSKU()) << ")"
                                  << " | " << l.getCantidadPlanificada() << " kg"
                                  << " | Eventos: " << l.getNumeroEventos() << "\n";
                    }
                }

                std::cout << "\n 1. Ver historial cronologico de un lote\n";
                std::cout << " 2. Registrar nuevo evento de auditoria en un lote\n";
                std::cout << " 3. Crear nuevo lote manual\n";
                std::cout << " 4. Regresar al menu principal\n";

                int subopc = leerEntero(" Seleccione una accion [1-4]: ", 1);

                if ((subopc == 1 || subopc == 2) && !lotes.is_empty())
                {
                    int idx = leerEntero(" Numero de lote [1-" + std::to_string(lotes.size()) + "]: ", 1);
                    if (idx >= 1 && static_cast<size_t>(idx) <= lotes.size())
                    {
                        LoteProduccion& loteSel = lotes[static_cast<size_t>(idx - 1)];
                        if (subopc == 2)
                        {
                            std::string etapa = leerLinea(" Ingrese la etapa (ej. Enfriamiento, Despacho): ");
                            std::string desc = leerLinea(" Ingrese la descripcion: ");
                            std::string resp = leerLinea(" Ingrese el responsable: ");
                            loteSel.registrarEvento(etapa, desc, resp, obtenerFechaActual());
                            sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                            std::cout << "\n[+] Evento agregado a la lista simplemente enlazada.\n\n";
                        }
                        sistema.mostrarHistorialLote(loteSel);
                    }
                    else
                    {
                        std::cout << " [!] Numero de lote invalido.\n";
                    }
                }
                else if (subopc == 3)
                {
                    ProductoPanaderia* prod = mostrarYSeleccionarProductoPaginado(sistema);
                    if (prod != nullptr)
                    {
                        fecha hoy = obtenerFechaActual();
                        std::string codDef = "LOT-" + std::to_string(lotes.size() + 1);
                        std::string codLote = leerLinea(" Codigo de lote [" + codDef + "]: ", codDef);
                        double cant = leerDouble(" Cantidad planificada (kg) [10.0]: ", 10.0);
                        LoteProduccion nuevo(codLote, prod->getSKU(), cant, hoy);
                        nuevo.registrarEvento("Creacion", "Lote registrado manualmente", "Operador CLI", hoy);
                        sistema.registrarLote(nuevo);
                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        std::cout << "\n[+] Lote '" << codLote << "' registrado para "
                                  << prod->getNombre() << " (" << prod->getSKU() << ").\n";
                    }
                }
                pausar();
                break;
            }

            case 5:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 5. GESTION DE PERECEDEROS (Min-Heap / FEFO)      \n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " Insumos perecederos en almacen: " << sistema.totalPerecederos() << "\n\n";

                if (sistema.tienePerecederos())
                {
                    ProductoPanaderia prox = sistema.consultarProximoAVencer();
                    fecha f = prox.getFechaVencimiento();
                    std::cout << "[!] Insumo MAS proximo a vencer (Cima del Min-Heap):\n"
                              << "    -> " << prox.getNombre() << " (SKU: " << prox.getSKU() << ")\n"
                              << "    -> Fecha de vencimiento: " << f.dia << "/" << f.mes << "/" << f.anio << "\n\n";
                }

                std::cout << " Opciones:\n";
                std::cout << " 1. Despachar el mas proximo a vencer (extraer del heap)\n";
                std::cout << " 2. Despachar TODOS los insumos en orden FEFO\n";
                std::cout << " 3. Registrar nuevo insumo perecedero en el Min-Heap\n";
                std::cout << " 4. Regresar al menu principal\n";

                int subopc = leerEntero(" Seleccione una accion [1-4]: ", 4);

                if (subopc == 1)
                {
                    if (sistema.tienePerecederos())
                    {
                        ProductoPanaderia desp = sistema.despacharProximoAVencer();
                        fecha f = desp.getFechaVencimiento();
                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        std::cout << "\n[+] DESPACHADO (FEFO): " << desp.getNombre()
                                  << " (" << desp.getSKU() << ")"
                                  << " | Vencimiento: " << f.dia << "/" << f.mes << "/" << f.anio << "\n";
                    }
                    else
                    {
                        std::cout << "\n[!] No quedan insumos perecederos en el almacen.\n";
                    }
                }
                else if (subopc == 2)
                {
                    if (!sistema.tienePerecederos())
                    {
                        std::cout << "\n[!] El almacen esta vacio.\n";
                    }
                    else
                    {
                        std::cout << "\n--> Despachando todos los perecederos por orden FEFO:\n";
                        int num = 1;
                        while (sistema.tienePerecederos())
                        {
                            ProductoPanaderia desp = sistema.despacharProximoAVencer();
                            fecha f = desp.getFechaVencimiento();
                            std::cout << "  " << num++ << ". [" << f.dia << "/" << f.mes << "/" << f.anio << "] "
                                      << desp.getNombre() << " (SKU: " << desp.getSKU() << ")\n";
                        }
                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    }
                }
                else if (subopc == 3)
                {
                    std::string skuPer = leerLinea(" SKU del insumo perecedero (ej. MP-LEC-001): ");
                    if (!skuPer.empty())
                    {
                        asegurarNombreSKU(sistema, skuPer);
                        std::string nomPer = sistema.obtenerNombreSKU(skuPer);
                        int stockPer = leerEntero(" Cantidad en almacen [10]: ", 10);
                        fecha fVenc = leerFecha(" Fecha de vencimiento (Dia Mes Anio) [25 11 2026]: ", {25, 11, 2026});
                        double costoPer = leerDouble(" Costo unitario ($) [2.50]: ", 2.50);

                        ProductoPanaderia nuevoPer(skuPer, nomPer, TipoMasaBase::BLANDA_ENRIQUECIDA,
                                                   0, 0, 0, 0, 0, 1.0, stockPer, 5, fVenc, costoPer);
                        sistema.registrarPerecedero(nuevoPer);
                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        std::cout << "\n[+] Perecedero '" << nomPer << "' (" << skuPer << ") ingresado al Min-Heap.\n";
                    }
                }
                pausar();
                break;
            }

            case 6:
            {
                std::cout << "\n--------------------------------------------------\n";
                std::cout << " 6. KARDEX DE INVENTARIO (ARBOL AVL)              \n";
                std::cout << "--------------------------------------------------\n";
                std::cout << " 1. Consultar por rango de fechas (In-order acotado O(log n + k))\n";
                std::cout << " 2. Ver historial completo del Kardex\n";
                std::cout << " 3. Registrar entrada de compra o ajuste manual\n";
                std::cout << " 4. Regresar al menu principal\n";

                int subopc = leerEntero(" Seleccione una accion [1-4]: ", 1);

                if (subopc == 1)
                {
                    fecha inicio = leerFecha(" Fecha Inicio (Dia Mes Anio) [01 11 2026]: ", {1, 11, 2026});
                    fecha fin = leerFecha(" Fecha Fin (Dia Mes Anio) [31 12 2026]: ", {31, 12, 2026});
                    sistema.consultarKardexPorRangoFechas(inicio, fin);
                }
                else if (subopc == 2)
                {
                    sistema.consultarKardexPorRangoFechas({1, 1, 2020}, {31, 12, 2030});
                }
                else if (subopc == 3)
                {
                    mostrarInsumosConocidos(sistema);
                    std::string skuMov = leerLinea(" Ingrese el SKU del producto o insumo: ");
                    if (!skuMov.empty())
                    {
                        asegurarNombreSKU(sistema, skuMov);
                        int tipoSel = leerEntero(" Tipo [1 = Entrada Compra, 2 = Ajuste Inventario]: ", 1);
                        TipoMovimiento tipo = (tipoSel == 2)
                                                  ? TipoMovimiento::AJUSTE_INVENTARIO
                                                  : TipoMovimiento::ENTRADA_COMPRA;
                        double cantMov = leerDouble(" Cantidad [25.0]: ", 25.0);
                        std::string descMov = leerLinea(" Descripcion del movimiento: ",
                                                        "Movimiento manual de inventario");
                        fecha hoy = obtenerFechaActual();
                        sistema.registrarMovimientoInventario({hoy, skuMov, tipo, cantMov, descMov});

                        ProductoPanaderia* prod = sistema.buscarProducto(skuMov);
                        if (prod != nullptr && tipo == TipoMovimiento::ENTRADA_COMPRA)
                        {
                            prod->aumentarStock(static_cast<int>(std::llround(cantMov)));
                        }

                        sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                        std::cout << "\n[+] Movimiento registrado en el Kardex AVL para "
                                  << skuMov << " (" << sistema.obtenerNombreSKU(skuMov) << ").\n";
                    }
                }
                pausar();
                break;
            }

            case 7:
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                std::cout << "\n[+] Datos guardados en " << RUTA_PERSISTENCIA_JSON << "\n";
                std::cout << "Saliendo del Bakery System. Hasta luego!\n";
                break;

            default:
                std::cout << "[!] Opcion desconocida. Seleccione entre 1 y 7.\n";
                pausar();
                break;
        }
    }

    return 0;
}
