#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#endif

#include "SistemaPanaderia.hpp"

namespace
{
    const std::string RUTA_PERSISTENCIA_JSON = "data/bakery_data.json";

    /**
     * Configura una paleta cálida inspirada en panadería artesanal sobre Dear ImGui.
     */
    void aplicarEstiloPanaderia()
    {
        ImGui::StyleColorsLight();
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding = 0.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.TabRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.WindowPadding = ImVec2(14.0f, 14.0f);
        style.FramePadding = ImVec2(10.0f, 7.0f);
        style.CellPadding = ImVec2(10.0f, 7.0f);
        style.ItemSpacing = ImVec2(10.0f, 9.0f);
        style.ScrollbarSize = 16.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]       = ImVec4(0.98f, 0.96f, 0.92f, 1.00f);
        colors[ImGuiCol_ChildBg]        = ImVec4(1.00f, 0.99f, 0.96f, 1.00f);
        colors[ImGuiCol_Border]         = ImVec4(0.84f, 0.77f, 0.67f, 1.00f);
        colors[ImGuiCol_FrameBg]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.96f, 0.91f, 0.83f, 1.00f);
        colors[ImGuiCol_Button]         = ImVec4(0.76f, 0.45f, 0.18f, 1.00f);
        colors[ImGuiCol_ButtonHovered]  = ImVec4(0.84f, 0.53f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonActive]   = ImVec4(0.65f, 0.37f, 0.13f, 1.00f);
        colors[ImGuiCol_Header]         = ImVec4(0.89f, 0.77f, 0.62f, 0.80f);
        colors[ImGuiCol_HeaderHovered]  = ImVec4(0.85f, 0.69f, 0.50f, 0.90f);
        colors[ImGuiCol_Tab]            = ImVec4(0.90f, 0.83f, 0.73f, 1.00f);
        colors[ImGuiCol_TabHovered]     = ImVec4(0.84f, 0.53f, 0.24f, 0.90f);
        colors[ImGuiCol_TabSelected]    = ImVec4(0.76f, 0.45f, 0.18f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]  = ImVec4(0.91f, 0.84f, 0.74f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt]  = ImVec4(0.97f, 0.94f, 0.89f, 0.60f);
    }

    /**
     * Carga una tipografía TrueType legible a 18.5 px (con fallback seguro) para mejorar
     * la lectura en pantallas pequeñas y de alta densidad.
     */
    void configurarTipografiaLegible(ImGuiIO& io)
    {
        const float tamFuente = 18.5f;
        const char* candidatos[] = {
            "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
            "C:\\Windows\\Fonts\\segoeui.ttf",
            "C:\\Windows\\Fonts\\arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/Library/Fonts/Arial.ttf"
        };

        ImFontConfig cfg;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        cfg.PixelSnapH = true;

        for (const char* ruta : candidatos)
        {
            std::error_code ec;
            if (std::filesystem::exists(ruta, ec))
            {
                ImFont* fuente = io.Fonts->AddFontFromFileTTF(
                    ruta, tamFuente, &cfg, io.Fonts->GetGlyphRangesDefault()
                );
                if (fuente != nullptr)
                {
                    io.FontDefault = fuente;
                    return;
                }
            }
        }

        ImFontConfig cfgDef;
        cfgDef.SizePixels = 17.0f;
        io.Fonts->AddFontDefault(&cfgDef);
    }

    /**
     * Dibuja una fila clave-valor dentro de una tabla ImGui de 2 columnas.
     */
    void filaDetalleProducto(const char* etiqueta, const std::string& valor)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(etiqueta);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(valor.c_str());
    }

    /**
     * Pestaña 1: Punto de Venta (POS) con catálogo paginado, selección por número y descuento de stock.
     */
    void renderPestanaPOS(SistemaPanaderia& sistema,
                          char* skuBusqueda, size_t skuBusquedaTam,
                          bool& busquedaEjecutada)
    {
        static int paginaActual = 0;
        static int numeroSeleccion = 1;
        static int unidadesVenta = 1;
        static std::string mensajePos;
        static bool exitoPos = true;

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Punto de Venta (POS) y Consulta Rapida en Tabla Hash O(1)");
        ImGui::TextWrapped("Seleccione un producto por su numero en la tabla paginada o por SKU "
                           "para ver su ficha tecnica, cobrar una venta y actualizar el Kardex AVL.");
        ImGui::Spacing();

        const Designar::DynArray<ProductoPanaderia*>& productos = sistema.obtenerListaProductos();
        const int totalProductos = static_cast<int>(productos.size());
        const int POR_PAGINA = 5;
        const int totalPaginas = (totalProductos > 0) ? ((totalProductos + POR_PAGINA - 1) / POR_PAGINA) : 1;
        if (paginaActual >= totalPaginas)
        {
            paginaActual = totalPaginas - 1;
        }
        if (paginaActual < 0)
        {
            paginaActual = 0;
        }

        const float em = ImGui::GetFontSize();
        float anchoIzq = (ImGui::GetContentRegionAvail().x - 14.0f) * 0.54f;

        // =====================================================================
        // PANEL IZQUIERDO: CATÁLOGO PAGINADO Y SELECCIÓN POR NÚMERO / SKU
        // =====================================================================
        ImGui::BeginChild("PanelPOSCatalogo", ImVec2(anchoIzq, 0.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Catalogo Paginado de Productos (%d)", totalProductos);
        ImGui::Separator();
        ImGui::Spacing();

        // Selección directa por número
        ImGui::SetNextItemWidth(7.2f * em);
        ImGui::InputInt("# Producto", &numeroSeleccion);
        if (numeroSeleccion < 1)
        {
            numeroSeleccion = 1;
        }
        if (totalProductos > 0 && numeroSeleccion > totalProductos)
        {
            numeroSeleccion = totalProductos;
        }
        ImGui::SameLine();
        if (ImGui::Button("Seleccionar #") && totalProductos > 0)
        {
            ProductoPanaderia* elegido = productos[static_cast<size_t>(numeroSeleccion - 1)];
            if (elegido != nullptr)
            {
                std::snprintf(skuBusqueda, skuBusquedaTam, "%s", elegido->getSKU().c_str());
                paginaActual = (numeroSeleccion - 1) / POR_PAGINA;
                busquedaEjecutada = true;
            }
        }

        // Búsqueda directa por código SKU en O(1)
        ImGui::SetNextItemWidth(12.5f * em);
        if (ImGui::InputText("SKU", skuBusqueda, skuBusquedaTam, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            busquedaEjecutada = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Buscar O(1)"))
        {
            busquedaEjecutada = true;
        }

        ImGui::Spacing();

        if (totalProductos == 0)
        {
            ImGui::TextDisabled("No hay productos registrados en el catalogo.");
        }
        else
        {
            float altoTabla = -ImGui::GetFrameHeightWithSpacing() * 1.6f;
            if (ImGui::BeginTable("TablaProductosPaginada", 6,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                                  ImVec2(0.0f, altoTabla)))
            {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 2.4f * em);
                ImGui::TableSetupColumn("SKU", ImGuiTableColumnFlags_WidthFixed, 7.6f * em);
                ImGui::TableSetupColumn("Nombre", ImGuiTableColumnFlags_WidthFixed, 13.5f * em);
                ImGui::TableSetupColumn("Stock", ImGuiTableColumnFlags_WidthFixed, 4.2f * em);
                ImGui::TableSetupColumn("Precio", ImGuiTableColumnFlags_WidthFixed, 5.0f * em);
                ImGui::TableSetupColumn("Receta", ImGuiTableColumnFlags_WidthFixed, 4.4f * em);
                ImGui::TableHeadersRow();

                int inicio = paginaActual * POR_PAGINA;
                int fin = inicio + POR_PAGINA;
                if (fin > totalProductos)
                {
                    fin = totalProductos;
                }

                for (int i = inicio; i < fin; ++i)
                {
                    const ProductoPanaderia* p = productos[static_cast<size_t>(i)];
                    if (p == nullptr)
                    {
                        continue;
                    }

                    bool seleccionado = (std::string(skuBusqueda) == p->getSKU());
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    char idSel[32];
                    std::snprintf(idSel, sizeof(idSel), "%d##row_%d", i + 1, i);
                    if (ImGui::Selectable(idSel, seleccionado, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        std::snprintf(skuBusqueda, skuBusquedaTam, "%s", p->getSKU().c_str());
                        numeroSeleccion = i + 1;
                        busquedaEjecutada = true;
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(p->getSKU().c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(p->getNombre().c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", p->getStockActual());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("$%.2f", p->getCostoUnitario());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::TextUnformatted(sistema.existeReceta(p->getSKU()) ? "SI" : "NO");
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::BeginDisabled(paginaActual <= 0);
            if (ImGui::Button("<< Anterior"))
            {
                --paginaActual;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::Text("Pagina %d de %d", paginaActual + 1, totalPaginas);
            ImGui::SameLine();

            ImGui::BeginDisabled(paginaActual + 1 >= totalPaginas);
            if (ImGui::Button("Siguiente >>"))
            {
                ++paginaActual;
            }
            ImGui::EndDisabled();
        }

        ImGui::EndChild();

        ImGui::SameLine();

        // =====================================================================
        // PANEL DERECHO: FICHA TÉCNICA Y CAJA REGISTRADORA (VENTA POS)
        // =====================================================================
        ImGui::BeginChild("PanelPOSDetalleVenta", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        if (!busquedaEjecutada)
        {
            ImGui::TextDisabled("Seleccione un producto en el catalogo o busque por SKU.");
            ImGui::EndChild();
            return;
        }

        ProductoPanaderia* prod = sistema.buscarProducto(std::string(skuBusqueda));
        if (prod == nullptr)
        {
            ImGui::TextColored(ImVec4(0.78f, 0.12f, 0.12f, 1.0f),
                               "[!] No existe un producto con el SKU: %s", skuBusqueda);
            ImGui::EndChild();
            return;
        }

        ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f),
                           "%s (%s)", prod->getNombre().c_str(), prod->getSKU().c_str());
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("TablaFichaProducto", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit |
                              ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Campo", ImGuiTableColumnFlags_WidthFixed, 8.8f * em);
            ImGui::TableSetupColumn("Detalle", ImGuiTableColumnFlags_WidthFixed, 14.5f * em);
            ImGui::TableHeadersRow();

            filaDetalleProducto("Codigo SKU", prod->getSKU());
            filaDetalleProducto("Nombre", prod->getNombre());
            filaDetalleProducto("Tipo de Masa", prod->getTipoMasaNombre());

            char buf[128];
            std::snprintf(buf, sizeof(buf), "%.1f %%", prod->getPorcentajeHidratacion());
            filaDetalleProducto("Hidratacion", buf);

            std::snprintf(buf, sizeof(buf), "%d min a %.1f C",
                          prod->getTiempoFermentacion(), prod->getTemperaturaFermentacion());
            filaDetalleProducto("Fermentacion", buf);

            std::snprintf(buf, sizeof(buf), "%d min a %d C",
                          prod->getTiempoHorneado(), prod->getTemperaturaHorneado());
            filaDetalleProducto("Horneado", buf);

            std::snprintf(buf, sizeof(buf), "%.0f %%", prod->getRendimientoMasa() * 100.0);
            filaDetalleProducto("Rendimiento", buf);

            fecha fv = prod->getFechaVencimiento();
            std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d", fv.dia, fv.mes, fv.anio);
            filaDetalleProducto("Vencimiento", buf);

            std::snprintf(buf, sizeof(buf), "$ %.2f", prod->getCostoUnitario());
            filaDetalleProducto("Precio Unitario", buf);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Stock Actual / Min");
            ImGui::TableSetColumnIndex(1);
            if (prod->requiereReabastecimiento())
            {
                ImGui::TextColored(ImVec4(0.80f, 0.15f, 0.15f, 1.0f),
                                   "%d / %d (REABASTECER)",
                                   prod->getStockActual(), prod->getStockMinimo());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f),
                                   "%d / %d (Saludable)",
                                   prod->getStockActual(), prod->getStockMinimo());
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Registrar Venta en Punto de Venta (POS)");

        if (unidadesVenta < 1)
        {
            unidadesVenta = 1;
        }
        ImGui::SetNextItemWidth(8.0f * em);
        ImGui::InputInt("Unidades a vender", &unidadesVenta);
        if (unidadesVenta < 1)
        {
            unidadesVenta = 1;
        }

        double totalCobro = static_cast<double>(unidadesVenta) * prod->getCostoUnitario();
        ImGui::Text("Total a cobrar: $ %.2f", totalCobro);

        ImGui::BeginDisabled(prod->getStockActual() <= 0);
        if (ImGui::Button("Confirmar Venta POS (Descontar Stock + Kardex)",
                          ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.25f)))
        {
            if (sistema.registrarVentaPOS(prod->getSKU(), unidadesVenta))
            {
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                char msgBuf[192];
                std::snprintf(msgBuf, sizeof(msgBuf),
                              "[+] Venta registrada: %d x %s (%s) = $%.2f | Nuevo stock: %d",
                              unidadesVenta,
                              prod->getNombre().c_str(),
                              prod->getSKU().c_str(),
                              totalCobro,
                              prod->getStockActual());
                mensajePos = msgBuf;
                exitoPos = true;
            }
            else
            {
                mensajePos = "[!] Stock insuficiente para vender esa cantidad.";
                exitoPos = false;
            }
        }
        ImGui::EndDisabled();

        if (prod->getStockActual() <= 0)
        {
            ImGui::TextColored(ImVec4(0.78f, 0.12f, 0.12f, 1.0f),
                               "[!] Sin stock disponible. Produzca un lote en la Pestana 3.");
        }

        if (!mensajePos.empty())
        {
            ImGui::Spacing();
            ImVec4 color = exitoPos
                ? ImVec4(0.10f, 0.50f, 0.20f, 1.0f)
                : ImVec4(0.78f, 0.12f, 0.12f, 1.0f);
            ImGui::TextColored(color, "%s", mensajePos.c_str());
        }

        ImGui::EndChild();
    }

    struct BorradorIngrediente
    {
        std::string sku;
        std::string nombre;
        double cantidad;
        std::string unidad;
        bool esSubproducto;
        std::string skuPadre; // "" significa directo bajo la raiz
        double rendimiento;
    };

    /**
     * Pestaña 2: Gestión de Catálogo (validación inmediata de SKU) y Constructor de Recetas.
     */
    void renderPestanaCatalogoYRecetas(SistemaPanaderia& sistema)
    {
        static char nuevoSku[64] = "";
        static char nuevoNombre[128] = "";
        static int idxMasa = 0;
        static float hidratacion = 58.0f;
        static int tiempoFerm = 120;
        static float tempFerm = 28.0f;
        static int tiempoHorn = 25;
        static int tempHorn = 190;
        static float rendimiento = 0.90f;
        static int stockAct = 25;
        static int stockMin = 10;
        static float costo = 4.50f;
        static int diaVenc = 20;
        static int mesVenc = 11;
        static int anioVenc = 2026;

        static int idxProductoReceta = 0;
        static std::vector<BorradorIngrediente> borradorReceta;
        static char ingSku[64] = "MP-HAR-001";
        static char ingNombre[128] = "Harina Panadera";
        static float ingCantidad = 0.50f;
        static char ingUnidad[16] = "kg";
        static bool ingEsSubproducto = false;
        static float ingRendimiento = 1.0f;
        static int idxPadreReceta = 0; // 0 = Raiz, >0 = subproducto i

        static int idxProductoEliminar = 0;
        static std::string mensajeEstado;
        static bool estadoExito = true;

        const char* tiposMasa[] = {
            "Dulce Sobada",
            "Hojaldrada",
            "Madre Acida",
            "Blanda Enriquecida",
            "Crujiente Rustica",
            "Integral"
        };

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Catalogo Hash (Validacion Temprana de SKU) y Constructor de Recetas");
        ImGui::Spacing();

        const float em = ImGui::GetFontSize();
        float anchoIzq = (ImGui::GetContentRegionAvail().x - 14.0f) * 0.45f;

        // =====================================================================
        // PANEL IZQUIERDO: ALTA DE PRODUCTO CON VALIDACIÓN TEMPRANA DE SKU
        // =====================================================================
        ImGui::BeginChild("PanelAltaProducto", ImVec2(anchoIzq, 0.0f),
                          ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("1. Registrar Nuevo Producto");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.52f);

        ImGui::InputText("SKU (ej. PAN-GOL-001)", nuevoSku, sizeof(nuevoSku));
        std::string skuStr(nuevoSku);
        bool skuDuplicado = !skuStr.empty() && sistema.existeProducto(skuStr);
        if (skuDuplicado)
        {
            ImGui::TextColored(ImVec4(0.80f, 0.12f, 0.12f, 1.0f),
                               "[!] Este SKU ya existe: '%s' (%s). Cambielo antes de continuar.",
                               skuStr.c_str(), sistema.obtenerNombreSKU(skuStr).c_str());
        }

        ImGui::BeginDisabled(skuDuplicado);
        ImGui::InputText("Nombre Producto", nuevoNombre, sizeof(nuevoNombre));
        ImGui::Combo("Tipo de Masa", &idxMasa, tiposMasa, 6);
        ImGui::SliderFloat("% Hidratacion", &hidratacion, 30.0f, 90.0f, "%.1f %%");
        ImGui::InputInt("Tiempo Ferm. (min)", &tiempoFerm);
        ImGui::InputFloat("Temp. Ferm. (C)", &tempFerm, 0.5f, 2.0f, "%.1f");
        ImGui::InputInt("Tiempo Horneado", &tiempoHorn);
        ImGui::InputInt("Temp. Horneado (C)", &tempHorn);
        ImGui::SliderFloat("Rendimiento Masa", &rendimiento, 0.50f, 1.00f, "%.2f");
        ImGui::InputInt("Stock Actual", &stockAct);
        ImGui::InputInt("Stock Minimo", &stockMin);
        ImGui::InputFloat("Precio / Costo ($)", &costo, 0.25f, 1.0f, "%.2f");
        ImGui::InputInt("Dia Venc.", &diaVenc);
        ImGui::InputInt("Mes Venc.", &mesVenc);
        ImGui::InputInt("Anio Venc.", &anioVenc);

        ImGui::Spacing();
        if (ImGui::Button("Registrar Producto en Tabla Hash O(1)",
                          ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.2f)))
        {
            std::string nomStr(nuevoNombre);
            if (skuStr.empty() || nomStr.empty())
            {
                mensajeEstado = "[!] Debe ingresar tanto el SKU como el Nombre del producto.";
                estadoExito = false;
            }
            else
            {
                TipoMasaBase masaElegida = static_cast<TipoMasaBase>(idxMasa);
                ProductoPanaderia* nuevo = new ProductoPanaderia(
                    skuStr,
                    nomStr,
                    masaElegida,
                    static_cast<double>(hidratacion),
                    tiempoFerm,
                    static_cast<double>(tempFerm),
                    tiempoHorn,
                    tempHorn,
                    static_cast<double>(rendimiento),
                    stockAct,
                    stockMin,
                    {diaVenc, mesVenc, anioVenc},
                    static_cast<double>(costo)
                );

                if (sistema.registrarProducto(nuevo, true))
                {
                    sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    const auto& lista = sistema.obtenerListaProductos();
                    if (!lista.is_empty())
                    {
                        idxProductoReceta = static_cast<int>(lista.size() - 1);
                    }
                    mensajeEstado = "[+] Producto '" + nomStr + "' (" + skuStr +
                                    ") registrado y guardado en JSON. Ya puede definir su receta a la derecha.";
                    estadoExito = true;
                    nuevoSku[0] = '\0';
                    nuevoNombre[0] = '\0';
                }
                else
                {
                    delete nuevo;
                    mensajeEstado = "[!] No se pudo registrar el SKU '" + skuStr + "'.";
                    estadoExito = false;
                }
            }
        }
        ImGui::EndDisabled();
        ImGui::PopItemWidth();

        ImGui::EndChild();

        ImGui::SameLine();

        // =====================================================================
        // PANEL DERECHO: CONSTRUCTOR DE RECETAS Y BAJA DE PRODUCTOS
        // =====================================================================
        ImGui::BeginChild("PanelRecetasYBaja", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        const Designar::DynArray<ProductoPanaderia*>& productos = sistema.obtenerListaProductos();

        ImGui::Text("2. Agregar o Reemplazar Receta a un Producto");
        ImGui::Separator();
        ImGui::Spacing();

        if (productos.is_empty())
        {
            ImGui::TextDisabled("Registre al menos un producto primero.");
        }
        else
        {
            if (idxProductoReceta >= static_cast<int>(productos.size()))
            {
                idxProductoReceta = 0;
            }

            ProductoPanaderia* prodRec = productos[static_cast<size_t>(idxProductoReceta)];
            char previewRec[192];
            std::snprintf(previewRec, sizeof(previewRec), "[%d] %s - %s (Receta: %s)",
                          idxProductoReceta + 1,
                          prodRec->getSKU().c_str(),
                          prodRec->getNombre().c_str(),
                          sistema.existeReceta(prodRec->getSKU()) ? "SI" : "NO");

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
            if (ImGui::BeginCombo("Producto destino", previewRec))
            {
                for (size_t i = 0; i < productos.size(); ++i)
                {
                    const ProductoPanaderia* p = productos[i];
                    char itemLbl[192];
                    std::snprintf(itemLbl, sizeof(itemLbl), "[%zu] %s - %s (Receta: %s)",
                                  i + 1, p->getSKU().c_str(), p->getNombre().c_str(),
                                  sistema.existeReceta(p->getSKU()) ? "SI" : "NO");
                    bool sel = (static_cast<int>(i) == idxProductoReceta);
                    if (ImGui::Selectable(itemLbl, sel))
                    {
                        idxProductoReceta = static_cast<int>(i);
                    }
                }
                ImGui::EndCombo();
            }

            // Selector rápido de insumos conocidos para no memorizar SKUs
            ImGui::TextDisabled("Insumos rapidos conocidos:");
            const char* skusBase[] = {"MP-HAR-001", "MP-AGU-001", "MP-LEV-001", "MP-AZU-001", "MP-SAL-001", "MP-MAR-001", "MP-JAM-001"};
            for (int k = 0; k < 7; ++k)
            {
                if (k % 4 != 0)
                {
                    ImGui::SameLine();
                }
                if (ImGui::SmallButton(skusBase[k]))
                {
                    std::snprintf(ingSku, sizeof(ingSku), "%s", skusBase[k]);
                    std::snprintf(ingNombre, sizeof(ingNombre), "%s", sistema.obtenerNombreSKU(skusBase[k]).c_str());
                }
            }

            ImGui::SetNextItemWidth(10.5f * em);
            if (ImGui::InputText("SKU Insumo/Subprod.", ingSku, sizeof(ingSku)))
            {
                std::string s(ingSku);
                if (sistema.obtenerCatalogoNombresSKU().has(s))
                {
                    std::snprintf(ingNombre, sizeof(ingNombre), "%s", sistema.obtenerNombreSKU(s).c_str());
                }
            }
            ImGui::SetNextItemWidth(15.0f * em);
            ImGui::InputText("Nombre Ingrediente", ingNombre, sizeof(ingNombre));

            ImGui::SetNextItemWidth(7.2f * em);
            ImGui::InputFloat("Cant./kg", &ingCantidad, 0.05f, 0.1f, "%.3f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(4.2f * em);
            ImGui::InputText("Unid.", ingUnidad, sizeof(ingUnidad));
            ImGui::SameLine();
            ImGui::Checkbox("Es Subproducto", &ingEsSubproducto);

            if (ingEsSubproducto)
            {
                ImGui::SetNextItemWidth(8.5f * em);
                ImGui::SliderFloat("Rendimiento Subprod.", &ingRendimiento, 0.50f, 1.00f, "%.2f");
            }
            else
            {
                // Construir lista de padres posibles (Raiz + subproductos en el borrador)
                std::vector<std::string> skusPadres;
                std::vector<std::string> labelsPadres;
                skusPadres.push_back("");
                labelsPadres.push_back("Raiz (" + prodRec->getSKU() + " - " + prodRec->getNombre() + ")");
                for (const auto& b : borradorReceta)
                {
                    if (b.esSubproducto)
                    {
                        skusPadres.push_back(b.sku);
                        labelsPadres.push_back(b.sku + " (" + b.nombre + ")");
                    }
                }
                if (idxPadreReceta >= static_cast<int>(labelsPadres.size()))
                {
                    idxPadreReceta = 0;
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
                if (ImGui::BeginCombo("Nodo Padre", labelsPadres[static_cast<size_t>(idxPadreReceta)].c_str()))
                {
                    for (size_t pIdx = 0; pIdx < labelsPadres.size(); ++pIdx)
                    {
                        bool sel = (static_cast<int>(pIdx) == idxPadreReceta);
                        if (ImGui::Selectable(labelsPadres[pIdx].c_str(), sel))
                        {
                            idxPadreReceta = static_cast<int>(pIdx);
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            if (ImGui::Button("+ Agregar Componente al Borrador"))
            {
                std::string sSku(ingSku);
                std::string sNom(ingNombre);
                if (!sSku.empty() && ingCantidad > 0.0f)
                {
                    std::string padreSku = "";
                    if (!ingEsSubproducto && idxPadreReceta > 0)
                    {
                        int contSub = 0;
                        for (const auto& b : borradorReceta)
                        {
                            if (b.esSubproducto)
                            {
                                ++contSub;
                                if (contSub == idxPadreReceta)
                                {
                                    padreSku = b.sku;
                                    break;
                                }
                            }
                        }
                    }
                    borradorReceta.push_back({
                        sSku,
                        sNom.empty() ? sistema.obtenerNombreSKU(sSku) : sNom,
                        static_cast<double>(ingCantidad),
                        ingUnidad[0] != '\0' ? std::string(ingUnidad) : "kg",
                        ingEsSubproducto,
                        padreSku,
                        ingEsSubproducto ? static_cast<double>(ingRendimiento) : 1.0
                    });
                }
            }

            ImGui::Spacing();
            if (ImGui::BeginTable("TablaBorradorReceta", 6,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                                  ImVec2(0.0f, 9.5f * em)))
            {
                ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 3.8f * em);
                ImGui::TableSetupColumn("Padre", ImGuiTableColumnFlags_WidthFixed, 6.8f * em);
                ImGui::TableSetupColumn("SKU", ImGuiTableColumnFlags_WidthFixed, 7.2f * em);
                ImGui::TableSetupColumn("Nombre", ImGuiTableColumnFlags_WidthFixed, 12.5f * em);
                ImGui::TableSetupColumn("Cant.", ImGuiTableColumnFlags_WidthFixed, 6.0f * em);
                ImGui::TableSetupColumn("Acc.", ImGuiTableColumnFlags_WidthFixed, 4.0f * em);
                ImGui::TableHeadersRow();

                int idxQuitar = -1;
                for (size_t i = 0; i < borradorReceta.size(); ++i)
                {
                    const auto& b = borradorReceta[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(b.esSubproducto ? "SUB" : "HOJA");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(b.skuPadre.empty() ? "RAIZ" : b.skuPadre.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(b.sku.c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(b.nombre.c_str());
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.3f %s", b.cantidad, b.unidad.c_str());
                    ImGui::TableSetColumnIndex(5);
                    char btnDel[24];
                    std::snprintf(btnDel, sizeof(btnDel), "X##b_%zu", i);
                    if (ImGui::SmallButton(btnDel))
                    {
                        idxQuitar = static_cast<int>(i);
                    }
                }
                if (idxQuitar >= 0)
                {
                    borradorReceta.erase(borradorReceta.begin() + idxQuitar);
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::BeginDisabled(borradorReceta.empty());
            if (ImGui::Button("Guardar Receta en Sistema y JSON"))
            {
                ProductoPanaderia* prodDest = productos[static_cast<size_t>(idxProductoReceta)];
                Receta* nuevaReceta = new Receta(*prodDest);
                NodoReceta* raiz = nuevaReceta->getRaiz();

                // Primero registrar los nombres y nodos directos / subproductos
                for (const auto& b : borradorReceta)
                {
                    if (!b.nombre.empty())
                    {
                        sistema.registrarNombreSKU(b.sku, b.nombre);
                    }
                    if (b.esSubproducto || b.skuPadre.empty())
                    {
                        nuevaReceta->agregarElemento(raiz, ElementoReceta(b.sku, b.cantidad, b.unidad, b.rendimiento));
                    }
                }
                // Luego adjuntar hojas que cuelgan de un subproducto
                for (const auto& b : borradorReceta)
                {
                    if (!b.esSubproducto && !b.skuPadre.empty())
                    {
                        NodoReceta* padre = nuevaReceta->buscarElemento(b.skuPadre);
                        nuevaReceta->agregarElemento(padre != nullptr ? padre : raiz,
                                                     ElementoReceta(b.sku, b.cantidad, b.unidad));
                    }
                }

                if (sistema.existeReceta(prodDest->getSKU()))
                {
                    sistema.eliminarReceta(prodDest->getSKU());
                }
                if (sistema.registrarReceta(nuevaReceta, true))
                {
                    sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    mensajeEstado = "[+] Receta guardada para '" + prodDest->getNombre() +
                                    "' (" + prodDest->getSKU() + ") y sincronizada en JSON.";
                    estadoExito = true;
                    borradorReceta.clear();
                    idxPadreReceta = 0;
                }
                else
                {
                    delete nuevaReceta;
                    mensajeEstado = "[!] Error al registrar la receta.";
                    estadoExito = false;
                }
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Limpiar Borrador"))
            {
                borradorReceta.clear();
                idxPadreReceta = 0;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("3. Eliminar Producto del Catalogo");
        if (!productos.is_empty())
        {
            if (idxProductoEliminar >= static_cast<int>(productos.size()))
            {
                idxProductoEliminar = 0;
            }
            ProductoPanaderia* prodDel = productos[static_cast<size_t>(idxProductoEliminar)];
            char previewDel[192];
            std::snprintf(previewDel, sizeof(previewDel), "[%d] %s - %s",
                          idxProductoEliminar + 1, prodDel->getSKU().c_str(), prodDel->getNombre().c_str());

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.56f);
            if (ImGui::BeginCombo("Producto a eliminar", previewDel))
            {
                for (size_t i = 0; i < productos.size(); ++i)
                {
                    char itemLbl[192];
                    std::snprintf(itemLbl, sizeof(itemLbl), "[%zu] %s - %s",
                                  i + 1, productos[i]->getSKU().c_str(), productos[i]->getNombre().c_str());
                    if (ImGui::Selectable(itemLbl, static_cast<int>(i) == idxProductoEliminar))
                    {
                        idxProductoEliminar = static_cast<int>(i);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (ImGui::Button("Eliminar O(1)"))
            {
                std::string skuDel = prodDel->getSKU();
                std::string nomDel = prodDel->getNombre();
                if (sistema.eliminarProducto(skuDel))
                {
                    sistema.eliminarReceta(skuDel);
                    sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    mensajeEstado = "[+] Eliminado: " + nomDel + " (" + skuDel + ").";
                    estadoExito = true;
                    idxProductoEliminar = 0;
                }
            }
        }

        if (!mensajeEstado.empty())
        {
            ImGui::Spacing();
            ImVec4 color = estadoExito
                ? ImVec4(0.10f, 0.50f, 0.20f, 1.0f)
                : ImVec4(0.78f, 0.12f, 0.12f, 1.0f);
            ImGui::TextColored(color, "%s", mensajeEstado.c_str());
        }

        ImGui::EndChild();
    }

    /**
     * Dibuja recursivamente el Árbol N-ario de Receta resolviendo el nombre de cada SKU en O(1).
     */
    void renderNodoArbolReceta(const SistemaPanaderia& sistema, NodoReceta* nodo, bool esRaiz)
    {
        if (nodo == nullptr)
        {
            return;
        }

        const ElementoReceta& elem = nodo->get_key();
        std::string nombreItem = sistema.obtenerNombreSKU(elem.getCodigoSKU());
        char etiqueta[256];

        if (esRaiz)
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[RAIZ] %s (%s) | Base: %.2f %s | Rendimiento: %.0f%%",
                          elem.getCodigoSKU().c_str(),
                          nombreItem.c_str(),
                          elem.getCantidad(),
                          elem.getUnidadMedida().c_str(),
                          elem.getFactorRendimiento() * 100.0);
        }
        else if (nodo->is_leaf())
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[HOJA] %s (%s) | Proporcion: %.3f %s",
                          elem.getCodigoSKU().c_str(),
                          nombreItem.c_str(),
                          elem.getCantidad(),
                          elem.getUnidadMedida().c_str());
        }
        else
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[SUBPRODUCTO] %s (%s) | Proporcion: %.2f %s",
                          elem.getCodigoSKU().c_str(),
                          nombreItem.c_str(),
                          elem.getCantidad(),
                          elem.getUnidadMedida().c_str());
        }

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
        if (nodo->is_leaf())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(nodo, flags, "%s", etiqueta);
            return;
        }

        bool abierto = ImGui::TreeNodeEx(nodo, flags, "%s", etiqueta);
        if (abierto)
        {
            for (NodoReceta::ChildrenIterator it = NodoReceta::ChildrenIterator(*nodo);
                 it.has_current(); it.next())
            {
                renderNodoArbolReceta(sistema, it.get_current(), false);
            }
            ImGui::TreePop();
        }
    }

    /**
     * Pestaña 3: Cocina y Explosión de Receta (Previsualización DFS y Ejecución en Kardex + Stock + Lote).
     */
    void renderPestanaCocina(SistemaPanaderia& sistema)
    {
        static char skuReceta[64] = "PAN-JAM-001";
        static float cantidadKg = 10.0f;
        static Designar::DynArray<ElementoReceta> insumosResultantes;
        static std::string ultimoSkuCalculado;
        static double ultimaCantidadCalculada = 0.0;
        static bool pedidoProcesado = false;
        static bool ultimoExito = false;
        static std::string mensajeOrden;

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Orden de Produccion para Cocina (Hash O(1) -> DFS O(n) -> Kardex AVL)");
        ImGui::TextWrapped("Seleccione un producto para previsualizar el desglose DFS con nombres "
                           "resueltos por SKU, o ejecute la orden para descontar en Kardex AVL, "
                           "sumar stock y crear su lote de trazabilidad.");
        ImGui::Spacing();

        const float em = ImGui::GetFontSize();
        const Designar::DynArray<ProductoPanaderia*>& productos = sistema.obtenerListaProductos();
        if (!productos.is_empty())
        {
            std::string previewProd = std::string(skuReceta) + " - " + sistema.obtenerNombreSKU(skuReceta);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.42f);
            if (ImGui::BeginCombo("Producto", previewProd.c_str()))
            {
                for (size_t i = 0; i < productos.size(); ++i)
                {
                    const ProductoPanaderia* p = productos[i];
                    char itemLbl[192];
                    std::snprintf(itemLbl, sizeof(itemLbl), "[%zu] %s - %s (Receta: %s)",
                                  i + 1, p->getSKU().c_str(), p->getNombre().c_str(),
                                  sistema.existeReceta(p->getSKU()) ? "SI" : "NO");
                    if (ImGui::Selectable(itemLbl, std::string(skuReceta) == p->getSKU()))
                    {
                        std::snprintf(skuReceta, sizeof(skuReceta), "%s", p->getSKU().c_str());
                        pedidoProcesado = false;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
        }

        ImGui::SetNextItemWidth(9.0f * em);
        ImGui::InputText("SKU", skuReceta, sizeof(skuReceta));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(7.5f * em);
        ImGui::InputFloat("Lote (kg)", &cantidadKg, 1.0f, 5.0f, "%.2f");
        if (cantidadKg < 0.1f)
        {
            cantidadKg = 0.1f;
        }

        if (ImGui::Button("Previsualizar Explosion DFS") || !pedidoProcesado)
        {
            insumosResultantes = Designar::DynArray<ElementoReceta>();
            ultimoSkuCalculado = std::string(skuReceta);
            ultimaCantidadCalculada = static_cast<double>(cantidadKg);
            Receta* rec = sistema.buscarReceta(ultimoSkuCalculado);
            if (rec != nullptr)
            {
                rec->calcularMateriasPrimas(ultimaCantidadCalculada, insumosResultantes);
                ultimoExito = true;
            }
            else
            {
                ultimoExito = false;
            }
            pedidoProcesado = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Ejecutar Orden (Registrar Egresos Kardex + Stock + Lote)"))
        {
            insumosResultantes = Designar::DynArray<ElementoReceta>();
            ultimoSkuCalculado = std::string(skuReceta);
            ultimaCantidadCalculada = static_cast<double>(cantidadKg);
            ultimoExito = sistema.procesarPedidoProduccion(
                ultimoSkuCalculado, ultimaCantidadCalculada, insumosResultantes
            );
            pedidoProcesado = true;

            if (ultimoExito)
            {
                ProductoPanaderia* prod = sistema.buscarProducto(ultimoSkuCalculado);
                int unids = static_cast<int>(std::llround(ultimaCantidadCalculada));
                if (unids < 1)
                {
                    unids = 1;
                }
                if (prod != nullptr)
                {
                    prod->aumentarStock(unids);
                }

                fecha hoy = obtenerFechaActual();
                char codLote[48];
                std::snprintf(codLote, sizeof(codLote), "LOT-%04d%02d%02d-%03zu",
                              hoy.anio, hoy.mes, hoy.dia, sistema.getLotes().size() + 1);
                LoteProduccion lote(codLote, ultimoSkuCalculado, ultimaCantidadCalculada, hoy);
                lote.registrarEvento("Orden de Produccion",
                                     "Explosion DFS y egreso de materias primas al Kardex",
                                     "Sistema GUI / Cocina", hoy);
                sistema.registrarLote(lote);
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);

                mensajeOrden = "[+] Orden ejecutada: " + std::to_string(insumosResultantes.size()) +
                               " egresos en Kardex AVL | Lote: " + std::string(codLote);
            }
            else
            {
                mensajeOrden.clear();
            }
        }

        if (!mensajeOrden.empty())
        {
            ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f), "%s", mensajeOrden.c_str());
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (!ultimoExito)
        {
            ImGui::TextColored(ImVec4(0.78f, 0.12f, 0.12f, 1.0f),
                               "[!] No existe una receta registrada para el SKU '%s'. "
                               "Puede agregarle una en la Pestana 2.",
                               ultimoSkuCalculado.c_str());
            return;
        }

        Receta* recetaActiva = sistema.buscarReceta(ultimoSkuCalculado);
        ProductoPanaderia* prodActivo = sistema.buscarProducto(ultimoSkuCalculado);

        float anchoIzq = (ImGui::GetContentRegionAvail().x - 16.0f) * 0.46f;

        // Panel izquierdo: Árbol N-ario interactivo
        ImGui::BeginChild("PanelArbolReceta", ImVec2(anchoIzq, 0.0f),
                          ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Estructura del Arbol de Receta (MTreeNode)");
        ImGui::Separator();
        ImGui::Spacing();
        if (recetaActiva != nullptr && recetaActiva->getRaiz() != nullptr)
        {
            renderNodoArbolReceta(sistema, recetaActiva->getRaiz(), true);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Panel derecho: Reporte consolidado de materias primas con Nombre O(1)
        ImGui::BeginChild("PanelOrdenCocina", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Orden Consolidada para Cocina");
        ImGui::Separator();
        ImGui::Spacing();

        if (prodActivo != nullptr)
        {
            ImGui::Text("Producto: %s (%s)",
                        prodActivo->getNombre().c_str(),
                        prodActivo->getSKU().c_str());
            ImGui::Text("Rendimiento: %.0f %% | Lote: %.2f kg | Stock actual: %d",
                        prodActivo->getRendimientoMasa() * 100.0,
                        ultimaCantidadCalculada,
                        prodActivo->getStockActual());
        }
        else
        {
            ImGui::Text("Producto: %s (%s) | Lote: %.2f kg",
                        sistema.obtenerNombreSKU(ultimoSkuCalculado).c_str(),
                        ultimoSkuCalculado.c_str(),
                        ultimaCantidadCalculada);
        }
        ImGui::Spacing();

        if (ImGui::BeginTable("TablaInsumosCocina", 5,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                              ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 1.3f)))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 2.4f * em);
            ImGui::TableSetupColumn("SKU Insumo", ImGuiTableColumnFlags_WidthFixed, 7.6f * em);
            ImGui::TableSetupColumn("Nombre (Hash O(1))", ImGuiTableColumnFlags_WidthFixed, 13.5f * em);
            ImGui::TableSetupColumn("Cantidad", ImGuiTableColumnFlags_WidthFixed, 6.2f * em);
            ImGui::TableSetupColumn("Unid.", ImGuiTableColumnFlags_WidthFixed, 4.0f * em);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < insumosResultantes.size(); ++i)
            {
                const ElementoReceta& item = insumosResultantes[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%zu", i + 1);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(item.getCodigoSKU().c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(sistema.obtenerNombreSKU(item.getCodigoSKU()).c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.4f", item.getCantidad());

                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(item.getUnidadMedida().c_str());
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f),
                           "Total de insumos elementales consolidados: %zu",
                           insumosResultantes.size());
        ImGui::EndChild();
    }

    /**
     * Pestaña 4: Trazabilidad de Lotes (SLList) y Perecederos FEFO (Min-Heap) integrados en SistemaPanaderia.
     */
    void renderPestanaTrazabilidadYPerecederos(SistemaPanaderia& sistema)
    {
        static int idxLoteSeleccionado = 0;
        static char etapaInput[64] = "";
        static char descInput[160] = "";
        static char respInput[64] = "";
        static Designar::DynArray<ProductoPanaderia> historialDespachosFefo;

        static char skuPer[64] = "";
        static char nomPer[128] = "";
        static int stockPer = 20;
        static int diaPer = 28;
        static int mesPer = 11;
        static int anioPer = 2026;
        static float costoPer = 3.00f;

        const float em = ImGui::GetFontSize();
        float anchoIzq = (ImGui::GetContentRegionAvail().x - 14.0f) * 0.54f;

        // =====================================================================
        // COLUMNA IZQUIERDA: TRAZABILIDAD DE LOTES (SLList)
        // =====================================================================
        ImGui::BeginChild("PanelTrazabilidadSLList", ImVec2(anchoIzq, 0.0f), ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Trazabilidad de Lotes (Designar::SLList)");

        Designar::DynArray<LoteProduccion>& lotes = sistema.getLotes();
        if (lotes.is_empty())
        {
            ImGui::TextDisabled("No hay lotes registrados.");
        }
        else
        {
            if (idxLoteSeleccionado >= static_cast<int>(lotes.size()))
            {
                idxLoteSeleccionado = 0;
            }
            LoteProduccion& lote = lotes[static_cast<size_t>(idxLoteSeleccionado)];
            std::string nomProdLote = sistema.obtenerNombreSKU(lote.getCodigoSKU());

            char previewLote[192];
            std::snprintf(previewLote, sizeof(previewLote), "[%d] %s | %s (%s)",
                          idxLoteSeleccionado + 1,
                          lote.getCodigoLote().c_str(),
                          lote.getCodigoSKU().c_str(),
                          nomProdLote.c_str());

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.72f);
            if (ImGui::BeginCombo("Lote activo", previewLote))
            {
                for (size_t i = 0; i < lotes.size(); ++i)
                {
                    const LoteProduccion& l = lotes[i];
                    char itemLbl[192];
                    std::snprintf(itemLbl, sizeof(itemLbl), "[%zu] %s | %s (%s)",
                                  i + 1, l.getCodigoLote().c_str(),
                                  l.getCodigoSKU().c_str(),
                                  sistema.obtenerNombreSKU(l.getCodigoSKU()).c_str());
                    if (ImGui::Selectable(itemLbl, static_cast<int>(i) == idxLoteSeleccionado))
                    {
                        idxLoteSeleccionado = static_cast<int>(i);
                    }
                }
                ImGui::EndCombo();
            }

            LoteProduccion& loteAct = lotes[static_cast<size_t>(idxLoteSeleccionado)];
            ImGui::TextWrapped("Producto: %s (%s) | Cantidad: %.1f kg | Eventos: %zu",
                               sistema.obtenerNombreSKU(loteAct.getCodigoSKU()).c_str(),
                               loteAct.getCodigoSKU().c_str(),
                               loteAct.getCantidadPlanificada(),
                               loteAct.getNumeroEventos());
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTable("TablaTrazabilidad", 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                                  ImVec2(0.0f, 11.5f * em)))
            {
                ImGui::TableSetupColumn("Fecha", ImGuiTableColumnFlags_WidthFixed, 6.6f * em);
                ImGui::TableSetupColumn("Etapa", ImGuiTableColumnFlags_WidthFixed, 8.8f * em);
                ImGui::TableSetupColumn("Descripcion", ImGuiTableColumnFlags_WidthFixed, 21.0f * em);
                ImGui::TableSetupColumn("Responsable", ImGuiTableColumnFlags_WidthFixed, 10.0f * em);
                ImGui::TableHeadersRow();

                for (const auto& ev : loteAct.getEventos())
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%02d/%02d/%04d", ev.fechaEvento.dia, ev.fechaEvento.mes, ev.fechaEvento.anio);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(ev.etapa.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextWrapped("%s", ev.descripcion.c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(ev.responsable.c_str());
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Text("Registrar nuevo evento de auditoria:");
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.62f);
            ImGui::InputText("Etapa", etapaInput, sizeof(etapaInput));
            ImGui::InputText("Descripcion", descInput, sizeof(descInput));
            ImGui::InputText("Responsable", respInput, sizeof(respInput));
            ImGui::PopItemWidth();

            if (ImGui::Button("Agregar Evento a SLList"))
            {
                if (etapaInput[0] != '\0' && descInput[0] != '\0' && respInput[0] != '\0')
                {
                    loteAct.registrarEvento(etapaInput, descInput, respInput, obtenerFechaActual());
                    sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                    etapaInput[0] = '\0';
                    descInput[0] = '\0';
                    respInput[0] = '\0';
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // =====================================================================
        // COLUMNA DERECHA: PERECEDEROS FEFO (Min-Heap en SistemaPanaderia)
        // =====================================================================
        ImGui::BeginChild("PanelPerecederosMinHeap", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Gestion FEFO de Perecederos (Min-Heap)");
        ImGui::Text("Insumos esperando en el Min-Heap: %zu", sistema.totalPerecederos());
        ImGui::Separator();
        ImGui::Spacing();

        if (sistema.tienePerecederos())
        {
            ProductoPanaderia prox = sistema.consultarProximoAVencer();
            fecha f = prox.getFechaVencimiento();
            ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.05f, 1.0f),
                               "Cima del Min-Heap (Proximo a vencer):");
            ImGui::BulletText("%s (SKU: %s)", prox.getNombre().c_str(), prox.getSKU().c_str());
            ImGui::BulletText("Fecha de vencimiento: %02d/%02d/%04d", f.dia, f.mes, f.anio);
            ImGui::Spacing();

            if (ImGui::Button("Despachar Proximo a Vencer (FEFO)"))
            {
                ProductoPanaderia desp = sistema.despacharProximoAVencer();
                historialDespachosFefo.append(desp);
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
            }
        }
        else
        {
            ImGui::TextDisabled("El almacen de perecederos esta vacio.");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Ingresar nuevo perecedero:");
        ImGui::SetNextItemWidth(10.0f * em);
        ImGui::InputText("SKU Perecedero", skuPer, sizeof(skuPer));
        ImGui::SetNextItemWidth(14.0f * em);
        ImGui::InputText("Nombre Perecedero", nomPer, sizeof(nomPer));
        ImGui::SetNextItemWidth(5.8f * em);
        ImGui::InputInt("Cant.", &stockPer);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(5.8f * em);
        ImGui::InputFloat("Costo ($)", &costoPer, 0.5f, 1.0f, "%.2f");

        ImGui::SetNextItemWidth(3.8f * em);
        ImGui::InputInt("Dia", &diaPer, 0, 0);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(3.8f * em);
        ImGui::InputInt("Mes", &mesPer, 0, 0);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(4.8f * em);
        ImGui::InputInt("Anio", &anioPer, 0, 0);

        if (ImGui::Button("Agregar al Min-Heap"))
        {
            std::string sSku(skuPer);
            std::string sNom(nomPer);
            if (!sSku.empty())
            {
                if (!sNom.empty())
                {
                    sistema.registrarNombreSKU(sSku, sNom);
                }
                std::string nomFinal = sNom.empty() ? sistema.obtenerNombreSKU(sSku) : sNom;
                ProductoPanaderia nuevoPer(sSku, nomFinal, TipoMasaBase::BLANDA_ENRIQUECIDA,
                                           0, 0, 0, 0, 0, 1.0, stockPer, 5,
                                           {diaPer, mesPer, anioPer}, static_cast<double>(costoPer));
                sistema.registrarPerecedero(nuevoPer);
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                skuPer[0] = '\0';
                nomPer[0] = '\0';
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Despachados en esta sesion (%zu):", historialDespachosFefo.size());

        if (ImGui::BeginTable("TablaDespachoFEFO", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                              ImVec2(0.0f, 7.5f * em)))
        {
            ImGui::TableSetupColumn("Orden", ImGuiTableColumnFlags_WidthFixed, 3.8f * em);
            ImGui::TableSetupColumn("Vencimiento", ImGuiTableColumnFlags_WidthFixed, 6.8f * em);
            ImGui::TableSetupColumn("Insumo", ImGuiTableColumnFlags_WidthFixed, 15.0f * em);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < historialDespachosFefo.size(); ++i)
            {
                const ProductoPanaderia& p = historialDespachosFefo[i];
                fecha f = p.getFechaVencimiento();
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%zu", i + 1);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%02d/%02d/%04d", f.dia, f.mes, f.anio);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s (%s)", p.getNombre().c_str(), p.getSKU().c_str());
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    /**
     * Pestaña 5: Kardex de Inventario (Árbol AVL) con consulta por rango de fechas y registro manual.
     */
    void renderPestanaKardex(SistemaPanaderia& sistema)
    {
        static int dIni = 1, mIni = 11, aIni = 2026;
        static int dFin = 31, mFin = 12, aFin = 2026;

        static char movSku[64] = "MP-HAR-001";
        static char movNombre[128] = "";
        static int movTipoIdx = 0;
        static float movCant = 25.0f;
        static char movDesc[160] = "Compra / reposicion de inventario";
        static std::string msgKardex;

        const float em = ImGui::GetFontSize();

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Kardex de Inventario (Arbol AVL auto-balanceado)");
        ImGui::Text("Total de movimientos en AVL: %zu | Estado: %s",
                    sistema.getKardex().totalMovimientos(),
                    sistema.getKardex().verificarBalanceoAVL() ? "BALANCEADO (OK)" : "DESBALANCEADO");
        ImGui::Spacing();

        // Filtros de rango compactos y adaptativos
        ImGui::TextUnformatted("Desde:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(3.2f * em); ImGui::InputInt("##dIni", &dIni, 0, 0); ImGui::SameLine();
        ImGui::TextUnformatted("/"); ImGui::SameLine();
        ImGui::SetNextItemWidth(3.2f * em); ImGui::InputInt("##mIni", &mIni, 0, 0); ImGui::SameLine();
        ImGui::TextUnformatted("/"); ImGui::SameLine();
        ImGui::SetNextItemWidth(4.5f * em); ImGui::InputInt("##aIni", &aIni, 0, 0); ImGui::SameLine();
        ImGui::TextUnformatted("  Hasta:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(3.2f * em); ImGui::InputInt("##dFin", &dFin, 0, 0); ImGui::SameLine();
        ImGui::TextUnformatted("/"); ImGui::SameLine();
        ImGui::SetNextItemWidth(3.2f * em); ImGui::InputInt("##mFin", &mFin, 0, 0); ImGui::SameLine();
        ImGui::TextUnformatted("/"); ImGui::SameLine();
        ImGui::SetNextItemWidth(4.5f * em); ImGui::InputInt("##aFin", &aFin, 0, 0); ImGui::SameLine();
        if (ImGui::Button("Ver Todo"))
        {
            dIni = 1; mIni = 1; aIni = 2020;
            dFin = 31; mFin = 12; aFin = 2030;
        }

        fecha fIni{dIni, mIni, aIni};
        fecha fFin{dFin, mFin, aFin};
        Designar::DynArray<MovimientoKardex> movs =
            sistema.getKardex().consultarPorRangoFechas(fIni, fFin);

        ImGui::Spacing();
        float altoTablaKardex = -ImGui::GetFrameHeightWithSpacing() * 5.2f;
        if (ImGui::BeginTable("TablaKardexAVL", 7,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable,
                              ImVec2(0.0f, altoTablaKardex)))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 2.4f * em);
            ImGui::TableSetupColumn("Fecha", ImGuiTableColumnFlags_WidthFixed, 6.6f * em);
            ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 9.8f * em);
            ImGui::TableSetupColumn("SKU", ImGuiTableColumnFlags_WidthFixed, 7.6f * em);
            ImGui::TableSetupColumn("Nombre (Hash O(1))", ImGuiTableColumnFlags_WidthFixed, 13.5f * em);
            ImGui::TableSetupColumn("Cant.", ImGuiTableColumnFlags_WidthFixed, 5.2f * em);
            ImGui::TableSetupColumn("Concepto", ImGuiTableColumnFlags_WidthFixed, 24.0f * em);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < movs.size(); ++i)
            {
                const MovimientoKardex& m = movs[i];
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%zu", i + 1);
                ImGui::TableSetColumnIndex(1); ImGui::Text("%02d/%02d/%04d", m.fechaTransaccion.dia, m.fechaTransaccion.mes, m.fechaTransaccion.anio);
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(tipoMovimientoToString(m.tipo).c_str());
                ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(m.codigoSKU.c_str());
                ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(sistema.obtenerNombreSKU(m.codigoSKU).c_str());
                ImGui::TableSetColumnIndex(5); ImGui::Text("%.3f", m.cantidad);
                ImGui::TableSetColumnIndex(6); ImGui::TextUnformatted(m.descripcion.c_str());
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Registrar Movimiento Manual en Kardex AVL:");
        ImGui::SetNextItemWidth(9.0f * em);
        ImGui::InputText("SKU Item", movSku, sizeof(movSku));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(12.5f * em);
        ImGui::InputText("Nombre (SKU nuevo)", movNombre, sizeof(movNombre));
        ImGui::SameLine();
        const char* tiposMov[] = {"Entrada (Compra)", "Ajuste (Inventario)"};
        ImGui::SetNextItemWidth(10.5f * em);
        ImGui::Combo("Tipo", &movTipoIdx, tiposMov, 2);

        ImGui::SetNextItemWidth(7.2f * em);
        ImGui::InputFloat("Cantidad", &movCant, 1.0f, 5.0f, "%.2f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(17.5f * em);
        ImGui::InputText("Concepto", movDesc, sizeof(movDesc));
        ImGui::SameLine();
        if (ImGui::Button("Registrar en AVL"))
        {
            std::string sSku(movSku);
            if (!sSku.empty() && movCant > 0.0f)
            {
                if (movNombre[0] != '\0')
                {
                    sistema.registrarNombreSKU(sSku, std::string(movNombre));
                }
                TipoMovimiento tipo = (movTipoIdx == 1)
                                          ? TipoMovimiento::AJUSTE_INVENTARIO
                                          : TipoMovimiento::ENTRADA_COMPRA;
                sistema.registrarMovimientoInventario({
                    obtenerFechaActual(), sSku, tipo, static_cast<double>(movCant), std::string(movDesc)
                });
                ProductoPanaderia* prod = sistema.buscarProducto(sSku);
                if (prod != nullptr && tipo == TipoMovimiento::ENTRADA_COMPRA)
                {
                    prod->aumentarStock(static_cast<int>(std::llround(movCant)));
                }
                sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
                msgKardex = "[+] Movimiento registrado para " + sSku + " (" + sistema.obtenerNombreSKU(sSku) + ").";
                movNombre[0] = '\0';
            }
        }
        if (!msgKardex.empty())
        {
            ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f), "%s", msgKardex.c_str());
        }
    }

    /**
     * Dibuja la ventana principal anclada y las 5 pestañas de la aplicación.
     */
    void renderizarFramePanaderia(
        ImGuiIO& io,
        SistemaPanaderia& sistema,
        char* skuBusquedaPos,
        size_t skuBusquedaPosTam,
        bool& busquedaPosEjecutada)
    {
        static std::string estadoPersistencia = "JSON sincronizado: " + RUTA_PERSISTENCIA_JSON;

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGuiWindowFlags flagsPanel =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("BakerySystemMain", nullptr, flagsPanel);

        ImGui::TextColored(ImVec4(0.55f, 0.28f, 0.06f, 1.0f),
                           "BAKERY SYSTEM - GESTION INTEGRAL DE PANADERIA");
        ImGui::SameLine();
        if (ImGui::SmallButton("A-"))
        {
            io.FontGlobalScale = (io.FontGlobalScale > 0.85f) ? (io.FontGlobalScale - 0.10f) : 0.85f;
        }
        ImGui::SameLine();
        char lblZoom[24];
        std::snprintf(lblZoom, sizeof(lblZoom), "%.0f%%", io.FontGlobalScale * 100.0f);
        if (ImGui::SmallButton(lblZoom))
        {
            io.FontGlobalScale = 1.0f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("A+"))
        {
            io.FontGlobalScale = (io.FontGlobalScale < 1.65f) ? (io.FontGlobalScale + 0.10f) : 1.65f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Recargar JSON"))
        {
            if (sistema.cargarDatosJSON(RUTA_PERSISTENCIA_JSON))
            {
                estadoPersistencia = "Recargado desde " + RUTA_PERSISTENCIA_JSON;
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Guardar JSON"))
        {
            if (sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON))
            {
                estadoPersistencia = "Guardado en " + RUTA_PERSISTENCIA_JSON;
            }
        }
        ImGui::TextDisabled("%s", estadoPersistencia.c_str());

        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("MenuPrincipalTabs"))
        {
            if (ImGui::BeginTabItem("1. Punto de Venta (POS)"))
            {
                ImGui::Spacing();
                renderPestanaPOS(sistema, skuBusquedaPos, skuBusquedaPosTam, busquedaPosEjecutada);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("2. Catalogo y Recetas"))
            {
                ImGui::Spacing();
                renderPestanaCatalogoYRecetas(sistema);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("3. Cocina y Explosion DFS"))
            {
                ImGui::Spacing();
                renderPestanaCocina(sistema);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("4. Lotes y Perecederos"))
            {
                ImGui::Spacing();
                renderPestanaTrazabilidadYPerecederos(sistema);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("5. Kardex Inventario (AVL)"))
            {
                ImGui::Spacing();
                renderPestanaKardex(sistema);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
} // namespace

#ifdef _WIN32
struct WGL_WindowData { HDC hDC; };

static HGLRC          g_hRC = nullptr;
static WGL_WindowData g_MainWindow = {nullptr};
static int            g_Width = 1280;
static int            g_Height = 820;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
    {
        ::ReleaseDC(hWnd, hDc);
        return false;
    }
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
    {
        ::ReleaseDC(hWnd, hDc);
        return false;
    }
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
    {
        g_hRC = wglCreateContext(data->hDC);
    }
    return g_hRC != nullptr;
}

static void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    if (data->hDC != nullptr)
    {
        ::ReleaseDC(hWnd, data->hDC);
        data->hDC = nullptr;
    }
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                g_Width = LOWORD(lParam);
                g_Height = HIWORD(lParam);
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
            {
                return 0;
            }
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
#endif

int main(int /*argc*/, char* /*argv*/[])
{
    // =========================================================================
    // 1. INICIALIZACIÓN UNIFICADA CON PERSISTENCIA JSON (COMPARTIDA CON CLI)
    // =========================================================================
    SistemaPanaderia sistema;
    sistema.inicializarConPersistencia(RUTA_PERSISTENCIA_JSON);

    char skuBusquedaPos[64] = "PAN-JAM-001";
    const auto& listaInicial = sistema.obtenerListaProductos();
    if (!listaInicial.is_empty() && listaInicial[0] != nullptr)
    {
        std::snprintf(skuBusquedaPos, sizeof(skuBusquedaPos), "%s", listaInicial[0]->getSKU().c_str());
    }
    bool busquedaPosEjecutada = true;

#ifdef _WIN32
    // =========================================================================
    // 2. VENTANA NATIVA WINDOWS (WIN32 + WGL OPENGL3, SIN DEPENDENCIAS EXTERNAS)
    // =========================================================================
    WNDCLASSEXW wc = {
        sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr, nullptr, L"BakerySystemGUI", nullptr
    };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName,
        L"Bakery System - Gestion Panadera ULA (Programacion III)",
        WS_OVERLAPPEDWINDOW,
        100, 100, g_Width, g_Height,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    configurarTipografiaLegible(io);
    aplicarEstiloPanaderia();

    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    // =========================================================================
    // 3. BUCLE PRINCIPAL DE RENDERIZADO WIN32
    // =========================================================================
    bool ejecutando = true;
    while (ejecutando)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                ejecutando = false;
            }
        }
        if (!ejecutando)
        {
            break;
        }
        if (::IsIconic(hwnd))
        {
            ::Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        renderizarFramePanaderia(
            io, sistema, skuBusquedaPos, sizeof(skuBusquedaPos), busquedaPosEjecutada
        );

        ImGui::Render();
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(0.95f, 0.92f, 0.87f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ::SwapBuffers(g_MainWindow.hDC);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
#else
    // =========================================================================
    // 2. VENTANA LINUX / MACOS (SDL2 + OPENGL3 + DEAR IMGUI)
    // =========================================================================
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cerr << "[ERROR] No se pudo inicializar SDL2: " << SDL_GetError() << "\n";
        return 1;
    }

#if defined(__APPLE__)
    const char* glslVersion = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    const char* glslVersion = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags windowFlags = static_cast<SDL_WindowFlags>(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    SDL_Window* window = SDL_CreateWindow(
        "Bakery System - Gestion Panadera ULA (Programacion III)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        820,
        windowFlags
    );

    if (window == nullptr)
    {
        std::cerr << "[ERROR] No se pudo crear la ventana SDL2: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    configurarTipografiaLegible(io);
    aplicarEstiloPanaderia();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // =========================================================================
    // 3. BUCLE PRINCIPAL DE RENDERIZADO SDL2 (60 FPS)
    // =========================================================================
    bool ejecutando = true;
    while (ejecutando)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                ejecutando = false;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                ejecutando = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        renderizarFramePanaderia(
            io, sistema, skuBusquedaPos, sizeof(skuBusquedaPos), busquedaPosEjecutada
        );

        ImGui::Render();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClearColor(0.95f, 0.92f, 0.87f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif

    // Guardar estado al cerrar (la memoria es liberada por ~SistemaPanaderia via RAII)
    sistema.guardarDatosJSON(RUTA_PERSISTENCIA_JSON);
    return 0;
}
