#include <QApplication>
#include "InventoryManager.h"
#include "report.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Abrir DB
    DatabaseManager::getDatabase();

    // Crear manejadores
    InventoryManager manager;
    CSVReportGenerator report;

    // Crear tabla
    manager.createTable();

    // Insertar algunos datos de prueba
    manager.addItem("laptop", 50, 1.20);
    manager.addItem("teclado", 40, 1.50);

    // Obtener datos
    QList<InventoryItem> items = manager.getAllItems();

    // Generar reporte
    report.generateReport(items, "reporte_inventario.csv");

    return 0;
}


