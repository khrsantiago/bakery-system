#include "SistemaPanaderia.hpp"
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace
{
    // =========================================================================
    // AUXILIARES DE SERIALIZACIÓN Y PARSEO JSON (Sin dependencias externas)
    // =========================================================================

    std::string escaparJSON(const std::string& texto)
    {
        std::ostringstream oss;
        for (unsigned char c : texto)
        {
            switch (c)
            {
                case '\"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (c < 0x20)
                    {
                        oss << "\\u00" << std::hex << std::setw(2) << std::setfill('0')
                            << static_cast<int>(c) << std::dec;
                    }
                    else
                    {
                        oss << static_cast<char>(c);
                    }
                    break;
            }
        }
        return oss.str();
    }

    enum class JsonTipo
    {
        Nulo,
        Booleano,
        Numero,
        Cadena,
        Arreglo,
        Objeto
    };

    struct JsonValor
    {
        JsonTipo tipo = JsonTipo::Nulo;
        bool valorBool = false;
        double valorNum = 0.0;
        std::string valorStr;
        std::vector<JsonValor> arreglo;
        std::vector<std::pair<std::string, JsonValor>> objeto;

        const JsonValor* buscar(const std::string& clave) const
        {
            if (tipo != JsonTipo::Objeto)
            {
                return nullptr;
            }
            for (const auto& par : objeto)
            {
                if (par.first == clave)
                {
                    return &par.second;
                }
            }
            return nullptr;
        }

        std::string comoCadena(const std::string& porDefecto = "") const
        {
            return (tipo == JsonTipo::Cadena) ? valorStr : porDefecto;
        }

        double comoNumero(double porDefecto = 0.0) const
        {
            return (tipo == JsonTipo::Numero) ? valorNum : porDefecto;
        }

        int comoEntero(int porDefecto = 0) const
        {
            return (tipo == JsonTipo::Numero) ? static_cast<int>(std::llround(valorNum)) : porDefecto;
        }
    };

    class ParserJSON
    {
    private:
        const std::string& src;
        size_t pos;

        void saltarEspacios()
        {
            while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos])))
            {
                ++pos;
            }
        }

        bool consumirChar(char esperado)
        {
            saltarEspacios();
            if (pos < src.size() && src[pos] == esperado)
            {
                ++pos;
                return true;
            }
            return false;
        }

        bool parsearCadena(std::string& salida)
        {
            saltarEspacios();
            if (pos >= src.size() || src[pos] != '\"')
            {
                return false;
            }
            ++pos;
            salida.clear();

            while (pos < src.size())
            {
                char c = src[pos++];
                if (c == '\"')
                {
                    return true;
                }
                if (c == '\\')
                {
                    if (pos >= src.size())
                    {
                        return false;
                    }
                    char esc = src[pos++];
                    switch (esc)
                    {
                        case '\"': salida.push_back('\"'); break;
                        case '\\': salida.push_back('\\'); break;
                        case '/':  salida.push_back('/'); break;
                        case 'b':  salida.push_back('\b'); break;
                        case 'f':  salida.push_back('\f'); break;
                        case 'n':  salida.push_back('\n'); break;
                        case 'r':  salida.push_back('\r'); break;
                        case 't':  salida.push_back('\t'); break;
                        case 'u':
                            if (pos + 4 <= src.size())
                            {
                                pos += 4;
                            }
                            else
                            {
                                return false;
                            }
                            break;
                        default:
                            salida.push_back(esc);
                            break;
                    }
                }
                else
                {
                    salida.push_back(c);
                }
            }
            return false;
        }

        bool parsearNumero(double& salida)
        {
            saltarEspacios();
            size_t inicio = pos;
            if (pos < src.size() && src[pos] == '-')
            {
                ++pos;
            }
            bool tieneDigito = false;
            while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos])))
            {
                tieneDigito = true;
                ++pos;
            }
            if (pos < src.size() && src[pos] == '.')
            {
                ++pos;
                while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos])))
                {
                    tieneDigito = true;
                    ++pos;
                }
            }
            if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E'))
            {
                ++pos;
                if (pos < src.size() && (src[pos] == '+' || src[pos] == '-'))
                {
                    ++pos;
                }
                bool expDig = false;
                while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos])))
                {
                    expDig = true;
                    ++pos;
                }
                if (!expDig)
                {
                    return false;
                }
            }
            if (!tieneDigito)
            {
                return false;
            }
            try
            {
                salida = std::stod(src.substr(inicio, pos - inicio));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool parsearArreglo(JsonValor& salida)
        {
            if (!consumirChar('['))
            {
                return false;
            }
            salida = JsonValor();
            salida.tipo = JsonTipo::Arreglo;

            saltarEspacios();
            if (pos < src.size() && src[pos] == ']')
            {
                ++pos;
                return true;
            }

            while (pos < src.size())
            {
                JsonValor elem;
                if (!parsearValor(elem))
                {
                    return false;
                }
                salida.arreglo.push_back(std::move(elem));

                saltarEspacios();
                if (pos < src.size() && src[pos] == ',')
                {
                    ++pos;
                    continue;
                }
                if (pos < src.size() && src[pos] == ']')
                {
                    ++pos;
                    return true;
                }
                return false;
            }
            return false;
        }

        bool parsearObjeto(JsonValor& salida)
        {
            if (!consumirChar('{'))
            {
                return false;
            }
            salida = JsonValor();
            salida.tipo = JsonTipo::Objeto;

            saltarEspacios();
            if (pos < src.size() && src[pos] == '}')
            {
                ++pos;
                return true;
            }

            while (pos < src.size())
            {
                std::string clave;
                if (!parsearCadena(clave))
                {
                    return false;
                }
                if (!consumirChar(':'))
                {
                    return false;
                }
                JsonValor val;
                if (!parsearValor(val))
                {
                    return false;
                }
                salida.objeto.emplace_back(std::move(clave), std::move(val));

                saltarEspacios();
                if (pos < src.size() && src[pos] == ',')
                {
                    ++pos;
                    continue;
                }
                if (pos < src.size() && src[pos] == '}')
                {
                    ++pos;
                    return true;
                }
                return false;
            }
            return false;
        }

    public:
        explicit ParserJSON(const std::string& texto)
            : src(texto), pos(0)
        {
        }

        bool parsearValor(JsonValor& salida)
        {
            saltarEspacios();
            if (pos >= src.size())
            {
                return false;
            }
            char c = src[pos];
            if (c == '{')
            {
                return parsearObjeto(salida);
            }
            if (c == '[')
            {
                return parsearArreglo(salida);
            }
            if (c == '\"')
            {
                salida = JsonValor();
                salida.tipo = JsonTipo::Cadena;
                return parsearCadena(salida.valorStr);
            }
            if (c == 't' && src.compare(pos, 4, "true") == 0)
            {
                pos += 4;
                salida = JsonValor();
                salida.tipo = JsonTipo::Booleano;
                salida.valorBool = true;
                return true;
            }
            if (c == 'f' && src.compare(pos, 5, "false") == 0)
            {
                pos += 5;
                salida = JsonValor();
                salida.tipo = JsonTipo::Booleano;
                salida.valorBool = false;
                return true;
            }
            if (c == 'n' && src.compare(pos, 4, "null") == 0)
            {
                pos += 4;
                salida = JsonValor();
                salida.tipo = JsonTipo::Nulo;
                return true;
            }
            if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
            {
                salida = JsonValor();
                salida.tipo = JsonTipo::Numero;
                return parsearNumero(salida.valorNum);
            }
            return false;
        }
    };

    void escribirFechaJSON(std::ostream& out, const fecha& f)
    {
        out << "{\"dia\": " << f.dia << ", \"mes\": " << f.mes << ", \"anio\": " << f.anio << "}";
    }

    fecha leerFechaJSON(const JsonValor* obj)
    {
        if (obj == nullptr || obj->tipo != JsonTipo::Objeto)
        {
            return {1, 1, 2026};
        }
        const JsonValor* d = obj->buscar("dia");
        const JsonValor* m = obj->buscar("mes");
        const JsonValor* a = obj->buscar("anio");
        return {
            d ? d->comoEntero(1) : 1,
            m ? m->comoEntero(1) : 1,
            a ? a->comoEntero(2026) : 2026
        };
    }

    void escribirProductoJSON(std::ostream& out, const ProductoPanaderia& p, const std::string& indent)
    {
        out << indent << "{\n"
            << indent << "  \"sku\": \"" << escaparJSON(p.getSKU()) << "\",\n"
            << indent << "  \"nombre\": \"" << escaparJSON(p.getNombre()) << "\",\n"
            << indent << "  \"tipoMasa\": " << static_cast<int>(p.getTipoMasa()) << ",\n"
            << indent << "  \"hidratacion\": " << p.getPorcentajeHidratacion() << ",\n"
            << indent << "  \"tiempoFermentacion\": " << p.getTiempoFermentacion() << ",\n"
            << indent << "  \"temperaturaFermentacion\": " << p.getTemperaturaFermentacion() << ",\n"
            << indent << "  \"tiempoHorneado\": " << p.getTiempoHorneado() << ",\n"
            << indent << "  \"temperaturaHorneado\": " << p.getTemperaturaHorneado() << ",\n"
            << indent << "  \"rendimientoMasa\": " << p.getRendimientoMasa() << ",\n"
            << indent << "  \"stockActual\": " << p.getStockActual() << ",\n"
            << indent << "  \"stockMinimo\": " << p.getStockMinimo() << ",\n"
            << indent << "  \"fechaVencimiento\": ";
        escribirFechaJSON(out, p.getFechaVencimiento());
        out << ",\n"
            << indent << "  \"costoUnitario\": " << p.getCostoUnitario() << ",\n"
            << indent << "  \"alergenos\": [";
        const Designar::DynArray<std::string>& alerg = p.getAlergenos();
        for (size_t i = 0; i < alerg.size(); ++i)
        {
            if (i > 0)
            {
                out << ", ";
            }
            out << "\"" << escaparJSON(alerg[i]) << "\"";
        }
        out << "]\n" << indent << "}";
    }

    ProductoPanaderia leerProductoJSON(const JsonValor& obj)
    {
        auto getStr = [&](const char* k, const std::string& def) {
            const JsonValor* v = obj.buscar(k);
            return v ? v->comoCadena(def) : def;
        };
        auto getNum = [&](const char* k, double def) {
            const JsonValor* v = obj.buscar(k);
            return v ? v->comoNumero(def) : def;
        };
        auto getInt = [&](const char* k, int def) {
            const JsonValor* v = obj.buscar(k);
            return v ? v->comoEntero(def) : def;
        };

        int tipoMasaInt = getInt("tipoMasa", static_cast<int>(TipoMasaBase::BLANDA_ENRIQUECIDA));
        if (tipoMasaInt < 0 || tipoMasaInt > static_cast<int>(TipoMasaBase::INTEGRAL))
        {
            tipoMasaInt = static_cast<int>(TipoMasaBase::BLANDA_ENRIQUECIDA);
        }

        ProductoPanaderia prod(
            getStr("sku", ""),
            getStr("nombre", ""),
            static_cast<TipoMasaBase>(tipoMasaInt),
            getNum("hidratacion", 55.0),
            getInt("tiempoFermentacion", 120),
            getNum("temperaturaFermentacion", 28.0),
            getInt("tiempoHorneado", 25),
            getInt("temperaturaHorneado", 180),
            getNum("rendimientoMasa", 0.90),
            getInt("stockActual", 0),
            getInt("stockMinimo", 0),
            leerFechaJSON(obj.buscar("fechaVencimiento")),
            getNum("costoUnitario", 0.0)
        );

        const JsonValor* alerg = obj.buscar("alergenos");
        if (alerg != nullptr && alerg->tipo == JsonTipo::Arreglo)
        {
            for (const auto& a : alerg->arreglo)
            {
                std::string nombreAlergeno = a.comoCadena();
                if (!nombreAlergeno.empty())
                {
                    prod.agregarAlergeno(nombreAlergeno);
                }
            }
        }

        return prod;
    }

    struct NodoRecetaPlano
    {
        int padreIndice;
        ElementoReceta elemento;
    };

    void aplanarNodosPreorden(NodoReceta* nodo, int padreIdx, std::vector<NodoRecetaPlano>& salida)
    {
        if (nodo == nullptr)
        {
            return;
        }
        int indiceActual = static_cast<int>(salida.size());
        salida.push_back({padreIdx, nodo->get_key()});

        for (NodoReceta::ChildrenIterator it(*nodo); it.has_current(); it.next())
        {
            aplanarNodosPreorden(it.get_current(), indiceActual, salida);
        }
    }

    void imprimirArbolRecursivo(const SistemaPanaderia& sistema, NodoReceta* nodo, int nivel)
    {
        if (nodo == nullptr)
        {
            return;
        }

        for (int i = 0; i < nivel; ++i)
        {
            std::cout << "  ";
        }
        if (nivel > 0)
        {
            std::cout << "|- ";
        }

        const ElementoReceta& elem = nodo->get_key();
        std::cout << elem.getCodigoSKU()
                  << " (" << sistema.obtenerNombreSKU(elem.getCodigoSKU()) << ")"
                  << " : " << elem.getCantidad() << " " << elem.getUnidadMedida();

        if (!nodo->is_leaf() && elem.getFactorRendimiento() > 0.0 && elem.getFactorRendimiento() < 1.0)
        {
            std::cout << " [Rend: " << (elem.getFactorRendimiento() * 100.0) << "%]";
        }
        std::cout << "\n";

        for (NodoReceta::ChildrenIterator it(*nodo); it.has_current(); it.next())
        {
            imprimirArbolRecursivo(sistema, it.get_current(), nivel + 1);
        }
    }

    void registrarNombresBaseSKU(SistemaPanaderia& sistema)
    {
        // Materias primas base
        sistema.registrarNombreSKU("MP-HAR-001", "Harina Panadera");
        sistema.registrarNombreSKU("MP-JAM-001", "Jamon Ahumado Especial");
        sistema.registrarNombreSKU("MP-TOC-001", "Tocineta Ahumada");
        sistema.registrarNombreSKU("MP-PAS-001", "Pasitas");
        sistema.registrarNombreSKU("MP-ACE-001", "Aceitunas Rellenas");
        sistema.registrarNombreSKU("MP-MAR-001", "Margarina con Sal");
        sistema.registrarNombreSKU("MP-AZU-001", "Azucar Blanquilla");
        sistema.registrarNombreSKU("MP-AGU-001", "Agua / Liquidos");
        sistema.registrarNombreSKU("MP-LEV-001", "Levadura Fresca en Pasta");
        sistema.registrarNombreSKU("MP-PAP-001", "Papelon (Melao)");
        sistema.registrarNombreSKU("MP-SAL-001", "Sal Refinada");
        sistema.registrarNombreSKU("MP-MAN-001", "Manteca Vegetal");
        sistema.registrarNombreSKU("MP-LEC-001", "Leche Pasteurizada Completa");

        // Subproductos intermedios base
        sistema.registrarNombreSKU("SUB-REL-001", "Relleno Pan de Jamon");
        sistema.registrarNombreSKU("SUB-PRE-001", "Prefermento / Esponja");
        sistema.registrarNombreSKU("SUB-MMA-001", "Masa Madre Activa");
    }
} // namespace

