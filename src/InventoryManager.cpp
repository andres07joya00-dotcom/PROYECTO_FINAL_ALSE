#include "InventoryManager.h"
#include <QSqlRecord>
#include <QDebug>

InventoryManager::InventoryManager(QObject *parent)
    : QObject(parent)
{
}

// Obtiene la base de datos abierta en DatabaseManager (QSqlDatabase::addDatabase)
QSqlDatabase InventoryManager::db() const
{
    return QSqlDatabase::database();
}

bool InventoryManager::createTable()
{
    QSqlQuery query(db());

    QString sql =
        "CREATE TABLE IF NOT EXISTS inventario ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nombre TEXT NOT NULL, "
        "cantidad INTEGER NOT NULL, "
        "precio REAL NOT NULL"
        ");";

    if (!query.exec(sql)) {
        qDebug() << "Error creando tabla inventario:" << query.lastError();
        return false;
    }

    return true;
}

bool InventoryManager::addItem(const QString &nombre, int cantidad, double precio)
{
    QSqlQuery query(db());

    query.prepare("INSERT INTO inventario (nombre, cantidad, precio) "
                  "VALUES (:nombre, :cantidad, :precio)");

    query.bindValue(":nombre", nombre);
    query.bindValue(":cantidad", cantidad);
    query.bindValue(":precio", precio);

    if (!query.exec()) {
        qDebug() << "Error insertando item:" << query.lastError();
        return false;
    }

    return true;
}

bool InventoryManager::updateQuantity(int id, int newQuantity)
{
    QSqlQuery query(db());

    query.prepare("UPDATE inventario SET cantidad = :cantidad WHERE id = :id");
    query.bindValue(":cantidad", newQuantity);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Error actualizando cantidad:" << query.lastError();
        return false;
    }

    return true;
}

bool InventoryManager::removeItem(int id)
{
    QSqlQuery query(db());

    query.prepare("DELETE FROM inventario WHERE id = :id");
    query.bindValue(":id", id);

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
    query.prepare("SELECT id, nombre, cantidad, precio FROM inventario");

    if (!query.exec()) {
        qDebug() << "Error obteniendo items:" << query.lastError();
        return items;
    }

    while (query.next()) {
        InventoryItem item;
        item.id       = query.value("id").toInt();
        item.nombre   = query.value("nombre").toString();
        item.cantidad = query.value("cantidad").toInt();
        item.precio   = query.value("precio").toDouble();

        items.append(item);
    }

    return items;
}

