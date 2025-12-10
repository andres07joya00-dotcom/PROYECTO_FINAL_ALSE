/**
 * @file CSVReport.h
 * @brief Declaración de la clase responsable de generar reportes en formato CSV.
 *
 * La clase CSVReport permite crear un archivo CSV a partir de una lista
 * de objetos InventoryItem, exportando la información del inventario
 * para análisis o respaldo externo.
 */

#ifndef CSVREPORT_H
#define CSVREPORT_H

#include <QString>
#include <QList>

/**
 * @struct InventoryItem
 * @brief Estructura que representa un elemento del inventario.
 *
 * Esta estructura es declarada en otro archivo (InventoryManager.h),
 * y se utiliza aquí solo como referencia para exportar datos.
 */
struct InventoryItem;

/**
 * @class CSVReport
 * @brief Clase encargada de generar reportes CSV del inventario.
 *
 * Esta clase ofrece un método principal para crear un archivo CSV
 * que contiene todos los campos relevantes de cada InventoryItem.
 */
class CSVReport
{
public:
    /**
     * @brief Constructor por defecto.
     *
     * No realiza ninguna operación específica, pero se define
     * para mantener consistencia en la estructura del proyecto.
     */
    CSVReport();

    /**
     * @brief Genera un archivo CSV con la lista de items proporcionada.
     *
     * El archivo generado contiene encabezados y cada fila representa
     * un elemento del inventario con su información completa.
     *
     * @param items Lista de elementos del inventario a exportar.
     * @param filePath Ruta completa donde se guardará el archivo CSV.
     *
     * @return true si el archivo se generó correctamente, false en caso contrario.
     */
    bool generate(const QList<InventoryItem> &items,
                  const QString &filePath);
};

#endif // CSVREPORT_H

