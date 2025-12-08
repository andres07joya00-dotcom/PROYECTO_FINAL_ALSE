#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QDebug>

#include "report.h"

// Helpers
static QString dateToString(const QDate &d) {
    return d.toString("yyyy-MM-dd");
}

static Component inventoryItemToComponent(const InventoryItem &it) {
    return Component(it.id, it.nombre, it.tipo, it.cantidad, it.ubicacion, it.fechaAdquisicion);
}

/* ------------------------------
 *      AddDialog
 * ------------------------------ */
AddDialog::AddDialog(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Agregar componente");
    QVBoxLayout *v = new QVBoxLayout(this);

    nameEdit = new QLineEdit(); nameEdit->setPlaceholderText("Nombre");
    typeEdit = new QLineEdit(); typeEdit->setPlaceholderText("Tipo");
    qtySpin = new QSpinBox(); qtySpin->setRange(0, 1000000); qtySpin->setValue(1);
    locEdit = new QLineEdit(); locEdit->setPlaceholderText("Ubicación");
    dateEdit = new QDateEdit(QDate::currentDate()); dateEdit->setCalendarPopup(true);

    QPushButton *btnOk = new QPushButton("Agregar");
    QPushButton *btnCancel = new QPushButton("Cancelar");

    v->addWidget(new QLabel("Nombre:"));    v->addWidget(nameEdit);
    v->addWidget(new QLabel("Tipo:"));      v->addWidget(typeEdit);
    v->addWidget(new QLabel("Cantidad:"));  v->addWidget(qtySpin);
    v->addWidget(new QLabel("Ubicación:")); v->addWidget(locEdit);
    v->addWidget(new QLabel("Fecha adquisición:")); v->addWidget(dateEdit);

    QHBoxLayout *h = new QHBoxLayout();
    h->addWidget(btnOk);
    h->addWidget(btnCancel);
    v->addLayout(h);

    connect(btnOk, &QPushButton::clicked, this, &AddDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &AddDialog::onCancel);
}

QString AddDialog::getName() const { return nameEdit->text().trimmed(); }
QString AddDialog::getType() const { return typeEdit->text().trimmed(); }
int AddDialog::getQuantity() const { return qtySpin->value(); }
QString AddDialog::getLocation() const { return locEdit->text().trimmed(); }
QDate AddDialog::getDate() const { return dateEdit->date(); }

void AddDialog::onOk() { emit accepted(); }
void AddDialog::onCancel() { emit cancelled(); }

/* ------------------------------
 *      MainWindow
 * ------------------------------ */
MainWindow::MainWindow(QSqlDatabase db, QWidget *parent)
    : QWidget(parent), manager(db)
{
    setWindowTitle("Gestión de Inventario");
    resize(1000, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();

    searchEdit = new QLineEdit(); searchEdit->setPlaceholderText("Buscar...");
    QPushButton *btnAdd = new QPushButton("Agregar");
    QPushButton *btnDelete = new QPushButton("Eliminar seleccionado");
    QPushButton *btnExport = new QPushButton("Exportar CSV");
    QPushButton *btnLowStock = new QPushButton("Revisar stock bajo");

    topLayout->addWidget(new QLabel("Buscar:"));
    topLayout->addWidget(searchEdit);
    topLayout->addWidget(btnAdd);
    topLayout->addWidget(btnDelete);
    topLayout->addWidget(btnLowStock);
    topLayout->addWidget(btnExport);

    mainLayout->addLayout(topLayout);

    model = new QSqlQueryModel(this);
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1);

    tableView = new QTableView();
    tableView->setModel(proxy);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(tableView);

    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(btnLowStock, &QPushButton::clicked, this, &MainWindow::onLowStock);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);

    if (!manager.createTable()) {
        QMessageBox::critical(this, "Error", "No se pudo crear la tabla de inventario.");
    }

    refreshModel();
    checkLowStockOnStart();
}

void MainWindow::onAdd()
{
    AddDialog *dlg = new AddDialog();
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->show();

    connect(dlg, &AddDialog::accepted, this, [this, dlg]() {
        if (dlg->getName().isEmpty() || dlg->getType().isEmpty()) {
            QMessageBox::warning(this, "Validación", "Nombre y tipo son obligatorios.");
            return;
        }

        Component c(0,
                    dlg->getName(),
                    dlg->getType(),
                    dlg->getQuantity(),
                    dlg->getLocation(),
                    dateToString(dlg->getDate()));

        if (!manager.addItem(c.getName(), c.getType(), c.getQuantity(), c.getLocation(), c.getPurchaseDate())) {
            QMessageBox::critical(this, "Error", "No se pudo agregar el componente.");
        } else {
            refreshModel();
        }
        dlg->close();
        dlg->deleteLater();
    });

    connect(dlg, &AddDialog::cancelled, dlg, &AddDialog::close);
}

void MainWindow::onDelete()
{
    QModelIndexList sel = tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona una fila primero.");
        return;
    }

    QModelIndex idx = sel.first();
    QModelIndex src = proxy->mapToSource(idx);
    int id = model->data(model->index(src.row(), 0)).toInt();

    if (QMessageBox::question(this, "Confirmar",
                              QString("¿Eliminar el registro ID %1?").arg(id)) == QMessageBox::Yes)
    {
        if (!manager.removeItem(id)) {
            QMessageBox::critical(this, "Error", "No se pudo eliminar.");
        } else {
            refreshModel();
        }
    }
}

void MainWindow::onExport()
{
    QString filename = QFileDialog::getSaveFileName(
        this, "Guardar CSV", "reporte.csv", "CSV files (*.csv)");

    if (filename.isEmpty()) return;

    CSVReport report;
    if (report.generate(manager.getAllItems(), filename)) {
        QMessageBox::information(this, "OK", "Reporte exportado.");
    } else {
        QMessageBox::critical(this, "Error", "No se pudo exportar.");
    }
}

void MainWindow::onLowStock()
{
    QStringList low;
    for (const auto &it : manager.getAllItems()) {
        if (it.cantidad < lowStockThreshold)
            low << QString("%1 (ID %2) - %3").arg(it.nombre).arg(it.id).arg(it.cantidad);
    }

    if (low.isEmpty())
        QMessageBox::information(this, "Stock", "No hay stock bajo.");
    else
        QMessageBox::warning(this, "Stock bajo", low.join("\n"));
}

void MainWindow::onSearch(const QString &text)
{
    proxy->setFilterFixedString(text);
}

void MainWindow::refreshModel()
{
    model->setQuery("SELECT id, nombre, tipo, cantidad, ubicacion, fechaAdquisicion "
                    "FROM inventario ORDER BY id DESC");

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nombre");
    model->setHeaderData(2, Qt::Horizontal, "Tipo");
    model->setHeaderData(3, Qt::Horizontal, "Cantidad");
    model->setHeaderData(4, Qt::Horizontal, "Ubicación");
    model->setHeaderData(5, Qt::Horizontal, "Fecha adquisición");

    tableView->resizeColumnsToContents();
}

void MainWindow::checkLowStockOnStart()
{
    QStringList low;
    for (const auto &it : manager.getAllItems()) {
        if (it.cantidad < lowStockThreshold)
            low << QString("%1 (ID %2) - %3").arg(it.nombre).arg(it.id).arg(it.cantidad);
    }

    if (!low.isEmpty())
        QMessageBox::warning(this, "Stock bajo", low.join("\n"));
}
