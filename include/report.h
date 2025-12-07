#ifndef CSVREPORT_H
#define CSVREPORT_H

#include <QString>
#include <QList>

struct InventoryItem;

class CSVReport
{
public:
    CSVReport();

    // Genera archivo CSV con la lista de items
    bool generate(const QList<InventoryItem> &items,
                  const QString &filePath);
};

#endif // CSVREPORT_H
