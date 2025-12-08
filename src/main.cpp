#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QDebug>

#include "DatabaseManager.h"
#include "InventoryManager.h"
#include "report.h"
#include "component.h"

// ---------- Helpers ----------
static QString dateToString(const QDate &d) {
    return d.toString("yyyy-MM-dd");
}

static Component inventoryItemToComponent(const InventoryItem &it) {
    return Component(it.id, it.nombre, it.tipo, it.cantidad, it.ubicacion, it.fechaAdquisicion);
}

// ---------- Dialog simple para agregar (widget inline en este main) ----------
class AddDialog : public QWidget {
    Q_OBJECT
public:
    AddDialog(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Agregar componente");
        QVBoxLayout *v = new QVBoxLayout(this);

        nameEdit = new QLineEdit(); nameEdit->setPlaceholderText("Nombre");
        typeEdit = new QLineEdit(); typeEdit->setPlaceholderText("Tipo");
        qtySpin = new QSpinBox(); qtySpin->setRange(0, 1000000); qtySpin->setValue(1);
        locEdit = new QLineEdit(); locEdit->setPlaceholderText("Ubicación");
        dateEdit = new QDateEdit(QDate::currentDate()); dateEdit->setCalendarPopup(true);

        QPushButton *btnOk = new QPushButton("Agregar");
        QPushButton *btnCancel = new QPushButton("Cancelar");

        v->addWidget(new QLabel("Nombre:")); v->addWidget(nameEdit);
        v->addWidget(new QLabel("Tipo:")); v->addWidget(typeEdit);
        v->addWidget(new QLabel("Cantidad:")); v->addWidget(qtySpin);
        v->addWidget(new QLabel("Ubicación:")); v->addWidget(locEdit);
        v->addWidget(new QLabel("Fecha de adquisición:")); v->addWidget(dateEdit);

        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(btnOk); h->addWidget(btnCancel);
        v->addLayout(h);

        connect(btnOk, &QPushButton::clicked, this, &AddDialog::onOk);
        connect(btnCancel, &QPushButton::clicked, this, &AddDialog::onCancel);
    }

    QString getName() const { return nameEdit->text().trimmed(); }
    QString getType() const { return typeEdit->text().trimmed(); }
    int getQuantity() const { return qtySpin->value(); }
    QString getLocation() const { return locEdit->text().trimmed(); }
    QDate getDate() const { return dateEdit->date(); }

signals:
    void accepted();
    void cancelled();

private slots:
    void onOk() { emit accepted(); }
    void onCancel() { emit cancelled(); }

private:
    QLineEdit *nameEdit;
    QLineEdit *typeEdit;
    QSpinBox *qtySpin;
    QLineEdit *locEdit;
    QDateEdit *dateEdit;
};

// ---------- Main Window ----------
class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QSqlDatabase db, QWidget *parent = nullptr)
        : QWidget(parent),
          manager(db)
    {
        setWindowTitle("Gestión de Inventario");
        resize(1000, 600);

        // Layout principal
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Barra superior: búsquedas y botones
        QHBoxLayout *topLayout = new QHBoxLayout();
        searchEdit = new QLineEdit(); searchEdit->setPlaceholderText("Buscar por nombre, tipo, ubicación...");
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

        // Tabla
        model = new QSqlQueryModel(this);
        proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(model);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(-1); // buscar en todas las columnas

        tableView = new QTableView();
        tableView->setModel(proxy);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mainLayout->addWidget(tableView);

        // Conexiones
        connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
        connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
        connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExport);
        connect(btnLowStock, &QPushButton::clicked, this, &MainWindow::onLowStock);
        connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);

        // Inicializar tabla/DB
        if (!manager.createTable()) {
            QMessageBox::critical(this, "Error", "No se pudo crear/abrir la tabla de inventario.");
        }

        refreshModel();
        checkLowStockOnStart();
    }

