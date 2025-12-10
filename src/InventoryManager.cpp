#include "InventoryManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/**
 * @brief Constructor de InventoryManager.
 *
 * Inicializa el gestor de inventario con la base de datos proporcionada.
 * Se verifica que la base de datos sea válida; de no serlo, la ejecución
 * del programa se detiene mediante qFatal, ya que el inventario depende
 * completamente del acceso a la base de datos.
 *
 * @param database Conexión a la base de datos SQLite.
 * @param parent Objeto padre opcional según el sistema de jerarquía de Qt.
 */
InventoryManager::InventoryManager(QSqlDatabase database, QObject *parent)
    : QObject(parent), db(database)
{
    if (!db.isValid()) {
        qFatal("ERROR FATAL: Base de datos no válida. "
               "Asegúrate de usar DatabaseManager::getDatabase().");
    }
}

/**
 * @brief Crea la tabla principal del inventario si no existe.
 *
 * La tabla contiene los campos:
 * - id (PRIMARY KEY AUTOINCREMENT)
 * - nombre
 * - tipo
 * - cantidad
 * - ubicacion
 * - fechaAdquisicion
 *
 * @return true si la tabla se creó o ya existía; false si hubo error en la ejecución.
 */
bool InventoryManager::createTable()
{
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS inventario ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nombre TEXT NOT NULL,"
        "tipo TEXT NOT NULL,"
        "cantidad INTEGER NOT NULL,"
        "ubicacion TEXT NOT NULL,"
        "fechaAdquisicion TEXT NOT NULL"
        ");";

    return query.exec(sql);
}

/**
 * @brief Inserta un nuevo elemento en la tabla inventario.
 *
 * @param nombre Nombre del componente.
 * @param tipo Tipo o categoría.
 * @param cantidad Cantidad disponible.
 * @param ubicacion Ubicación física.
 * @param fechaAdquisicion Fecha de adquisición.
 *
 * @return true si la operación fue exitosa, false si el INSERT falló.
 */
bool InventoryManager::addItem(const QString &nombre,
                               const QString &tipo,
                               int cantidad,
                               const QString &ubicacion,
                               const QString &fechaAdquisicion)
{
    QSqlQuery query(db);

    query.prepare(
        "INSERT INTO inventario "
        "(nombre, tipo, cantidad, ubicacion, fechaAdquisicion) "
        "VALUES (?, ?, ?, ?, ?)"
    );

    query.addBindValue(nombre);
    query.addBindValue(tipo);
    query.addBindValue(cantidad);
    query.addBindValue(ubicacion);
    query.addBindValue(fechaAdquisicion);

    return query.exec();
}

/**
 * @brief Actualiza únicamente la cantidad de un elemento identificado por id.
 *
 * @param id Identificador del registro a actualizar.
 * @param newQuantity Nueva cantidad a asignar.
 *
 * @return true si la operación fue exitosa, false si falló.
 */
bool InventoryManager::updateQuantity(int id, int newQuantity)
{
    QSqlQuery query(db);
    query.prepare("UPDATE inventario SET cantidad = ? WHERE id = ?");
    query.addBindValue(newQuantity);
    query.addBindValue(id);
    return query.exec();
}

/**
 * @brief Elimina un elemento del inventario.
 *
 * @param id Identificador del elemento a borrar.
 *
 * @return true si el registro fue eliminado correctamente.
 */
bool InventoryManager::removeItem(int id)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM inventario WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

/**
 * @brief Obtiene todos los registros almacenados en la tabla inventario.
 *
 * @return Lista con todos los InventoryItem encontrados.
 */
QList<InventoryItem> InventoryManager::getAllItems()
{
    QList<InventoryItem> items;
    QSqlQuery query(db);

    query.exec("SELECT id, nombre, tipo, cantidad, ubicacion, fechaAdquisicion FROM inventario");

    while (query.next()) {
        InventoryItem it;
        it.id = query.value(0).toInt();
        it.nombre = query.value(1).toString();
        it.tipo = query.value(2).toString();
        it.cantidad = query.value(3).toInt();
        it.ubicacion = query.value(4).toString();
        it.fechaAdquisicion = query.value(5).toString();
        items.append(it);
    }
    return items;
}

/**
 * @brief Actualiza todos los campos de un elemento del inventario.
 *
 * @param id Identificador del registro a actualizar.
 * @param nombre Nuevo nombre.
 * @param tipo Nuevo tipo.
 * @param cantidad Nueva cantidad.
 * @param ubicacion Nueva ubicación.
 * @param fechaAdquisicion Nueva fecha de adquisición.
 *
 * @return true si el registro fue modificado correctamente.
 */
bool InventoryManager::updateItem(int id,
                                  const QString &nombre,
                                  const QString &tipo,
                                  int cantidad,
                                  const QString &ubicacion,
                                  const QString &fechaAdquisicion)
{
    QSqlQuery query(db);

    query.prepare(
        "UPDATE inventario SET "
        "nombre = ?, tipo = ?, cantidad = ?, ubicacion = ?, fechaAdquisicion = ? "
        "WHERE id = ?"
    );

    query.addBindValue(nombre);
    query.addBindValue(tipo);
    query.addBindValue(cantidad);
    query.addBindValue(ubicacion);
    query.addBindValue(fechaAdquisicion);
    query.addBindValue(id);

    return query.exec();
}

/**
 * @brief Obtiene un único elemento del inventario según su id.
 *
 * @param id Identificador buscado.
 *
 * @return Estructura InventoryItem con los datos encontrados.
 *         Si no existe, retorna un objeto vacío con valores por defecto.
 */
InventoryItem InventoryManager::getItemById(int id)
{
    InventoryItem it;

    QSqlQuery query(db);
    query.prepare("SELECT id, nombre, tipo, cantidad, ubicacion, fechaAdquisicion "
                  "FROM inventario WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        return it;  // vacío si no se encuentra
    }

    it.id = query.value(0).toInt();
    it.nombre = query.value(1).toString();
    it.tipo = query.value(2).toString();
    it.cantidad = query.value(3).toInt();
    it.ubicacion = query.value(4).toString();
    it.fechaAdquisicion = query.value(5).toString();

    return it;
}
