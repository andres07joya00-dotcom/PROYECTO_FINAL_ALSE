#ifndef LOWSTOCKDELEGATE_H
#define LOWSTOCKDELEGATE_H

#include <QStyledItemDelegate>
#include "InventoryManager.h"

/**
 * @class LowStockDelegate
 * @brief Delegate que resalta en rojo los elementos con bajo stock.
 *
 * Esta clase hereda de QStyledItemDelegate y se encarga de modificar
 * la apariencia de las celdas en un QTableView o QListView cuando
 * la cantidad de un componente es menor a un umbral definido.
 *
 * Útil para visualizar rápidamente inventario crítico en interfaces Qt.
 */
class LowStockDelegate : public QStyledItemDelegate
{
public:
    /**
     * @brief Constructor del delegado de bajo stock.
     *
     * @param threshold Umbral mínimo de cantidad antes de marcar el texto en rojo.
     * @param parent Objeto padre opcional según la jerarquía Qt.
     */
    LowStockDelegate(int threshold, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), limit(threshold) {}

    /**
     * @brief Sobrescribe el método paint para aplicar resaltado.
     *
     * Cambia el color del texto a rojo cuando la columna correspondiente
     * a la cantidad (columna 3 del modelo) tiene un valor menor al umbral
     * definido en @ref limit.
     *
     * @param painter Objeto empleado para realizar el dibujo.
     * @param option Opciones de estilo de la celda.
     * @param index Índice del elemento dentro del modelo.
     */
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);

        // Columna 3 → cantidad del componente
        if (index.column() == 3 && index.data().toInt() < limit) {
            opt.palette.setColor(QPalette::Text, Qt::red);
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    int limit; ///< Umbral mínimo de cantidad para marcar el texto en rojo.
};

#endif // LOWSTOCKDELEGATE_H
