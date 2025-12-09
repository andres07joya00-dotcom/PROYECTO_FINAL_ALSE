#ifndef LOWSTOCKDELEGATE_H
#define LOWSTOCKDELEGATE_H

#include <QStyledItemDelegate>
#include "InventoryManager.h"

class LowStockDelegate : public QStyledItemDelegate
{
public:
    LowStockDelegate(int threshold, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), limit(threshold) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);

        // columna 3 = cantidad
        if (index.column() == 3 && index.data().toInt() < limit) {
            opt.palette.setColor(QPalette::Text, Qt::red);
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    int limit;
};

#endif // LOWSTOCKDELEGATE_H
