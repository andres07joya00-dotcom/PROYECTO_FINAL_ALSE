#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QObject>
#include <QList>
#include <QSqlDatabase>

struct InventoryItem {
    int id;
    QString nombre;
    QString tipo;
    int cantidad;
    QString ubicacion;
    QString fechaAdquisicion;
};

class InventoryManager : public QObject
{
    Q_OBJECT

public:
    explicit InventoryManager(QSqlDatabase database,
                              QObject *parent = nullptr);

    bool createTable();
    bool addItem(const QString &nombre,
                 const QString &tipo,
                 int cantidad,
                 const QString &ubicacion,
                 const QString &fechaAdquisicion);

    bool updateQuantity(int id, int newQuantity);
    bool removeItem(int id);
    QList<InventoryItem> getAllItems();

private:
    QSqlDatabase db;
};

#endif // INVENTORYMANAGER_H
