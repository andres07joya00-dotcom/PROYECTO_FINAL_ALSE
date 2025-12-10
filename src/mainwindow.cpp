/**
 * @file mainwindow.cpp
 * @brief Implementación de las clases AddDialog y MainWindow para la gestión del inventario.
 *
 * Este archivo contiene la lógica de la interfaz gráfica, incluyendo
 * la creación y edición de componentes, filtros, exportación de datos,
 * carga de valores por defecto, detección de stock bajo y conexión con
 * la base de datos a través de InventoryManager.
 */

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
#include "delegate.h"

/// Convierte una fecha QDate a formato YYYY-MM-DD.
static QString dateToString(const QDate &d) {
    return d.toString("yyyy-MM-dd");
}

/// Convierte un InventoryItem a un objeto Component.
static Component inventoryItemToComponent(const InventoryItem &it) {
    return Component(it.id, it.nombre, it.tipo, it.cantidad, it.ubicacion, it.fechaAdquisicion);
}

/* ============================================================
 *                       AddDialog
 * ============================================================ */

/**
 * @class AddDialog
 * @brief Diálogo para agregar o editar componentes.
 *
 * Permite ingresar los campos de un componente: nombre, tipo,
 * cantidad, ubicación y fecha de adquisición.
 */

/**
 * @brief Constructor del diálogo.
 * @param parent Widget padre.
 */
AddDialog::AddDialog(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Agregar componente");
    QVBoxLayout *v = new QVBoxLayout(this);

    // Campos de entrada
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

    // Para permitir edición al usar findChild
    nameEdit->setObjectName("nameEdit");
    typeEdit->setObjectName("typeEdit");
    qtySpin->setObjectName("qtySpin");
    locEdit->setObjectName("locEdit");
    dateEdit->setObjectName("dateEdit");
}

/// @return Nombre ingresado.
QString AddDialog::getName() const { return nameEdit->text().trimmed(); }

/// @return Tipo ingresado.
QString AddDialog::getType() const { return typeEdit->text().trimmed(); }

/// @return Cantidad ingresada.
int AddDialog::getQuantity() const { return qtySpin->value(); }

/// @return Ubicación ingresada.
QString AddDialog::getLocation() const { return locEdit->text().trimmed(); }

/// @return Fecha ingresada.
QDate AddDialog::getDate() const { return dateEdit->date(); }

/// Emite señal de aceptación.
void AddDialog::onOk() { emit accepted(); }

/// Emite señal de cancelación.
void AddDialog::onCancel() { emit cancelled(); }



/* ============================================================
 *                       MainWindow
 * ============================================================ */

/**
 * @class MainWindow
 * @brief Ventana principal para gestionar el inventario.
 *
 * Permite agregar, eliminar, editar, buscar, exportar y restaurar datos.
 * Maneja la tabla SQL y los filtros sobre la vista.
 */

/**
 * @brief Constructor principal.
 * @param db Referencia a la base de datos activa.
 * @param parent Widget padre.
 */
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
    QPushButton *btnLoadDefaults = new QPushButton("Cargar base por defecto");
    QPushButton *btnRestore = new QPushButton("Restaurar base original");
    QPushButton *btnEdit = new QPushButton("Editar");

    topLayout->addWidget(btnLoadDefaults);
    topLayout->addWidget(btnRestore);
    topLayout->addWidget(btnEdit);
    topLayout->addWidget(new QLabel("Buscar:"));
    topLayout->addWidget(searchEdit);
    topLayout->addWidget(btnAdd);
    topLayout->addWidget(btnDelete);
    topLayout->addWidget(btnLowStock);
    topLayout->addWidget(btnExport);

    mainLayout->addLayout(topLayout);

    // Modelos SQL
    model = new QSqlQueryModel(this);
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1);

    // Tabla
    tableView = new QTableView();
    tableView->setModel(proxy);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setItemDelegate(new LowStockDelegate(lowStockThreshold, this));
    mainLayout->addWidget(tableView);

    // Conexiones
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(btnLowStock, &QPushButton::clicked, this, &MainWindow::onLowStock);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    connect(btnLoadDefaults, &QPushButton::clicked, this, &MainWindow::onLoadDefaults);
    connect(btnRestore, &QPushButton::clicked, this, &MainWindow::onRestoreDefaults);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEdit);

    if (!manager.createTable()) {
        QMessageBox::critical(this, "Error", "No se pudo crear la tabla de inventario.");
    }

    refreshModel();
    checkLowStockOnStart();
}