Designar::nat_t hashSKU(const std::string& sku)
{
    Designar::nat_t hash = 14695981039346656037ULL; // Offset basis FNV-1a 64-bit
    for (unsigned char c : sku)
    {
        hash ^= static_cast<Designar::nat_t>(c);
        hash *= 1099511628211ULL; // FNV prime 64-bit
    }
    return hash;
}

SistemaPanaderia::SistemaPanaderia()
    : catalogoRecetas(std::equal_to<std::string>(), &hashSKU),
      catalogoProductos(std::equal_to<std::string>(), &hashSKU),
      catalogoNombresSKU(std::equal_to<std::string>(), &hashSKU),
      listaProductos(),
      productosPropios(),
      recetasPropias(),
      lotesProduccion(),
      kardex(),
      almacenPerecederos()
{
    registrarNombresBaseSKU(*this);
}

SistemaPanaderia::~SistemaPanaderia()
{
    limpiarMemoriaPropia();
}

void SistemaPanaderia::limpiarMemoriaPropia()
{
    for (size_t i = 0; i < recetasPropias.size(); ++i)
    {
        delete recetasPropias[i];
    }
    recetasPropias.clear();

    for (size_t i = 0; i < productosPropios.size(); ++i)
    {
        delete productosPropios[i];
    }
    productosPropios.clear();
}

