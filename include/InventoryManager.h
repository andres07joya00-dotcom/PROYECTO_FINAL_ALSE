#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>

    struct InventoryItem {
        int id;
QString nombre;
int cantidad;
double precio;
};

class InventoryManager : public QObject
{
    Q_OBJECT

public:
    explicit InventoryManager(QObject *parent = nullptr);

    bool createTable();
    bool addItem(const QString &nombre, int cantidad, double precio);
    bool updateQuantity(int id, int newQuantity);
    bool removeItem(int id);

    QList<InventoryItem> getAllItems();

private:
    QSqlDatabase db() const;
};

#endif // INVENTORYMANAGER_H
