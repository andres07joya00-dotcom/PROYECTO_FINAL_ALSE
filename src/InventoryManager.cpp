#include "InventoryManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

InventoryManager::InventoryManager(QObject *parent)
    : QObject(parent)
{
    DatabaseManager::getDatabase();
}

QSqlDatabase InventoryManager::db() const
{
    return DatabaseManager::getDatabase();
}

bool InventoryManager::createTable()
{
    QSqlQuery query(db());

    QString sql =
        "CREATE TABLE IF NOT EXISTS inventario ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nombre TEXT NOT NULL,"
        "cantidad INTEGER NOT NULL,"
        "precio REAL NOT NULL"
        ");";

    if (!query.exec(sql)) {
        qDebug() << "Error creando tabla:" << query.lastError();
        return false;
    }
    return true;
}

bool InventoryManager::addItem(const QString &nombre, int cantidad, double precio)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO inventario (nombre, cantidad, precio) VALUES (?, ?, ?)");
    query.addBindValue(nombre);
    query.addBindValue(cantidad);
    query.addBindValue(precio);

    if (!query.exec()) {
        qDebug() << "Error insertando item:" << query.lastError();
        return false;
    }

    return true;
}

bool InventoryManager::updateQuantity(int id, int newQuantity)
{
    QSqlQuery query(db());
    query.prepare("UPDATE inventario SET cantidad = ? WHERE id = ?");
    query.addBindValue(newQuantity);
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Error actualizando cantidad:" << query.lastError();
        return false;
    }

    return true;
}

bool InventoryManager::removeItem(int id)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM inventario WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Error eliminando item:" << query.lastError();
        return false;
    }

    return true;
}

QList<InventoryItem> InventoryManager::getAllItems()
{
    QList<InventoryItem> items;
    QSqlQuery query(db());

    if (!query.exec("SELECT id, nombre, cantidad, precio FROM inventario")) {
        qDebug() << "Error obteniendo items:" << query.lastError();
        return items;
    }

    while (query.next()) {
        InventoryItem it;
        it.id = query.value(0).toInt();
        it.nombre = query.value(1).toString();
        it.cantidad = query.value(2).toInt();
        it.precio = query.value(3).toDouble();
        items.append(it);
    }

    return items;
}