private slots:
    void onAdd() {
        AddDialog dlg;
        // Abrimos como ventana modal no bloqueante (simple)
        dlg.setWindowModality(Qt::ApplicationModal);
        dlg.show();

        // Cuando el diálogo emite accepted, guardamos
        connect(&dlg, &AddDialog::accepted, this, [&dlg, this]() {
            // Validaciones básicas
            if (dlg.getName().isEmpty()) {
                QMessageBox::warning(this, "Validación", "El nombre no puede estar vacío.");
                return;
            }
            if (dlg.getType().isEmpty()) {
                QMessageBox::warning(this, "Validación", "El tipo no puede estar vacío.");
                return;
            }

            // Crear Component y pasar a InventoryManager
            Component c(0,
                        dlg.getName(),
                        dlg.getType(),
                        dlg.getQuantity(),
                        dlg.getLocation(),
                        dateToString(dlg.getDate()));

            bool ok = manager.addItem(c.getName(), c.getType(), c.getQuantity(), c.getLocation(), c.getPurchaseDate());
            if (!ok) {
                QMessageBox::critical(this, "Error", "No se pudo agregar el componente a la base de datos.");
            } else {
                refreshModel();
                if (c.getQuantity() < lowStockThreshold) {
                    QMessageBox::warning(this, "Stock bajo", QString("El componente '%1' tiene stock bajo (%2).")
                                         .arg(c.getName()).arg(c.getQuantity()));
                }
            }
            dlg.close();
        });

        connect(&dlg, &AddDialog::cancelled, &dlg, &AddDialog::close);
    }

    void onDelete() {
        QModelIndexList sel = tableView->selectionModel()->selectedRows();
        if (sel.isEmpty()) {
            QMessageBox::information(this, "Eliminar", "Selecciona una fila para eliminar.");
            return;
        }
        // Obtener id desde el modelo proxy -> map to source
        QModelIndex idx = sel.first();
        QModelIndex src = proxy->mapToSource(idx);
        int id = model->data(model->index(src.row(), 0)).toInt(); // columna 0 = id

        if (QMessageBox::question(this, "Confirmar", QString("Eliminar el registro ID %1 ?").arg(id))
            == QMessageBox::Yes) {
            if (!manager.removeItem(id)) {
                QMessageBox::critical(this, "Error", "No se pudo eliminar el item.");
            } else {
                refreshModel();
            }
        }
    }

    void onExport() {
        QString filename = QFileDialog::getSaveFileName(this, "Guardar CSV", "reporte_inventario.csv", "CSV files (*.csv)");
        if (filename.isEmpty()) return;

        QList<InventoryItem> items = manager.getAllItems();
        CSVReport report;
        if (report.generate(items, filename)) {
            QMessageBox::information(this, "Exportar CSV", "Reporte generado correctamente.");
        } else {
            QMessageBox::critical(this, "Exportar CSV", "Error generando el reporte.");
        }
    }

    void onLowStock() {
        QList<InventoryItem> items = manager.getAllItems();
        QStringList low;
        for (const InventoryItem &it : items) {
            if (it.cantidad < lowStockThreshold) {
                low << QString("%1 (ID %2) - Cantidad: %3").arg(it.nombre).arg(it.id).arg(it.cantidad);
            }
        }
        if (low.isEmpty()) {
            QMessageBox::information(this, "Stock bajo", "No hay componentes con stock bajo.");
        } else {
            QMessageBox::warning(this, "Stock bajo", low.join("\n"));
        }
    }

    void onSearch(const QString &text) {
        // Filtrado simple: aplicamos texto fijo (match exact substring)
        proxy->setFilterFixedString(text);
    }

private:
    void refreshModel() {
        model->setQuery("SELECT id, nombre, tipo, cantidad, ubicacion, fechaAdquisicion FROM inventario ORDER BY id DESC");
        // Encabezados amigables
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Nombre");
        model->setHeaderData(2, Qt::Horizontal, "Tipo");
        model->setHeaderData(3, Qt::Horizontal, "Cantidad");
        model->setHeaderData(4, Qt::Horizontal, "Ubicación");
        model->setHeaderData(5, Qt::Horizontal, "Fecha adquisición");

        // Ajustar columnas
        tableView->resizeColumnsToContents();
    }

    void checkLowStockOnStart() {
        // Mostrar alerta resumida al iniciar si hay artículos bajos
        QList<InventoryItem> items = manager.getAllItems();
        QStringList low;
        for (const InventoryItem &it : items) {
            if (it.cantidad < lowStockThreshold) {
                low << QString("%1 (ID %2) - Cantidad: %3").arg(it.nombre).arg(it.id).arg(it.cantidad);
            }
        }
        if (!low.isEmpty()) {
            QMessageBox::warning(this, "Stock bajo al iniciar", low.join("\n"));
        }
    }

private:
    InventoryManager manager;
    QSqlQueryModel *model;
    QSortFilterProxyModel *proxy;
    QTableView *tableView;
    QLineEdit *searchEdit;

    const int lowStockThreshold = 5;
};

// ---------- main ----------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Inicializar DB (DatabaseManager asegura conexión única)
    QSqlDatabase db = DatabaseManager::getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(nullptr, "Error DB", "No se pudo abrir la base de datos.");
        return -1;
    }

    MainWindow w(db);
    w.show();

    return app.exec();
}

#include "main.moc"