bool SistemaPanaderia::registrarProducto(ProductoPanaderia* producto, bool tomarPropiedad)
{
    if (producto == nullptr || producto->getSKU().empty())
    {
        return false;
    }

    if (catalogoProductos.has(producto->getSKU()))
    {
        return false;
    }

    if (catalogoProductos.insert(producto->getSKU(), producto) == nullptr)
    {
        return false;
    }

    listaProductos.append(producto);
    registrarNombreSKU(producto->getSKU(), producto->getNombre());

    if (tomarPropiedad)
    {
        productosPropios.append(producto);
    }

    return true;
}

bool SistemaPanaderia::registrarReceta(Receta* receta, bool tomarPropiedad)
{
    if (receta == nullptr || receta->getSKU().empty())
    {
        return false;
    }

    if (catalogoRecetas.has(receta->getSKU()))
    {
        return false;
    }

    if (catalogoRecetas.insert(receta->getSKU(), receta) == nullptr)
    {
        return false;
    }

    if (!receta->getNombre().empty())
    {
        registrarNombreSKU(receta->getSKU(), receta->getNombre());
    }

    if (tomarPropiedad)
    {
        recetasPropias.append(receta);
    }

    return true;
}

void SistemaPanaderia::registrarNombreSKU(const std::string& sku, const std::string& nombre)
{
    if (sku.empty() || nombre.empty())
    {
        return;
    }
    catalogoNombresSKU[sku] = nombre;
}

std::string SistemaPanaderia::obtenerNombreSKU(const std::string& sku) const
{
    if (sku.empty())
    {
        return "";
    }

    ProductoPanaderia* const* ptrProd = catalogoProductos.search(sku);
    if (ptrProd != nullptr && *ptrProd != nullptr && !(*ptrProd)->getNombre().empty())
    {
        return (*ptrProd)->getNombre();
    }

    Receta* const* ptrRec = catalogoRecetas.search(sku);
    if (ptrRec != nullptr && *ptrRec != nullptr && !(*ptrRec)->getNombre().empty())
    {
        return (*ptrRec)->getNombre();
    }

    const std::string* ptrNom = catalogoNombresSKU.search(sku);
    if (ptrNom != nullptr && !ptrNom->empty())
    {
        return *ptrNom;
    }

    return "Insumo / Item";
}

bool SistemaPanaderia::existeProducto(const std::string& sku) const
{
    return !sku.empty() && catalogoProductos.has(sku);
}

bool SistemaPanaderia::existeReceta(const std::string& sku) const
{
    return !sku.empty() && catalogoRecetas.has(sku);
}

const Designar::DynArray<ProductoPanaderia*>& SistemaPanaderia::obtenerListaProductos() const
{
    return listaProductos;
}

const Designar::HashMap<std::string, std::string>& SistemaPanaderia::obtenerCatalogoNombresSKU() const
{
    return catalogoNombresSKU;
}

ProductoPanaderia* SistemaPanaderia::buscarProducto(const std::string& sku)
{
    ProductoPanaderia** ptrProducto = catalogoProductos.search(sku);
    if (ptrProducto == nullptr)
    {
        return nullptr;
    }
    return *ptrProducto;
}

Receta* SistemaPanaderia::buscarReceta(const std::string& sku)
{
    Receta** ptrReceta = catalogoRecetas.search(sku);
    if (ptrReceta == nullptr)
    {
        return nullptr;
    }
    return *ptrReceta;
}

bool SistemaPanaderia::eliminarProducto(const std::string& sku)
{
    if (!catalogoProductos.remove(sku))
    {
        return false;
    }

    Designar::DynArray<ProductoPanaderia*> nuevaLista;
    for (size_t i = 0; i < listaProductos.size(); ++i)
    {
        if (listaProductos[i] != nullptr && listaProductos[i]->getSKU() != sku)
        {
            nuevaLista.append(listaProductos[i]);
        }
    }
    listaProductos = std::move(nuevaLista);
    return true;
}

