#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include "InventoryManager.h"
#include "component.h"
#include "report.h"

class AddDialog : public QWidget {
    Q_OBJECT
public:
    explicit AddDialog(QWidget *parent = nullptr);

    QString getName() const;
    QString getType() const;
    int getQuantity() const;
    QString getLocation() const;
    QDate getDate() const;

signals:
    void accepted();
    void cancelled();

private slots:
    void onOk();
    void onCancel();

private:
    QLineEdit *nameEdit;
    QLineEdit *typeEdit;
    QSpinBox  *qtySpin;
    QLineEdit *locEdit;
    QDateEdit *dateEdit;
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QSqlDatabase db, QWidget *parent = nullptr);

private slots:
    void onEdit();
    void onAdd();
    void onDelete();
    void onExport();
    void onLowStock();
    void onSearch(const QString &text);

private:
    void refreshModel();
    void checkLowStockOnStart();
    void onLoadDefaults();
    void onRestoreDefaults();

private:
    InventoryManager manager;
    QSqlQueryModel *model;
    QSortFilterProxyModel *proxy;
    QTableView *tableView;
    QLineEdit *searchEdit;

    const int lowStockThreshold = 5;
};

#endif // MAINWINDOW_H
