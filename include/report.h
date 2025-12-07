#ifndef CSVREPORTGENERATOR_H
#define CSVREPORTGENERATOR_H

#include <QString>
#include <QList>
#include "InventoryManager.h"

class CSVReportGenerator
{
public:
    bool generateReport(const QList<InventoryItem> &items, const QString &filePath);
};

#endif // CSVREPORTGENERATOR_H
