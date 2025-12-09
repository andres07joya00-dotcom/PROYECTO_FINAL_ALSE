#include "InventoryManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

InventoryManager::InventoryManager(QSqlDatabase database, QObject *parent)
    : QObject(parent), db(database)
{
    if (!db.isValid()) {
        qFatal("ERROR FATAL: Base de datos no válida. "
               "Asegúrate de usar DatabaseManager::getDatabase().");
    }
}

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

bool InventoryManager::updateQuantity(int id, int newQuantity)
{
    QSqlQuery query(db);
    query.prepare("UPDATE inventario SET cantidad = ? WHERE id = ?");
    query.addBindValue(newQuantity);
    query.addBindValue(id);
    return query.exec();
}

bool InventoryManager::removeItem(int id)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM inventario WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

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

InventoryItem InventoryManager::getItemById(int id)
{
    InventoryItem it;

    QSqlQuery query(db);
    query.prepare("SELECT id, nombre, tipo, cantidad, ubicacion, fechaAdquisicion "
                  "FROM inventario WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        return it;  // vacío si no encuentra
    }

    it.id = query.value(0).toInt();
    it.nombre = query.value(1).toString();
    it.tipo = query.value(2).toString();
    it.cantidad = query.value(3).toInt();
    it.ubicacion = query.value(4).toString();
    it.fechaAdquisicion = query.value(5).toString();

    return it;
}
