#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

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
        style.FramePadding = ImVec2(10.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);

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
     * Dibuja botones de acceso rápido para seleccionar cualquiera de los SKUs registrados.
     */
    void renderBotonesSkusRapidos(const Designar::DynArray<std::string>& listaSkus,
                                  char* bufferDestino, size_t bufferTam,
                                  const char* idGrupo)
    {
        ImGui::TextDisabled("SKUs registrados en Tabla Hash:");
        ImGui::PushID(idGrupo);
        for (size_t i = 0; i < listaSkus.size(); ++i)
        {
            if (i > 0 && (i % 5 != 0))
            {
                ImGui::SameLine();
            }
            if (ImGui::SmallButton(listaSkus[i].c_str()))
            {
                std::snprintf(bufferDestino, bufferTam, "%s", listaSkus[i].c_str());
            }
        }
        ImGui::PopID();
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
     * Pestaña 1: Punto de Venta (POS) — Búsqueda instantánea por SKU en O(1).
     */
    void renderPestanaPOS(SistemaPanaderia& sistema,
                          const Designar::DynArray<std::string>& listaSkusCatalogo,
                          char* skuBusqueda, size_t skuBusquedaTam,
                          bool& busquedaEjecutada)
    {
        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Consulta Rapida en Punto de Venta (Tabla Hash O(1))");
        ImGui::TextWrapped("Ingrese o seleccione un SKU para consultar instantaneamente "
                           "la ficha tecnica, rendimiento, vencimiento y stock del producto.");
        ImGui::Spacing();

        renderBotonesSkusRapidos(listaSkusCatalogo, skuBusqueda, skuBusquedaTam, "pos_skus");
        ImGui::Spacing();

        ImGui::SetNextItemWidth(260.0f);
        if (ImGui::InputText("SKU a consultar", skuBusqueda, skuBusquedaTam,
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            busquedaEjecutada = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Buscar en Tabla Hash O(1)"))
        {
            busquedaEjecutada = true;
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (!busquedaEjecutada)
        {
            ImGui::TextDisabled("Presione 'Buscar en Tabla Hash O(1)' para consultar un producto.");
            return;
        }

        ProductoPanaderia* prod = sistema.buscarProducto(std::string(skuBusqueda));
        if (prod == nullptr)
        {
            ImGui::TextColored(ImVec4(0.78f, 0.12f, 0.12f, 1.0f),
                               "[!] No se encontro ningun producto registrado con el SKU: %s",
                               skuBusqueda);
            return;
        }

        ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f),
                           "[+] Producto localizado en tiempo promedio O(1): %s",
                           prod->getNombre().c_str());
        ImGui::Spacing();

        if (ImGui::BeginTable("TablaFichaProducto", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Campo", ImGuiTableColumnFlags_WidthFixed, 220.0f);
            ImGui::TableSetupColumn("Detalle", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            filaDetalleProducto("Codigo SKU", prod->getSKU());
            filaDetalleProducto("Nombre del Producto", prod->getNombre());
            filaDetalleProducto("Tipo de Masa Base", prod->getTipoMasaNombre());

            char buf[128];
            std::snprintf(buf, sizeof(buf), "%.1f %%", prod->getPorcentajeHidratacion());
            filaDetalleProducto("Porcentaje de Hidratacion", buf);

            std::snprintf(buf, sizeof(buf), "%d min a %.1f C",
                          prod->getTiempoFermentacion(), prod->getTemperaturaFermentacion());
            filaDetalleProducto("Fermentacion", buf);

            std::snprintf(buf, sizeof(buf), "%d min a %d C",
                          prod->getTiempoHorneado(), prod->getTemperaturaHorneado());
            filaDetalleProducto("Horneado", buf);

            std::snprintf(buf, sizeof(buf), "%.0f %% (merma: %.0f %%)",
                          prod->getRendimientoMasa() * 100.0,
                          (1.0 - prod->getRendimientoMasa()) * 100.0);
            filaDetalleProducto("Rendimiento de Masa", buf);

            fecha fv = prod->getFechaVencimiento();
            std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d", fv.dia, fv.mes, fv.anio);
            filaDetalleProducto("Fecha de Vencimiento", buf);

            std::snprintf(buf, sizeof(buf), "$ %.2f", prod->getCostoUnitario());
            filaDetalleProducto("Costo Unitario", buf);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Stock Actual / Minimo");
            ImGui::TableSetColumnIndex(1);
            if (prod->requiereReabastecimiento())
            {
                ImGui::TextColored(ImVec4(0.80f, 0.15f, 0.15f, 1.0f),
                                   "%d / %d unidades (REQUIERE REABASTECIMIENTO)",
                                   prod->getStockActual(), prod->getStockMinimo());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.10f, 0.50f, 0.20f, 1.0f),
                                   "%d / %d unidades (Stock Saludable)",
                                   prod->getStockActual(), prod->getStockMinimo());
            }

            ImGui::EndTable();
        }
    }

    /**
     * Pestaña 2: Gestión de Catálogo Hash — Inserción y Eliminación en O(1).
     */
    void renderPestanaCatalogo(SistemaPanaderia& sistema,
                               Designar::DynArray<ProductoPanaderia*>& productosDinamicos,
                               Designar::DynArray<std::string>& listaSkusCatalogo)
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

        static char skuEliminar[64] = "";
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
                           "Gestion del Catalogo en Tabla Hash (Insercion y Eliminacion O(1))");
        ImGui::Spacing();

        float anchoIzq = (ImGui::GetContentRegionAvail().x - 16.0f) * 0.55f;

        ImGui::BeginChild("PanelAltaProducto", ImVec2(anchoIzq, 450.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Registrar Nuevo Producto");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("SKU (ej. PAN-GOL-001)", nuevoSku, sizeof(nuevoSku));
        ImGui::InputText("Nombre del Producto", nuevoNombre, sizeof(nuevoNombre));
        ImGui::Combo("Tipo de Masa", &idxMasa, tiposMasa, 6);
        ImGui::SliderFloat("% Hidratacion", &hidratacion, 30.0f, 90.0f, "%.1f %%");
        ImGui::InputInt("Tiempo Ferm. (min)", &tiempoFerm);
        ImGui::InputFloat("Temp. Ferm. (C)", &tempFerm, 0.5f, 2.0f, "%.1f");
        ImGui::InputInt("Tiempo Horneado (min)", &tiempoHorn);
        ImGui::InputInt("Temp. Horneado (C)", &tempHorn);
        ImGui::SliderFloat("Rendimiento Masa", &rendimiento, 0.50f, 1.00f, "%.2f");
        ImGui::InputInt("Stock Actual", &stockAct);
        ImGui::InputInt("Stock Minimo", &stockMin);
        ImGui::InputFloat("Costo Unitario ($)", &costo, 0.25f, 1.0f, "%.2f");
        ImGui::InputInt("Dia Venc.", &diaVenc);
        ImGui::InputInt("Mes Venc.", &mesVenc);
        ImGui::InputInt("Anio Venc.", &anioVenc);

        ImGui::Spacing();
        if (ImGui::Button("Registrar en Tabla Hash O(1)"))
        {
            std::string skuStr(nuevoSku);
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

                if (sistema.registrarProducto(nuevo))
                {
                    productosDinamicos.append(nuevo);
                    listaSkusCatalogo.append(skuStr);
                    mensajeEstado = "[+] Producto '" + nomStr + "' (" + skuStr +
                                    ") registrado en la Tabla Hash en O(1).";
                    estadoExito = true;
                    nuevoSku[0] = '\0';
                    nuevoNombre[0] = '\0';
                }
                else
                {
                    delete nuevo;
                    mensajeEstado = "[!] Error: El SKU '" + skuStr + "' ya existe en el catalogo.";
                    estadoExito = false;
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("PanelBajaProducto", ImVec2(0.0f, 450.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Eliminar Producto del Catalogo");
        ImGui::Separator();
        ImGui::Spacing();

        renderBotonesSkusRapidos(listaSkusCatalogo, skuEliminar, sizeof(skuEliminar), "del_skus");
        ImGui::Spacing();

        ImGui::InputText("SKU a eliminar", skuEliminar, sizeof(skuEliminar));
        if (ImGui::Button("Eliminar por SKU O(1)"))
        {
            std::string skuDel(skuEliminar);
            if (sistema.eliminarProducto(skuDel))
            {
                Designar::DynArray<std::string> nuevaLista;
                for (size_t i = 0; i < listaSkusCatalogo.size(); ++i)
                {
                    if (listaSkusCatalogo[i] != skuDel)
                    {
                        nuevaLista.append(listaSkusCatalogo[i]);
                    }
                }
                listaSkusCatalogo = nuevaLista;
                mensajeEstado = "[+] Producto con SKU '" + skuDel + "' eliminado en O(1).";
                estadoExito = true;
                skuEliminar[0] = '\0';
            }
            else
            {
                mensajeEstado = "[!] No se encontro el SKU '" + skuDel + "' para eliminar.";
                estadoExito = false;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("SKUs activos en memoria (%zu):", listaSkusCatalogo.size());
        for (size_t i = 0; i < listaSkusCatalogo.size(); ++i)
        {
            ProductoPanaderia* p = sistema.buscarProducto(listaSkusCatalogo[i]);
            if (p != nullptr)
            {
                ImGui::BulletText("%s — %s (Stock: %d)",
                                  p->getSKU().c_str(),
                                  p->getNombre().c_str(),
                                  p->getStockActual());
            }
        }
        ImGui::EndChild();

        if (!mensajeEstado.empty())
        {
            ImGui::Spacing();
            ImVec4 color = estadoExito
                ? ImVec4(0.10f, 0.50f, 0.20f, 1.0f)
                : ImVec4(0.78f, 0.12f, 0.12f, 1.0f);
            ImGui::TextColored(color, "%s", mensajeEstado.c_str());
        }
    }

    /**
     * Dibuja recursivamente el Árbol N-ario de Receta (MTreeNode) en ImGui.
     */
    void renderNodoArbolReceta(NodoReceta* nodo, bool esRaiz)
    {
        if (nodo == nullptr)
        {
            return;
        }

        const ElementoReceta& elem = nodo->get_key();
        char etiqueta[192];

        if (esRaiz)
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[RAIZ] %s | Base: %.2f %s | Rendimiento: %.0f%%",
                          elem.getCodigoSKU().c_str(),
                          elem.getCantidad(),
                          elem.getUnidadMedida().c_str(),
                          elem.getFactorRendimiento() * 100.0);
        }
        else if (nodo->is_leaf())
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[HOJA] %s | Proporcion: %.3f %s",
                          elem.getCodigoSKU().c_str(),
                          elem.getCantidad(),
                          elem.getUnidadMedida().c_str());
        }
        else
        {
            std::snprintf(etiqueta, sizeof(etiqueta),
                          "[SUBPRODUCTO] %s | Proporcion: %.2f %s",
                          elem.getCodigoSKU().c_str(),
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
                renderNodoArbolReceta(it.get_current(), false);
            }
            ImGui::TreePop();
        }
    }

    /**
     * Pestaña 3: Cocina y Explosión de Receta (Tabla Hash O(1) -> Árbol DFS O(n)).
     */
    void renderPestanaCocina(SistemaPanaderia& sistema,
                             const Designar::DynArray<std::string>& listaSkusCatalogo)
    {
        static char skuReceta[64] = "PAN-JAM-001";
        static float cantidadKg = 10.0f;
        static Designar::DynArray<ElementoReceta> insumosResultantes;
        static std::string ultimoSkuCalculado;
        static double ultimaCantidadCalculada = 0.0;
        static bool pedidoProcesado = false;
        static bool ultimoExito = false;

        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Orden de Produccion para Cocina (Tabla Hash O(1) -> Explosion DFS O(n))");
        ImGui::TextWrapped("Seleccione una receta por SKU e indique el peso del lote en kg. "
                           "El sistema localiza la receta en O(1) y desglosa sus subproductos "
                           "mediante recorrido DFS ajustando por merma.");
        ImGui::Spacing();

        renderBotonesSkusRapidos(listaSkusCatalogo, skuReceta, sizeof(skuReceta), "cocina_skus");
        ImGui::Spacing();

        ImGui::SetNextItemWidth(220.0f);
        ImGui::InputText("SKU a producir", skuReceta, sizeof(skuReceta));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(160.0f);
        ImGui::InputFloat("Lote (kg)", &cantidadKg, 1.0f, 5.0f, "%.2f");
        if (cantidadKg < 0.1f)
        {
            cantidadKg = 0.1f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Calcular Explosion DFS") || !pedidoProcesado)
        {
            insumosResultantes = Designar::DynArray<ElementoReceta>();
            ultimoSkuCalculado = std::string(skuReceta);
            ultimaCantidadCalculada = static_cast<double>(cantidadKg);
            ultimoExito = sistema.procesarPedidoProduccion(
                ultimoSkuCalculado, ultimaCantidadCalculada, insumosResultantes
            );
            pedidoProcesado = true;
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (!ultimoExito)
        {
            ImGui::TextColored(ImVec4(0.78f, 0.12f, 0.12f, 1.0f),
                               "[!] No existe una receta registrada para el SKU '%s'.",
                               ultimoSkuCalculado.c_str());
            return;
        }

        Receta* recetaActiva = sistema.buscarReceta(ultimoSkuCalculado);
        ProductoPanaderia* prodActivo = sistema.buscarProducto(ultimoSkuCalculado);

        float anchoIzq = (ImGui::GetContentRegionAvail().x - 16.0f) * 0.46f;

        // Panel izquierdo: Árbol N-ario interactivo
        ImGui::BeginChild("PanelArbolReceta", ImVec2(anchoIzq, 0.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Estructura del Arbol de Receta (MTreeNode)");
        ImGui::Separator();
        ImGui::Spacing();
        if (recetaActiva != nullptr && recetaActiva->getRaiz() != nullptr)
        {
            renderNodoArbolReceta(recetaActiva->getRaiz(), true);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Panel derecho: Reporte consolidado de materias primas
        ImGui::BeginChild("PanelOrdenCocina", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        ImGui::Text("Orden Consolidada para Cocina");
        ImGui::Separator();
        ImGui::Spacing();

        if (prodActivo != nullptr)
        {
            ImGui::Text("Producto: %s (%s)",
                        prodActivo->getNombre().c_str(),
                        prodActivo->getSKU().c_str());
            ImGui::Text("Rendimiento: %.0f %% | Lote solicitado: %.2f kg",
                        prodActivo->getRendimientoMasa() * 100.0,
                        ultimaCantidadCalculada);
        }
        else
        {
            ImGui::Text("SKU: %s | Lote solicitado: %.2f kg",
                        ultimoSkuCalculado.c_str(),
                        ultimaCantidadCalculada);
        }
        ImGui::Spacing();

        if (ImGui::BeginTable("TablaInsumosCocina", 4,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("SKU Materia Prima", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Cantidad Total", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Unidad", ImGuiTableColumnFlags_WidthFixed, 70.0f);
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
                ImGui::Text("%.4f", item.getCantidad());

                ImGui::TableSetColumnIndex(3);
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
     * Carga los insumos perecederos de demostración en el Min-Heap FEFO.
     */
    void poblarAlmacenPerecederos(GestionPerecederos& almacen)
    {
        almacen.registrarPerecedero(ProductoPanaderia(
            "MP-LEV-001", "Levadura Fresca en Pasta", TipoMasaBase::BLANDA_ENRIQUECIDA,
            0, 0, 0, 0, 0, 1.0, 20, 5, {12, 11, 2026}, 1.20));
        almacen.registrarPerecedero(ProductoPanaderia(
            "MP-JAM-001", "Jamon Ahumado Especial", TipoMasaBase::BLANDA_ENRIQUECIDA,
            0, 0, 0, 0, 0, 1.0, 50, 10, {20, 11, 2026}, 8.50));
        almacen.registrarPerecedero(ProductoPanaderia(
            "MP-LEC-001", "Leche Pasteurizada Completa", TipoMasaBase::BLANDA_ENRIQUECIDA,
            0, 0, 0, 0, 0, 1.0, 30, 5, {8, 11, 2026}, 1.80));
        almacen.registrarPerecedero(ProductoPanaderia(
            "MP-MAR-001", "Margarina con Sal", TipoMasaBase::BLANDA_ENRIQUECIDA,
            0, 0, 0, 0, 0, 1.0, 40, 10, {25, 12, 2026}, 3.00));
    }

    /**
     * Pestaña 4: Trazabilidad de Lotes (SLList) y Perecederos FEFO (Min-Heap).
     */
    void renderPestanaTrazabilidadYPerecederos(LoteProduccion& lote,
                                               GestionPerecederos& almacen)
    {
        static char etapaInput[64] = "";
        static char descInput[160] = "";
        static char respInput[64] = "";
        static Designar::DynArray<ProductoPanaderia> historialDespachosFefo;

        float anchoIzq = (ImGui::GetContentRegionAvail().x - 16.0f) * 0.54f;

        // Columna izquierda: Trazabilidad del Lote (SLList)
        ImGui::BeginChild("PanelTrazabilidadSLList", ImVec2(anchoIzq, 0.0f), ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Trazabilidad de Lote (Designar::SLList)");
        ImGui::Text("Lote: %s | SKU: %s | Cantidad: %.1f kg | Eventos: %zu",
                    lote.getCodigoLote().c_str(),
                    lote.getCodigoSKU().c_str(),
                    lote.getCantidadPlanificada(),
                    lote.getNumeroEventos());
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("TablaTrazabilidad", 4,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp,
                              ImVec2(0.0f, 250.0f)))
        {
            ImGui::TableSetupColumn("Fecha", ImGuiTableColumnFlags_WidthFixed, 85.0f);
            ImGui::TableSetupColumn("Etapa", ImGuiTableColumnFlags_WidthFixed, 125.0f);
            ImGui::TableSetupColumn("Descripcion", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Responsable", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableHeadersRow();

            for (const auto& ev : lote.getEventos())
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
        ImGui::InputText("Etapa (ej. Enfriamiento)", etapaInput, sizeof(etapaInput));
        ImGui::InputText("Descripcion", descInput, sizeof(descInput));
        ImGui::InputText("Responsable", respInput, sizeof(respInput));

        if (ImGui::Button("Agregar Evento a SLList"))
        {
            if (etapaInput[0] != '\0' && descInput[0] != '\0' && respInput[0] != '\0')
            {
                lote.registrarEvento(etapaInput, descInput, respInput, {13, 11, 2026});
                etapaInput[0] = '\0';
                descInput[0] = '\0';
                respInput[0] = '\0';
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Columna derecha: Perecederos FEFO (Min-Heap)
        ImGui::BeginChild("PanelPerecederosMinHeap", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(0.45f, 0.25f, 0.08f, 1.0f),
                           "Gestion FEFO de Perecederos (Min-Heap)");
        ImGui::Text("Insumos esperando en el Min-Heap: %zu", almacen.totalPerecederos());
        ImGui::Separator();
        ImGui::Spacing();

        if (almacen.tienePerecederos())
        {
            ProductoPanaderia prox = almacen.consultarProximoAVencer();
            fecha f = prox.getFechaVencimiento();
            ImGui::TextColored(ImVec4(0.75f, 0.25f, 0.05f, 1.0f),
                               "Cima del Min-Heap (Proximo a vencer):");
            ImGui::BulletText("%s (SKU: %s)", prox.getNombre().c_str(), prox.getSKU().c_str());
            ImGui::BulletText("Fecha de vencimiento: %02d/%02d/%04d", f.dia, f.mes, f.anio);
            ImGui::Spacing();

            if (ImGui::Button("Despachar Proximo a Vencer (FEFO)"))
            {
                ProductoPanaderia desp = almacen.despacharProximoAVencer();
                historialDespachosFefo.append(desp);
            }
        }
        else
        {
            ImGui::TextDisabled("El almacen de perecederos esta vacio.");
            ImGui::Spacing();
            if (ImGui::Button("Recargar Insumos de Prueba"))
            {
                poblarAlmacenPerecederos(almacen);
                historialDespachosFefo = Designar::DynArray<ProductoPanaderia>();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Orden de Despacho FEFO Ejecutado (%zu):", historialDespachosFefo.size());

        if (ImGui::BeginTable("TablaDespachoFEFO", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Orden", ImGuiTableColumnFlags_WidthFixed, 55.0f);
            ImGui::TableSetupColumn("Vencimiento", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Insumo", ImGuiTableColumnFlags_WidthStretch);
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
     * Dibuja la ventana principal anclada y las 4 pestañas de la aplicación.
     */
    void renderizarFramePanaderia(
        ImGuiIO& io,
        SistemaPanaderia& sistema,
        Designar::DynArray<ProductoPanaderia*>& productosDinamicos,
        Designar::DynArray<std::string>& listaSkusCatalogo,
        LoteProduccion& lote1,
        GestionPerecederos& almacen,
        char* skuBusquedaPos,
        size_t skuBusquedaPosTam,
        bool& busquedaPosEjecutada)
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGuiWindowFlags flagsPanel =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("BakerySystemMain", nullptr, flagsPanel);

        ImGui::TextColored(ImVec4(0.55f, 0.28f, 0.06f, 1.0f),
                           "BAKERY SYSTEM — PUNTO DE VENTA (POS) Y PRODUCCION PANADERA");
        ImGui::SameLine();
        ImGui::TextDisabled(" | Programacion III - ULA (Luis Acevedo, Khristopher Santiago, Juan Rincon)");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("MenuPrincipalTabs"))
        {
            if (ImGui::BeginTabItem("1. Punto de Venta (POS - Hash O(1))"))
            {
                ImGui::Spacing();
                renderPestanaPOS(sistema, listaSkusCatalogo,
                                 skuBusquedaPos, skuBusquedaPosTam,
                                 busquedaPosEjecutada);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("2. Catalogo Hash (Alta / Baja O(1))"))
            {
                ImGui::Spacing();
                renderPestanaCatalogo(sistema, productosDinamicos, listaSkusCatalogo);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("3. Cocina y Explosion de Receta (DFS)"))
            {
                ImGui::Spacing();
                renderPestanaCocina(sistema, listaSkusCatalogo);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("4. Trazabilidad (SLList) y Perecederos (Min-Heap)"))
            {
                ImGui::Spacing();
                renderPestanaTrazabilidadYPerecederos(lote1, almacen);
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
static int            g_Width = 1180;
static int            g_Height = 760;

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
    // 1. INICIALIZACIÓN DE DATOS DEL SISTEMA (CATÁLOGO, RECETAS, LOTE, ALMACÉN)
    // =========================================================================
    SistemaPanaderia sistema;
    Designar::DynArray<ProductoPanaderia*> productosDinamicos;
    Designar::DynArray<std::string> listaSkusCatalogo;

    // Producto 1: Pan de Jamón Tradicional
    ProductoPanaderia panDeJamon(
        "PAN-JAM-001", "Pan de Jamon Tradicional", TipoMasaBase::DULCE_SOBADA,
        55.0, 180, 30.0, 35, 180, 0.90, 50, 10, {15, 11, 2026}, 12.50
    );
    Receta recetaPanJamon(panDeJamon);
    NodoReceta* raizJam = recetaPanJamon.buscarElemento("PAN-JAM-001");
    if (raizJam != nullptr)
    {
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("SUB-REL-001", 0.45, "kg"));
        NodoReceta* nodoRelleno = recetaPanJamon.buscarElemento("SUB-REL-001");
        if (nodoRelleno != nullptr)
        {
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-JAM-001", 0.70, "kg"));
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-TOC-001", 0.15, "kg"));
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-PAS-001", 0.10, "kg"));
            recetaPanJamon.agregarElemento(nodoRelleno, ElementoReceta("MP-ACE-001", 0.05, "kg"));
        }
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-HAR-001", 0.55, "kg"));
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-MAR-001", 0.06, "kg"));
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-AZU-001", 0.05, "kg"));
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-AGU-001", 0.20, "kg"));
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-LEV-001", 0.02, "kg"));
        recetaPanJamon.agregarElemento(raizJam, ElementoReceta("MP-PAP-001", 0.05, "kg"));
    }

    // Producto 2: Pan Canilla Venezolano
    ProductoPanaderia panCanilla(
        "PAN-CAN-001", "Pan Canilla Venezolano", TipoMasaBase::CRUJIENTE_RUSTICA,
        62.0, 90, 28.0, 22, 210, 0.88, 120, 30, {14, 11, 2026}, 1.50
    );
    Receta recetaCanilla(panCanilla);
    NodoReceta* raizCan = recetaCanilla.buscarElemento("PAN-CAN-001");
    if (raizCan != nullptr)
    {
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("SUB-PRE-001", 0.30, "kg"));
        NodoReceta* nodoPref = recetaCanilla.buscarElemento("SUB-PRE-001");
        if (nodoPref != nullptr)
        {
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-HAR-001", 0.60, "kg"));
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-AGU-001", 0.38, "kg"));
            recetaCanilla.agregarElemento(nodoPref, ElementoReceta("MP-LEV-001", 0.02, "kg"));
        }
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-HAR-001", 0.45, "kg"));
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-AGU-001", 0.20, "kg"));
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-SAL-001", 0.02, "kg"));
        recetaCanilla.agregarElemento(raizCan, ElementoReceta("MP-MAN-001", 0.03, "kg"));
    }

    // Producto 3: Pan Campesino Masa Madre
    ProductoPanaderia panCampesino(
        "PAN-CAM-001", "Pan Campesino Masa Madre", TipoMasaBase::MADRE_ACIDA,
        70.0, 240, 26.0, 40, 220, 0.85, 35, 10, {16, 11, 2026}, 3.50
    );
    Receta recetaCampesino(panCampesino);
    NodoReceta* raizCam = recetaCampesino.buscarElemento("PAN-CAM-001");
    if (raizCam != nullptr)
    {
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("SUB-MMA-001", 0.25, "kg"));
        NodoReceta* nodoMma = recetaCampesino.buscarElemento("SUB-MMA-001");
        if (nodoMma != nullptr)
        {
            recetaCampesino.agregarElemento(nodoMma, ElementoReceta("MP-HAR-001", 0.50, "kg"));
            recetaCampesino.agregarElemento(nodoMma, ElementoReceta("MP-AGU-001", 0.50, "kg"));
        }
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-HAR-001", 0.50, "kg"));
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-AGU-001", 0.23, "kg"));
        recetaCampesino.agregarElemento(raizCam, ElementoReceta("MP-SAL-001", 0.02, "kg"));
    }

    sistema.registrarProducto(&panDeJamon);
    sistema.registrarReceta(&recetaPanJamon);
    listaSkusCatalogo.append(panDeJamon.getSKU());

    sistema.registrarProducto(&panCanilla);
    sistema.registrarReceta(&recetaCanilla);
    listaSkusCatalogo.append(panCanilla.getSKU());

    sistema.registrarProducto(&panCampesino);
    sistema.registrarReceta(&recetaCampesino);
    listaSkusCatalogo.append(panCampesino.getSKU());

    // Lote de producción (SLList)
    LoteProduccion lote1("LOT-20261106-001", panDeJamon.getSKU(), 10.0, {6, 11, 2026});
    lote1.registrarEvento("Pesaje", "Pesaje exacto de harina, liquidos y relleno", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Amasado", "Amasado durante 12 min hasta desarrollar gluten", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Armado y Sobado", "Laminado e incorporacion de jamon, tocineta y pasas", "Khristopher Santiago", {6, 11, 2026});
    lote1.registrarEvento("Fermentacion Final", "Camara a 30C durante 180 min", "Luis Acevedo", {6, 11, 2026});
    lote1.registrarEvento("Horneado", "Horneado a 180C durante 35 min", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Control de Calidad", "Inspeccion de corteza, peso y empaque", "Khristopher Santiago", {6, 11, 2026});

    // Almacén de perecederos FEFO (Min-Heap)
    GestionPerecederos almacen;
    poblarAlmacenPerecederos(almacen);

    char skuBusquedaPos[64] = "PAN-JAM-001";
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
            io, sistema, productosDinamicos, listaSkusCatalogo,
            lote1, almacen, skuBusquedaPos, sizeof(skuBusquedaPos),
            busquedaPosEjecutada
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
        1180,
        760,
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
            io, sistema, productosDinamicos, listaSkusCatalogo,
            lote1, almacen, skuBusquedaPos, sizeof(skuBusquedaPos),
            busquedaPosEjecutada
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

    // =========================================================================
    // 4. LIBERACIÓN DE RECURSOS (RAII)
    // =========================================================================
    for (size_t i = 0; i < productosDinamicos.size(); ++i)
    {
        delete productosDinamicos[i];
    }

    return 0;
}

