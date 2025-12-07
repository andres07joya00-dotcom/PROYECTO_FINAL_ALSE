#include "report.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

bool CSVReportGenerator::generateReport(const QList<InventoryItem> &items,
                                        const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error abriendo archivo CSV.";
        return false;
    }

    QTextStream out(&file);

    out << "ID,Nombre,Cantidad,Precio\n";

    for (const InventoryItem &item : items) {
        out << item.id << ","
            << item.nombre << ","
            << item.cantidad << ","
            << item.precio << "\n";
    }

    file.close();
    qDebug() << "Reporte CSV generado en:" << filePath;

    return true;
}