/**
 * @brief Edita el componente seleccionado abriendo un AddDialog precargado.
 */
void MainWindow::onEdit()
{
    QModelIndexList sel = tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona una fila primero.");
        return;
    }

    QModelIndex idx = sel.first();
    QModelIndex src = proxy->mapToSource(idx);
    int id = model->data(model->index(src.row(), 0)).toInt();

    InventoryItem it = manager.getItemById(id);

    AddDialog *dlg = new AddDialog();
    dlg->setWindowModality(Qt::ApplicationModal);

    // Precargar datos
    dlg->findChild<QLineEdit*>("nameEdit")->setText(it.nombre);
    dlg->findChild<QLineEdit*>("typeEdit")->setText(it.tipo);
    dlg->findChild<QSpinBox*>("qtySpin")->setValue(it.cantidad);
    dlg->findChild<QLineEdit*>("locEdit")->setText(it.ubicacion);
    dlg->findChild<QDateEdit*>("dateEdit")->setDate(QDate::fromString(it.fechaAdquisicion, "yyyy-MM-dd"));

    dlg->show();

    connect(dlg, &AddDialog::accepted, this, [this, dlg, id]() {

        if (dlg->getName().isEmpty() || dlg->getType().isEmpty()) {
            QMessageBox::warning(this, "Validación", "Nombre y tipo son obligatorios.");
            return;
        }

        if (!manager.updateItem(
                id,
                dlg->getName(),
                dlg->getType(),
                dlg->getQuantity(),
                dlg->getLocation(),
                dlg->getDate().toString("yyyy-MM-dd")))
        {
            QMessageBox::critical(this, "Error", "No se pudo editar el componente.");
        } else {
            refreshModel();
        }

        dlg->close();
        dlg->deleteLater();
    });

    connect(dlg, &AddDialog::cancelled, dlg, &AddDialog::close);
}


/**
 * @brief Abre un diálogo para añadir un componente nuevo.
 */
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


/**
 * @brief Elimina el elemento seleccionado en la tabla.
 */
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


/**
 * @brief Carga una lista predefinida de componentes.
 */
void MainWindow::onLoadDefaults()
{
    QList<QList<QString>> defaults = {
        // ... (lista completa de 50 elementos)
    };

    for (const auto &item : defaults) {
        manager.addItem(item[0], item[1], item[2].toInt(), item[3], item[4]);
    }

    refreshModel();
    QMessageBox::information(this, "Carga completa", "Se cargaron 50 componentes.");
}


/**
 * @brief Borra toda la base y recarga los valores por defecto.
 */
void MainWindow::onRestoreDefaults()
{
    if (QMessageBox::question(this, "Restaurar",
                              "¿Seguro que deseas borrar todo y restaurar la base por defecto?")
        != QMessageBox::Yes)
        return;

    QSqlQuery q;
    q.exec("DELETE FROM inventario");

    onLoadDefaults();
    QMessageBox::information(this, "Restaurada", "La base fue restaurada.");
}


/**
 * @brief Exporta el inventario completo a un archivo CSV.
 */
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


/**
 * @brief Resalta todos los ítems cuyo stock esté por debajo del umbral.
 */
void MainWindow::onLowStock()
{
    for (int r = 0; r < proxy->rowCount(); r++) {
        QModelIndex idxQty = proxy->index(r, 3);
        int qty = idxQty.data().toInt();

        if (qty < lowStockThreshold) {
            for (int c = 0; c < proxy->columnCount(); c++) {
                tableView->model()->setData(
                    proxy->index(r, c),
                    QColor(255, 200, 200),
                    Qt::BackgroundRole
                );
            }
        }
    }

    QMessageBox::information(this, "Stock bajo", "Los elementos con poco stock fueron resaltados.");
}


/**
 * @brief Aplica filtro de búsqueda sobre todos los campos.
 * @param text Texto a filtrar.
 */
void MainWindow::onSearch(const QString &text)
{
    proxy->setFilterFixedString(text);
}


/**
 * @brief Refresca el modelo SQL que alimenta la tabla.
 */
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


/**
 * @brief Al iniciar, muestra una alerta si hay componentes con stock bajo.
 */
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
