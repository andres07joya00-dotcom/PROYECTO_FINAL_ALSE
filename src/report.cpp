#include "report.h"
#include "InventoryManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

CSVReport::CSVReport()
{
}

bool CSVReport::generate(const QList<InventoryItem> &items,
                         const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "No se puede abrir archivo CSV:" << filePath;
        return false;
    }

    QTextStream out(&file);

    out << "ID;Nombre;Tipo;Cantidad;Ubicacion;FechaAdquisicion\n";

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
