#include <QCoreApplication>
#include <QDebug>

#include "DatabaseManager.h"
#include "InventoryManager.h"
#include "report.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSqlDatabase db = DatabaseManager::getDatabase();

    InventoryManager manager(db);

    manager.createTable();

    manager.addItem("Arduino Uno", "Electrónica", 10, "Bodega", "2025-01-15");
    manager.addItem("Sensor HC-SR04", "Sensor", 25, "Estante B", "2025-02-02");

    QList<InventoryItem> lista = manager.getAllItems();

    CSVReport report;
    report.generate(lista, "reporte_inventario.csv");

    qDebug() << "Reporte generado.";

    return 0;
}


