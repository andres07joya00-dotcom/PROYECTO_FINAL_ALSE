#include "report.h"
#include "InventoryManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

/**
 * @brief Constructor por defecto de la clase CSVReport.
 */
CSVReport::CSVReport()
{
}

/**
 * @brief Genera un archivo CSV con la información del inventario.
 *
 * Abre o crea un archivo en la ruta indicada y escribe una fila con
 * los encabezados seguida por una línea por cada elemento del inventario.
 * Los campos de texto son encapsulados entre comillas dobles.
 *
 * Ejemplo de formato generado:
 * @code
 * ID;Nombre;Tipo;Cantidad;Ubicacion;FechaAdquisicion
 * 1;"Resistencia";"Electrónico";50;"Caja A";"2024-03-01"
 * @endcode
 *
 * @param items Lista de elementos del inventario.
 * @param filePath Ruta completa del archivo CSV a generar.
 *
 * @return `true` si el archivo fue generado correctamente,
 *         `false` si no fue posible abrirlo para escritura.
 */
bool CSVReport::generate(const QList<InventoryItem> &items,
                         const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "No se puede abrir archivo CSV:" << filePath;
        return false;
    }

    QTextStream out(&file);

    // Encabezados del CSV
    out << "ID;Nombre;Tipo;Cantidad;Ubicacion;FechaAdquisicion\n";

    // Datos de cada item
    for (const InventoryItem &it : items) {
        out << it.id << ";"
            << "\"" << it.nombre << "\";"
            << "\"" << it.tipo << "\";"
            << it.cantidad << ";"
            << "\"" << it.ubicacion << "\";"
            << "\"" << it.fechaAdquisicion << "\"\n";
    }

    file.close();
    return true;
}
