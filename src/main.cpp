#include <QCoreApplication>
#include <QSqlDatabase>
#include <QDebug>
#include "InventoryManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. Abrir base de datos SQLite
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("inventario.db");

    if (!db.open()) {
        qDebug() << "No se pudo abrir la base de datos:" << db.lastError();
        return -1;
    }

    qDebug() << "Base de datos abierta correctamente.";

    // 2. Crear gestor de inventario
    InventoryManager manager;

    // 3. Crear tabla
    manager.createTable();

    // 4. Insertar datos de prueba
    manager.addItem("Laptop", 5, 3500.0);
    manager.addItem("Mouse", 20, 25.5);
    manager.addItem("Teclado", 15, 40.0);

    // 5. Leer todos los items
    auto items = manager.getAllItems();
    qDebug() << "--- LISTA DE ITEMS ---";
    for (const auto &item : items) {
        qDebug() << "ID:" << item.id
                 << "Nombre:" << item.nombre
                 << "Cantidad:" << item.cantidad
                 << "Precio:" << item.precio;
    }

    // 6. Actualizar cantidad
    if (items.size() > 0) {
        manager.updateQuantity(items[0].id, 99);
    }

    // 7. Eliminar un item
    if (items.size() > 1) {
        manager.removeItem(items[1].id);
    }

    qDebug() << "Pruebas completadas.";

    return a.exec();
}