bool SistemaPanaderia::eliminarReceta(const std::string& sku)
{
    return catalogoRecetas.remove(sku);
}

bool SistemaPanaderia::procesarPedidoProduccion(const std::string& sku,
                                              double cantidadSolicitadaKg,
                                              const fecha& fechaHoy,
                                              Designar::DynArray<ElementoReceta>& insumosConsolidados)
{
    // 1. Búsqueda en O(1) sobre la Tabla Hash
    Receta** ptrReceta = catalogoRecetas.search(sku);
    if (ptrReceta == nullptr || *ptrReceta == nullptr)
    {
        std::cerr << "[!] Error: No se encontró receta para el SKU: " << sku << "\n";
        return false;
    }

    Receta* recetaElegida = *ptrReceta;

    // 2. Desglose recursivo DFS en O(n) sobre el Árbol General
    recetaElegida->calcularMateriasPrimas(cantidadSolicitadaKg, insumosConsolidados);

    // 3. Conversión a Movimientos y Registro en Kardex AVL en O(k log n)
    if (!insumosConsolidados.is_empty())
    {
        Designar::DynArray<MovimientoKardex> egresos = generarMovimientosEgreso(insumosConsolidados, sku, fechaHoy);
        kardex.registrarMovimientos(egresos);
    }

    return true;
}

bool SistemaPanaderia::procesarPedidoProduccion(const std::string& sku,
                                                double cantidadSolicitadaKg,
                                                Designar::DynArray<ElementoReceta>& insumosConsolidados)
{
    return procesarPedidoProduccion(sku, cantidadSolicitadaKg, obtenerFechaActual(), insumosConsolidados);
}

