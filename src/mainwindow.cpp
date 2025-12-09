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

    nameEdit->setObjectName("nameEdit");
    typeEdit->setObjectName("typeEdit");
    qtySpin->setObjectName("qtySpin");
    locEdit->setObjectName("locEdit");
    dateEdit->setObjectName("dateEdit");

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
    tableView->setItemDelegate(new LowStockDelegate(lowStockThreshold, this));
    mainLayout->addWidget(tableView);

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

void MainWindow::onLoadDefaults()
{
    QList<QList<QString>> defaults = {
        {"Resistor 10kΩ", "Electrónico", "100", "Cajón A1", "2024-01-10"},
        {"Capacitor 100nF", "Electrónico", "80", "Cajón A1", "2024-01-12"},
        {"Diodo 1N4148", "Electrónico", "150", "Cajón A2", "2024-02-01"},
        {"LED Rojo 5mm", "Electrónico", "200", "Cajón A2", "2024-03-05"},
        {"Transistor 2N3904", "Electrónico", "120", "Cajón A3", "2024-03-10"},
        {"Potenciómetro 10k", "Electrónico", "15", "Cajón B1", "2024-02-20"},
        {"Motor DC 6V", "Electrónico", "10", "Estante B2", "2024-03-03"},
        {"Sensor HC-SR04", "Sensor", "12", "Estante C1", "2024-02-28"},
        {"Arduino Uno", "Microcontrolador", "4", "Estante C2", "2023-11-22"},
        {"Protoboard", "Herramienta", "10", "Estante C3", "2023-12-10"},
        {"Raspberry Pi Pico", "Microcontrolador", "6", "Estante C2", "2024-01-30"},
        {"Relé 5V", "Electrónico", "30", "Cajón A3", "2024-01-11"},
        {"Cable Dupont (m/m)", "Accesorio", "300", "Cajón A1", "2024-01-05"},
        {"Cable Dupont (h/h)", "Accesorio", "300", "Cajón A1", "2024-01-05"},
        {"Batería 9V", "Electrónico", "25", "Estante D1", "2024-02-10"},
        {"Multímetro Digital", "Instrumento", "3", "Mesa Taller", "2023-12-10"},
        {"Osciloscopio", "Instrumento", "1", "Mesa Taller", "2023-09-10"},
        {"Fuente DC 30V", "Instrumento", "2", "Mesa Taller", "2023-09-10"},
        {"Sensor PIR SR505", "Sensor", "20", "Estante C1", "2024-01-05"},
        {"Sensor DHT11", "Sensor", "15", "Estante C1", "2024-01-08"},
        {"Sensor DHT22", "Sensor", "10", "Estante C1", "2024-01-08"},
        {"ESP32-CAM", "Microcontrolador", "7", "Estante C2", "2024-02-01"},
        {"ESP8266", "Microcontrolador", "10", "Estante C2", "2024-02-02"},
        {"Cámara Web USB", "Computación", "5", "Estante D2", "2023-11-11"},
        {"Router TP-Link", "Red", "3", "Estante D2", "2023-12-01"},
        {"Switch Lógico", "Electrónico", "100", "Cajón A4", "2024-01-22"},
        {"Cables USB", "Accesorio", "30", "Cajón B3", "2023-12-28"},
        {"Rollo Cinta Aislante", "Herramienta", "20", "Cajón B3", "2023-12-30"},
        {"Soldador 60W", "Herramienta", "2", "Mesa Taller", "2023-12-15"},
        {"Estaño", "Consumible", "10", "Cajón B1", "2023-12-15"},
        {"Alcohol Isopropílico", "Limpieza", "4", "Estante E1", "2024-01-02"},
        {"Guantes Nitrilo", "Laboratorio", "200", "Armario F1", "2024-01-01"},
        {"Termómetro Digital", "Laboratorio", "3", "Estante E2", "2023-12-10"},
        {"Placa de Calentamiento", "Laboratorio", "1", "Estante E3", "2023-11-20"},
        {"Cronómetro", "Laboratorio", "2", "Cajón B4", "2023-12-09"},
        {"Lupa de Banco", "Herramienta", "2", "Mesa Taller", "2023-10-15"},
        {"Extensión Eléctrica", "Hogar", "8", "Estante D1", "2023-10-10"},
        {"Bombillo LED 12W", "Hogar", "20", "Estante D1", "2023-09-10"},
        {"Tomacorriente", "Hogar", "40", "Estante D1", "2023-09-10"},
        {"Interruptor de Pared", "Hogar", "40", "Estante D1", "2023-09-10"},
        {"Control Remoto IR", "Electrónico", "15", "Cajón A2", "2024-02-01"},
        {"Buzzer 5V", "Electrónico", "40", "Cajón A3", "2024-02-03"},
        {"Servo SG90", "Electrónico", "15", "Cajón A3", "2024-02-10"},
        {"Switch de Palanca", "Electrónico", "50", "Cajón A4", "2024-02-11"},
        {"Jack DC 5.5mm", "Electrónico", "60", "Cajón A4", "2024-02-11"},
        {"Pinzas de Cocodrilo", "Accesorio", "50", "Cajón B3", "2024-01-01"},
        {"Termistor NTC 10k", "Sensor", "80", "Cajón A2", "2024-01-20"}
    };

    for (const auto &item : defaults) {
        manager.addItem(item[0], item[1], item[2].toInt(), item[3], item[4]);
    }

    refreshModel();
    QMessageBox::information(this, "Carga completa", "Se cargaron 50 componentes.");
}

void MainWindow::onRestoreDefaults()
{
    if (QMessageBox::question(this, "Restaurar",
                              "¿Seguro que deseas borrar todo y restaurar la base por defecto?")
        != QMessageBox::Yes)
        return;

    QSqlQuery q;
    q.exec("DELETE FROM inventario");  // borrar todo

    onLoadDefaults();  // volver a cargar

    QMessageBox::information(this, "Restaurada", "La base fue restaurada.");
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