void SistemaPanaderia::generarReporteCocina(const std::string& sku, double cantidadSolicitadaKg)
{
    Designar::DynArray<ElementoReceta> insumosConsolidados;

    std::cout << "\n=======================================================\n";
    std::cout << "           ORDEN DE PRODUCCIÓN PARA COCINA             \n";
    std::cout << "=======================================================\n";

    // 1. Ejecuta la búsqueda O(1) en la Tabla Hash y la explosión DFS O(n)
    fecha fechaHoy = obtenerFechaActual();

    if (!procesarPedidoProduccion(sku, cantidadSolicitadaKg, fechaHoy, insumosConsolidados))
    {
        std::cout << "[!] No se pudo generar el reporte. Verifique el SKU ingresado.\n";
        std::cout << "=======================================================\n\n";
        return;
    }

    // 2. Encabezado del reporte
    ProductoPanaderia* prod = buscarProducto(sku);
    if (prod != nullptr)
    {
        std::cout << " Producto             : " << prod->getNombre()
                  << " (Rendimiento: " << (prod->getRendimientoMasa() * 100.0) << "%)\n";
    }
    else
    {
        std::cout << " Producto             : " << obtenerNombreSKU(sku) << "\n";
    }
    std::cout << " SKU del Producto     : " << sku << "\n";
    std::cout << " Tamano del Lote      : " << cantidadSolicitadaKg << " kg\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << " MATERIAS PRIMAS CONSOLIDADAS:\n";
    std::cout << "-------------------------------------------------------\n";

    // 3. Recorrido del DynArray resultante resolviendo nombres por SKU en O(1)
    if (insumosConsolidados.is_empty())
    {
        std::cout << " (No se registraron materias primas para esta receta)\n";
    }
    else
    {
        for (size_t i = 0; i < insumosConsolidados.size(); ++i)
        {
            const ElementoReceta& ins = insumosConsolidados[i];
            std::cout << "  [" << (i + 1) << "] "
                      << "SKU: " << ins.getCodigoSKU()
                      << " (" << obtenerNombreSKU(ins.getCodigoSKU()) << ")"
                      << " | Cantidad: " << ins.getCantidad() << " " << ins.getUnidadMedida() << "\n";
        }
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << " Total de Insumos Domiciliados : " << insumosConsolidados.size() << "\n";
    std::cout << " Estado                        : LISTO PARA PREPARACIÓN\n";
    std::cout << "=======================================================\n\n";
}

void SistemaPanaderia::mostrarArbolReceta(const std::string& sku) const
{
    Receta* const* ptrReceta = catalogoRecetas.search(sku);
    if (ptrReceta == nullptr || *ptrReceta == nullptr)
    {
        std::cout << "[!] No existe receta registrada para el SKU: " << sku << "\n";
        return;
    }

    const Receta* receta = *ptrReceta;
    std::cout << "\n-------------------------------------------------------\n";
    std::cout << " ESTRUCTURA DE RECETA: " << receta->getNombre()
              << " [SKU: " << receta->getSKU() << "]\n";
    std::cout << "-------------------------------------------------------\n";
    imprimirArbolRecursivo(*this, receta->getRaiz(), 0);
    std::cout << "-------------------------------------------------------\n";
}

bool SistemaPanaderia::registrarVentaPOS(const std::string& sku, int unidadesVendidas, const fecha& fechaVenta)
{
    if (unidadesVendidas <= 0)
    {
        return false;
    }

    ProductoPanaderia* prod = buscarProducto(sku);
    if (prod == nullptr)
    {
        return false;
    }

    if (prod->getStockActual() < unidadesVendidas)
    {
        return false;
    }

    prod->reducirStock(unidadesVendidas);

    MovimientoKardex movVenta{
        fechaVenta,
        sku,
        TipoMovimiento::VENTA_POS,
        static_cast<double>(unidadesVendidas),
        "Venta en POS de: " + prod->getNombre()
    };
    kardex.registrarMovimiento(movVenta);
    return true;
}

bool SistemaPanaderia::registrarVentaPOS(const std::string& sku, int unidadesVendidas)
{
    return registrarVentaPOS(sku, unidadesVendidas, obtenerFechaActual());
}

void SistemaPanaderia::registrarLote(const LoteProduccion& lote)
{
    lotesProduccion.append(lote);
}

Designar::DynArray<LoteProduccion>& SistemaPanaderia::getLotes()
{
    return lotesProduccion;
}

const Designar::DynArray<LoteProduccion>& SistemaPanaderia::getLotes() const
{
    return lotesProduccion;
}

void SistemaPanaderia::mostrarHistorialLote(const LoteProduccion& lote) const
{
    std::cout << "==================================================\n";
    std::cout << " HISTORIAL DE TRAZABILIDAD DEL LOTE: " << lote.getCodigoLote() << "\n";
    std::cout << " Producto: " << obtenerNombreSKU(lote.getCodigoSKU())
              << " (SKU: " << lote.getCodigoSKU() << ")"
              << " | Cantidad: " << lote.getCantidadPlanificada() << " kg\n";
    std::cout << "==================================================\n";

    const Designar::SLList<EventoTrazabilidad>& eventos = lote.getEventos();
    if (eventos.is_empty())
    {
        std::cout << "  (No se han registrado eventos para este lote)\n";
    }
    else
    {
        for (const auto& evento : eventos)
        {
            evento.mostrarInfo();
        }
    }
    std::cout << "==================================================\n\n";
}

void SistemaPanaderia::registrarMovimientoInventario(const MovimientoKardex& mov)
{
    kardex.registrarMovimiento(mov);
}

void SistemaPanaderia::consultarKardexPorRangoFechas(const fecha& fechaInicio, const fecha& fechaFin) const
{
    std::cout << "=======================================================\n";
    std::cout << " KARDEX DE INVENTARIO (RANGO: "
              << fechaInicio.dia << "/" << fechaInicio.mes << "/" << fechaInicio.anio
              << " - "
              << fechaFin.dia << "/" << fechaFin.mes << "/" << fechaFin.anio << ")\n";
    std::cout << "=======================================================\n";

    if (fechaFin < fechaInicio)
    {
        std::cout << " [!] Rango de fechas invalido: la fecha final es anterior a la inicial.\n";
        std::cout << "=======================================================\n\n";
        return;
    }

    Designar::DynArray<MovimientoKardex> movimientos =
        kardex.consultarPorRangoFechas(fechaInicio, fechaFin);

    if (movimientos.is_empty())
    {
        std::cout << "  (No se registraron movimientos en este rango de fechas)\n";
    }
    else
    {
        for (size_t i = 0; i < movimientos.size(); ++i)
        {
            const MovimientoKardex& mov = movimientos[i];
            std::cout << "  [" << (i + 1) << "] "
                      << "[" << mov.fechaTransaccion.dia << "/" << mov.fechaTransaccion.mes
                      << "/" << mov.fechaTransaccion.anio << "] "
                      << "[" << tipoMovimientoToString(mov.tipo) << "] "
                      << "SKU: " << mov.codigoSKU
                      << " (" << obtenerNombreSKU(mov.codigoSKU) << ")"
                      << " | Cantidad: " << mov.cantidad
                      << " | Concepto: " << mov.descripcion << "\n";
        }
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << " Movimientos en rango : " << movimientos.size() << "\n";
    std::cout << " Total en Kardex AVL  : " << kardex.totalMovimientos() << "\n";
    std::cout << "=======================================================\n\n";
}

KardexInventario& SistemaPanaderia::getKardex()
{
    return kardex;
}

const KardexInventario& SistemaPanaderia::getKardex() const
{
    return kardex;
}

void SistemaPanaderia::registrarPerecedero(const ProductoPanaderia& prod)
{
    almacenPerecederos.registrarPerecedero(prod);
}

ProductoPanaderia SistemaPanaderia::consultarProximoAVencer() const
{
    return almacenPerecederos.consultarProximoAVencer();
}

ProductoPanaderia SistemaPanaderia::despacharProximoAVencer()
{
    return almacenPerecederos.despacharProximoAVencer();
}

bool SistemaPanaderia::tienePerecederos() const
{
    return almacenPerecederos.tienePerecederos();
}

size_t SistemaPanaderia::totalPerecederos() const
{
    return almacenPerecederos.totalPerecederos();
}

GestionPerecederos& SistemaPanaderia::getAlmacenPerecederos()
{
    return almacenPerecederos;
}

const GestionPerecederos& SistemaPanaderia::getAlmacenPerecederos() const
{
    return almacenPerecederos;
}

void SistemaPanaderia::cargarDatosPorDefecto()
{
    catalogoRecetas.clear();
    catalogoProductos.clear();
    catalogoNombresSKU.clear();
    listaProductos.clear();
    lotesProduccion.clear();
    kardex = KardexInventario();
    almacenPerecederos = GestionPerecederos();
    limpiarMemoriaPropia();

    registrarNombresBaseSKU(*this);

    // 1. Producto y Receta 1: Pan de Jamón Tradicional
    auto* panDeJamon = new ProductoPanaderia(
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
    auto* recetaPanJamon = new Receta(*panDeJamon);
    NodoReceta* raizJam = recetaPanJamon->buscarElemento("PAN-JAM-001");
    if (raizJam != nullptr)
    {
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("SUB-REL-001", 0.45, "kg"));
        NodoReceta* nodoRelleno = recetaPanJamon->buscarElemento("SUB-REL-001");
        if (nodoRelleno != nullptr)
        {
            recetaPanJamon->agregarElemento(nodoRelleno, ElementoReceta("MP-JAM-001", 0.70, "kg"));
            recetaPanJamon->agregarElemento(nodoRelleno, ElementoReceta("MP-TOC-001", 0.15, "kg"));
            recetaPanJamon->agregarElemento(nodoRelleno, ElementoReceta("MP-PAS-001", 0.10, "kg"));
            recetaPanJamon->agregarElemento(nodoRelleno, ElementoReceta("MP-ACE-001", 0.05, "kg"));
        }
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-HAR-001", 0.55, "kg"));
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-MAR-001", 0.06, "kg"));
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-AZU-001", 0.05, "kg"));
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-AGU-001", 0.20, "kg"));
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-LEV-001", 0.02, "kg"));
        recetaPanJamon->agregarElemento(raizJam, ElementoReceta("MP-PAP-001", 0.05, "kg"));
    }

    // 2. Producto y Receta 2: Pan Canilla Venezolano
    auto* panCanilla = new ProductoPanaderia(
        "PAN-CAN-001",
        "Pan Canilla Venezolano",
        TipoMasaBase::CRUJIENTE_RUSTICA,
        62.0,
        90,
        28.0,
        22,
        210,
        0.88,
        120,
        30,
        {14, 11, 2026},
        1.50
    );
    auto* recetaCanilla = new Receta(*panCanilla);
    NodoReceta* raizCan = recetaCanilla->buscarElemento("PAN-CAN-001");
    if (raizCan != nullptr)
    {
        recetaCanilla->agregarElemento(raizCan, ElementoReceta("SUB-PRE-001", 0.30, "kg"));
        NodoReceta* nodoPrefermento = recetaCanilla->buscarElemento("SUB-PRE-001");
        if (nodoPrefermento != nullptr)
        {
            recetaCanilla->agregarElemento(nodoPrefermento, ElementoReceta("MP-HAR-001", 0.60, "kg"));
            recetaCanilla->agregarElemento(nodoPrefermento, ElementoReceta("MP-AGU-001", 0.38, "kg"));
            recetaCanilla->agregarElemento(nodoPrefermento, ElementoReceta("MP-LEV-001", 0.02, "kg"));
        }
        recetaCanilla->agregarElemento(raizCan, ElementoReceta("MP-HAR-001", 0.45, "kg"));
        recetaCanilla->agregarElemento(raizCan, ElementoReceta("MP-AGU-001", 0.20, "kg"));
        recetaCanilla->agregarElemento(raizCan, ElementoReceta("MP-SAL-001", 0.02, "kg"));
        recetaCanilla->agregarElemento(raizCan, ElementoReceta("MP-MAN-001", 0.03, "kg"));
    }

    // 3. Producto y Receta 3: Pan Campesino Masa Madre
    auto* panCampesino = new ProductoPanaderia(
        "PAN-CAM-001",
        "Pan Campesino Masa Madre",
        TipoMasaBase::MADRE_ACIDA,
        70.0,
        240,
        26.0,
        40,
        220,
        0.85,
        35,
        10,
        {16, 11, 2026},
        3.50
    );
    auto* recetaCampesino = new Receta(*panCampesino);
    NodoReceta* raizCam = recetaCampesino->buscarElemento("PAN-CAM-001");
    if (raizCam != nullptr)
    {
        recetaCampesino->agregarElemento(raizCam, ElementoReceta("SUB-MMA-001", 0.25, "kg"));
        NodoReceta* nodoMasaMadre = recetaCampesino->buscarElemento("SUB-MMA-001");
        if (nodoMasaMadre != nullptr)
        {
            recetaCampesino->agregarElemento(nodoMasaMadre, ElementoReceta("MP-HAR-001", 0.50, "kg"));
            recetaCampesino->agregarElemento(nodoMasaMadre, ElementoReceta("MP-AGU-001", 0.50, "kg"));
        }
        recetaCampesino->agregarElemento(raizCam, ElementoReceta("MP-HAR-001", 0.50, "kg"));
        recetaCampesino->agregarElemento(raizCam, ElementoReceta("MP-AGU-001", 0.23, "kg"));
        recetaCampesino->agregarElemento(raizCam, ElementoReceta("MP-SAL-001", 0.02, "kg"));
    }

    registrarProducto(panDeJamon, true);
    registrarReceta(recetaPanJamon, true);
    registrarProducto(panCanilla, true);
    registrarReceta(recetaCanilla, true);
    registrarProducto(panCampesino, true);
    registrarReceta(recetaCampesino, true);

    // 4. Lote inicial de Producción con Trazabilidad (SLList)
    LoteProduccion lote1("LOT-20261106-001", panDeJamon->getSKU(), 10.0, {6, 11, 2026});
    lote1.registrarEvento("Pesaje", "Pesaje exacto de harina, liquidos y relleno", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Amasado", "Amasado durante 12 min hasta desarrollar gluten", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Armado y Sobado", "Laminado e incorporacion de jamon, tocineta y pasas", "Khristopher Santiago", {6, 11, 2026});
    lote1.registrarEvento("Fermentacion Final", "Camara a 30C durante 180 min", "Luis Acevedo", {6, 11, 2026});
    lote1.registrarEvento("Horneado", "Horneado a 180C durante 35 min", "Juan Rincon", {6, 11, 2026});
    lote1.registrarEvento("Control de Calidad", "Inspeccion de corteza, peso y empaque", "Khristopher Santiago", {6, 11, 2026});
    registrarLote(lote1);

    // 5. Almacén de Perecederos inicial (Min-Heap FEFO)
    registrarPerecedero(ProductoPanaderia("MP-LEV-001", "Levadura Fresca en Pasta", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                          0, 0, 0, 0, 0, 1.0, 20, 5, {12, 11, 2026}, 1.20));
    registrarPerecedero(ProductoPanaderia("MP-JAM-001", "Jamon Ahumado Especial", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                          0, 0, 0, 0, 0, 1.0, 50, 10, {20, 11, 2026}, 8.50));
    registrarPerecedero(ProductoPanaderia("MP-LEC-001", "Leche Pasteurizada Completa", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                          0, 0, 0, 0, 0, 1.0, 30, 5, {8, 11, 2026}, 1.80));
    registrarPerecedero(ProductoPanaderia("MP-MAR-001", "Margarina con Sal", TipoMasaBase::BLANDA_ENRIQUECIDA,
                                          0, 0, 0, 0, 0, 1.0, 40, 10, {25, 12, 2026}, 3.00));

    // 6. Precarga inicial de entradas en el Kardex de Inventario (Árbol AVL)
    registrarMovimientoInventario({{1, 11, 2026}, "MP-HAR-001", TipoMovimiento::ENTRADA_COMPRA,
                                   150.0, "Compra inicial de harina panadera - Molino Los Andes"});
    registrarMovimientoInventario({{5, 11, 2026}, "MP-JAM-001", TipoMovimiento::ENTRADA_COMPRA,
                                   40.0, "Ingreso de jamon ahumado especial"});
    registrarMovimientoInventario({{10, 11, 2026}, "MP-LEV-001", TipoMovimiento::ENTRADA_COMPRA,
                                   20.0, "Reposicion de levadura fresca"});
    registrarMovimientoInventario({{15, 11, 2026}, "MP-AZU-001", TipoMovimiento::ENTRADA_COMPRA,
                                   50.0, "Compra de azucar blanquilla"});
}

bool SistemaPanaderia::guardarDatosJSON(const std::string& rutaArchivo) const
{
    try
    {
        std::filesystem::path ruta(rutaArchivo);
        if (!ruta.parent_path().empty())
        {
            std::filesystem::create_directories(ruta.parent_path());
        }
    }
    catch (...)
    {
        return false;
    }

    std::ofstream out(rutaArchivo);
    if (!out.is_open())
    {
        return false;
    }

    out << "{\n";

    // 1. Diccionario de nombres por SKU
    out << "  \"nombresSKU\": [\n";
    bool primeroNombre = true;
    for (const auto& par : catalogoNombresSKU)
    {
        if (!primeroNombre)
        {
            out << ",\n";
        }
        primeroNombre = false;
        out << "    {\"sku\": \"" << escaparJSON(par.first)
            << "\", \"nombre\": \"" << escaparJSON(par.second) << "\"}";
    }
    out << "\n  ],\n";

    // 2. Productos del catálogo (en orden de listaProductos)
    out << "  \"productos\": [\n";
    bool primeroProd = true;
    for (size_t i = 0; i < listaProductos.size(); ++i)
    {
        if (listaProductos[i] == nullptr)
        {
            continue;
        }
        if (!primeroProd)
        {
            out << ",\n";
        }
        primeroProd = false;
        escribirProductoJSON(out, *listaProductos[i], "    ");
    }
    out << "\n  ],\n";

    // 3. Recetas (árboles N-arios serializados en preorden)
    out << "  \"recetas\": [\n";
    bool primeraReceta = true;
    for (const auto& par : catalogoRecetas)
    {
        const Receta* rec = par.second;
        if (rec == nullptr)
        {
            continue;
        }
        if (!primeraReceta)
        {
            out << ",\n";
        }
        primeraReceta = false;

        std::vector<NodoRecetaPlano> nodosPlanos;
        aplanarNodosPreorden(rec->getRaiz(), -1, nodosPlanos);

        out << "    {\n"
            << "      \"sku\": \"" << escaparJSON(rec->getSKU()) << "\",\n"
            << "      \"nombre\": \"" << escaparJSON(rec->getNombre()) << "\",\n"
            << "      \"nodosReceta\": [\n";
        for (size_t j = 0; j < nodosPlanos.size(); ++j)
        {
            if (j > 0)
            {
                out << ",\n";
            }
            const ElementoReceta& elem = nodosPlanos[j].elemento;
            out << "        {\"padreIndice\": " << nodosPlanos[j].padreIndice
                << ", \"sku\": \"" << escaparJSON(elem.getCodigoSKU()) << "\""
                << ", \"cantidad\": " << elem.getCantidad()
                << ", \"unidad\": \"" << escaparJSON(elem.getUnidadMedida()) << "\""
                << ", \"rendimiento\": " << elem.getFactorRendimiento() << "}";
        }
        out << "\n      ]\n    }";
    }
    out << "\n  ],\n";

    // 4. Lotes de producción y trazabilidad (SLList)
    out << "  \"lotes\": [\n";
    for (size_t i = 0; i < lotesProduccion.size(); ++i)
    {
        if (i > 0)
        {
            out << ",\n";
        }
        const LoteProduccion& lote = lotesProduccion[i];
        out << "    {\n"
            << "      \"codigoLote\": \"" << escaparJSON(lote.getCodigoLote()) << "\",\n"
            << "      \"sku\": \"" << escaparJSON(lote.getCodigoSKU()) << "\",\n"
            << "      \"cantidadPlanificada\": " << lote.getCantidadPlanificada() << ",\n"
            << "      \"fechaElaboracion\": ";
        escribirFechaJSON(out, lote.getFechaElaboracion());
        out << ",\n      \"eventos\": [\n";

        bool primeroEv = true;
        for (const auto& ev : lote.getEventos())
        {
            if (!primeroEv)
            {
                out << ",\n";
            }
            primeroEv = false;
            out << "        {\"etapa\": \"" << escaparJSON(ev.etapa)
                << "\", \"descripcion\": \"" << escaparJSON(ev.descripcion)
                << "\", \"responsable\": \"" << escaparJSON(ev.responsable)
                << "\", \"fecha\": ";
            escribirFechaJSON(out, ev.fechaEvento);
            out << "}";
        }
        out << "\n      ]\n    }";
    }
    out << "\n  ],\n";

    // 5. Perecederos FEFO (copiamos el Min-Heap para conservar su estado intacto)
    out << "  \"perecederos\": [\n";
    GestionPerecederos copiaPerecederos = almacenPerecederos;
    bool primeroPer = true;
    while (copiaPerecederos.tienePerecederos())
    {
        if (!primeroPer)
        {
            out << ",\n";
        }
        primeroPer = false;
        ProductoPanaderia per = copiaPerecederos.despacharProximoAVencer();
        escribirProductoJSON(out, per, "    ");
    }
    out << "\n  ],\n";

    // 6. Kardex de inventario (AVL Tree en orden cronológico)
    out << "  \"kardex\": [\n";
    Designar::DynArray<MovimientoKardex> movimientos =
        kardex.consultarPorRangoFechas({0, 0, 0}, {31, 12, 9999});
    for (size_t i = 0; i < movimientos.size(); ++i)
    {
        if (i > 0)
        {
            out << ",\n";
        }
        const MovimientoKardex& mov = movimientos[i];
        out << "    {\"fecha\": ";
        escribirFechaJSON(out, mov.fechaTransaccion);
        out << ", \"sku\": \"" << escaparJSON(mov.codigoSKU) << "\""
            << ", \"tipo\": " << static_cast<int>(mov.tipo)
            << ", \"cantidad\": " << mov.cantidad
            << ", \"descripcion\": \"" << escaparJSON(mov.descripcion) << "\"}";
    }
    out << "\n  ]\n";

    out << "}\n";
    out.flush();
    return out.good();
}

bool SistemaPanaderia::cargarDatosJSON(const std::string& rutaArchivo)
{
    std::ifstream in(rutaArchivo);
    if (!in.is_open())
    {
        return false;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string contenido = buffer.str();
    if (contenido.empty())
    {
        return false;
    }

    JsonValor raiz;
    ParserJSON parser(contenido);
    if (!parser.parsearValor(raiz) || raiz.tipo != JsonTipo::Objeto)
    {
        return false;
    }

    // Limpiar estado previo solo después de validar el parseo completo
    catalogoRecetas.clear();
    catalogoProductos.clear();
    catalogoNombresSKU.clear();
    listaProductos.clear();
    lotesProduccion.clear();
    kardex = KardexInventario();
    almacenPerecederos = GestionPerecederos();
    limpiarMemoriaPropia();

    registrarNombresBaseSKU(*this);

    // 1. Nombres por SKU
    const JsonValor* arrNombres = raiz.buscar("nombresSKU");
    if (arrNombres != nullptr && arrNombres->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrNombres->arreglo)
        {
            const JsonValor* vSku = item.buscar("sku");
            const JsonValor* vNom = item.buscar("nombre");
            if (vSku != nullptr && vNom != nullptr)
            {
                registrarNombreSKU(vSku->comoCadena(), vNom->comoCadena());
            }
        }
    }

    // 2. Productos
    const JsonValor* arrProductos = raiz.buscar("productos");
    if (arrProductos != nullptr && arrProductos->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrProductos->arreglo)
        {
            if (item.tipo != JsonTipo::Objeto)
            {
                continue;
            }
            ProductoPanaderia prod = leerProductoJSON(item);
            if (!prod.getSKU().empty())
            {
                auto* ptr = new ProductoPanaderia(prod);
                if (!registrarProducto(ptr, true))
                {
                    delete ptr;
                }
            }
        }
    }

    // 3. Recetas y árboles N-arios
    const JsonValor* arrRecetas = raiz.buscar("recetas");
    if (arrRecetas != nullptr && arrRecetas->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrRecetas->arreglo)
        {
            if (item.tipo != JsonTipo::Objeto)
            {
                continue;
            }
            const JsonValor* vSku = item.buscar("sku");
            const JsonValor* vNom = item.buscar("nombre");
            std::string skuReceta = vSku ? vSku->comoCadena() : "";
            std::string nomReceta = vNom ? vNom->comoCadena() : "";
            if (skuReceta.empty())
            {
                continue;
            }

            const JsonValor* arrNodos = item.buscar("nodosReceta");
            double rendRaiz = 1.0;
            if (arrNodos != nullptr && arrNodos->tipo == JsonTipo::Arreglo && !arrNodos->arreglo.empty())
            {
                const JsonValor* vRend = arrNodos->arreglo[0].buscar("rendimiento");
                if (vRend != nullptr)
                {
                    rendRaiz = vRend->comoNumero(1.0);
                }
            }

            ProductoPanaderia* prodBase = buscarProducto(skuReceta);
            Receta* receta = nullptr;
            if (prodBase != nullptr)
            {
                receta = new Receta(*prodBase);
                if (receta->getRaiz() != nullptr)
                {
                    receta->getRaiz()->get_key().setFactorRendimiento(rendRaiz);
                }
            }
            else
            {
                ProductoPanaderia temp(skuReceta, nomReceta, TipoMasaBase::BLANDA_ENRIQUECIDA,
                                       0, 0, 0, 0, 0, rendRaiz, 0, 0, {1, 1, 2026}, 0.0);
                receta = new Receta(temp);
            }

            if (arrNodos != nullptr && arrNodos->tipo == JsonTipo::Arreglo && !arrNodos->arreglo.empty())
            {
                std::vector<NodoReceta*> nodosCreados;
                nodosCreados.push_back(receta->getRaiz());

                for (size_t j = 1; j < arrNodos->arreglo.size(); ++j)
                {
                    const JsonValor& nodoJson = arrNodos->arreglo[j];
                    const JsonValor* vPadre = nodoJson.buscar("padreIndice");
                    const JsonValor* vNSku = nodoJson.buscar("sku");
                    const JsonValor* vCant = nodoJson.buscar("cantidad");
                    const JsonValor* vUnid = nodoJson.buscar("unidad");
                    const JsonValor* vRend = nodoJson.buscar("rendimiento");

                    int padreIdx = vPadre ? vPadre->comoEntero(0) : 0;
                    std::string nSku = vNSku ? vNSku->comoCadena() : "";
                    double nCant = vCant ? vCant->comoNumero(0.0) : 0.0;
                    std::string nUnid = vUnid ? vUnid->comoCadena("kg") : "kg";
                    double nRend = vRend ? vRend->comoNumero(1.0) : 1.0;

                    NodoReceta* padre = (padreIdx >= 0 && static_cast<size_t>(padreIdx) < nodosCreados.size())
                                            ? nodosCreados[static_cast<size_t>(padreIdx)]
                                            : receta->getRaiz();

                    receta->agregarElemento(padre, ElementoReceta(nSku, nCant, nUnid, nRend));

                    NodoReceta* ultimoHijo = nullptr;
                    for (NodoReceta::ChildrenIterator it(*padre); it.has_current(); it.next())
                    {
                        ultimoHijo = it.get_current();
                    }
                    nodosCreados.push_back(ultimoHijo != nullptr ? ultimoHijo : receta->getRaiz());
                }
            }

            if (!registrarReceta(receta, true))
            {
                delete receta;
            }
        }
    }

    // 4. Lotes de producción (SLList)
    const JsonValor* arrLotes = raiz.buscar("lotes");
    if (arrLotes != nullptr && arrLotes->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrLotes->arreglo)
        {
            if (item.tipo != JsonTipo::Objeto)
            {
                continue;
            }
            const JsonValor* vCod = item.buscar("codigoLote");
            const JsonValor* vSku = item.buscar("sku");
            const JsonValor* vCant = item.buscar("cantidadPlanificada");

            LoteProduccion lote(
                vCod ? vCod->comoCadena() : "",
                vSku ? vSku->comoCadena() : "",
                vCant ? vCant->comoNumero(0.0) : 0.0,
                leerFechaJSON(item.buscar("fechaElaboracion"))
            );

            const JsonValor* arrEv = item.buscar("eventos");
            if (arrEv != nullptr && arrEv->tipo == JsonTipo::Arreglo)
            {
                for (const auto& evJson : arrEv->arreglo)
                {
                    if (evJson.tipo != JsonTipo::Objeto)
                    {
                        continue;
                    }
                    const JsonValor* vEt = evJson.buscar("etapa");
                    const JsonValor* vDesc = evJson.buscar("descripcion");
                    const JsonValor* vResp = evJson.buscar("responsable");
                    lote.registrarEvento(
                        vEt ? vEt->comoCadena() : "",
                        vDesc ? vDesc->comoCadena() : "",
                        vResp ? vResp->comoCadena() : "",
                        leerFechaJSON(evJson.buscar("fecha"))
                    );
                }
            }

            registrarLote(lote);
        }
    }

    // 5. Perecederos FEFO (Min-Heap)
    const JsonValor* arrPer = raiz.buscar("perecederos");
    if (arrPer != nullptr && arrPer->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrPer->arreglo)
        {
            if (item.tipo != JsonTipo::Objeto)
            {
                continue;
            }
            ProductoPanaderia per = leerProductoJSON(item);
            if (!per.getSKU().empty())
            {
                registrarNombreSKU(per.getSKU(), per.getNombre());
                registrarPerecedero(per);
            }
        }
    }

    // 6. Movimientos del Kardex (AVL Tree)
    const JsonValor* arrKardex = raiz.buscar("kardex");
    if (arrKardex != nullptr && arrKardex->tipo == JsonTipo::Arreglo)
    {
        for (const auto& item : arrKardex->arreglo)
        {
            if (item.tipo != JsonTipo::Objeto)
            {
                continue;
            }
            const JsonValor* vSku = item.buscar("sku");
            const JsonValor* vTipo = item.buscar("tipo");
            const JsonValor* vCant = item.buscar("cantidad");
            const JsonValor* vDesc = item.buscar("descripcion");

            int tipoInt = vTipo ? vTipo->comoEntero(0) : 0;
            if (tipoInt < static_cast<int>(TipoMovimiento::ENTRADA_COMPRA) ||
                tipoInt > static_cast<int>(TipoMovimiento::VENTA_POS))
            {
                tipoInt = static_cast<int>(TipoMovimiento::AJUSTE_INVENTARIO);
            }

            MovimientoKardex mov{
                leerFechaJSON(item.buscar("fecha")),
                vSku ? vSku->comoCadena() : "",
                static_cast<TipoMovimiento>(tipoInt),
                vCant ? vCant->comoNumero(0.0) : 0.0,
                vDesc ? vDesc->comoCadena() : ""
            };
            registrarMovimientoInventario(mov);
        }
    }

    return true;
}

void SistemaPanaderia::inicializarConPersistencia(const std::string& rutaArchivo)
{
    if (!cargarDatosJSON(rutaArchivo))
    {
        cargarDatosPorDefecto();
        guardarDatosJSON(rutaArchivo);
    }
}

fecha obtenerFechaActual()
{
    std::time_t t = std::time(nullptr);
    std::tm* ahora = std::localtime(&t);

    return {
        ahora->tm_mday,         // Día del mes (1-31)
        ahora->tm_mon + 1,      // Mes (0-11) -> sumamos 1 para (1-12)
        ahora->tm_year + 1900   // Años desde 1900 -> sumamos 1900
    };
}
