/**
 * @file mainwindow.cpp
 * @brief Implementación de la lógica de interfaz gráfica para la gestión de inventario.
 * @author [Tu Nombre o Equipo de Desarrollo]
 * @date 2024
 *
 * Este archivo contiene la implementación de las clases:
 * - @ref AddDialog: Ventana modal para formularios de entrada.
 * - @ref MainWindow: Ventana principal que orquesta la vista, el modelo SQL y la lógica de negocio.
 *
 * Se utilizan componentes de Qt como QSqlTableModel, QSortFilterProxyModel y delegates
 * personalizados para manejar la presentación de datos.
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

// ============================================================================
// FUNCIONES AUXILIARES ESTÁTICAS
// ============================================================================

/**
 * @brief Convierte un objeto QDate a una cadena con formato estándar de base de datos.
 * @param d Fecha a convertir.
 * @return QString con formato "YYYY-MM-DD".
 */
static QString dateToString(const QDate &d) {
    return d.toString("yyyy-MM-dd");
}

/**
 * @brief Adaptador que convierte una estructura de base de datos (InventoryItem) a un objeto de lógica (Component).
 * @param it Objeto InventoryItem recuperado del InventoryManager.
 * @return Objeto Component listo para ser procesado por la lógica de negocio.
 */
static Component inventoryItemToComponent(const InventoryItem &it) {
    return Component(it.id, it.nombre, it.tipo, it.cantidad, it.ubicacion, it.fechaAdquisicion);
}


// ============================================================================
// CLASE ADDDIALOG
// ============================================================================

/**
 * @class AddDialog
 * @brief Diálogo modal para la creación y edición de componentes.
 *
 * Esta clase hereda de QWidget (usado como ventana) y proporciona un formulario
 * con validaciones básicas para ingresar Nombre, Tipo, Cantidad, Ubicación y Fecha.
 */

/**
 * @brief Constructor de la clase AddDialog.
 *
 * Configura el layout vertical, inicializa los widgets de entrada (QLineEdit, QSpinBox, etc.)
 * y conecta los botones de Aceptar/Cancelar a sus respectivos slots.
 *
 * @param parent Widget padre (opcional), por defecto es nullptr.
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

    // Construcción del formulario
    v->addWidget(new QLabel("Nombre:"));    v->addWidget(nameEdit);
    v->addWidget(new QLabel("Tipo:"));      v->addWidget(typeEdit);
    v->addWidget(new QLabel("Cantidad:"));  v->addWidget(qtySpin);
    v->addWidget(new QLabel("Ubicación:")); v->addWidget(locEdit);
    v->addWidget(new QLabel("Fecha adquisición:")); v->addWidget(dateEdit);

    // Botonera
    QHBoxLayout *h = new QHBoxLayout();
    h->addWidget(btnOk);
    h->addWidget(btnCancel);
    v->addLayout(h);

    // Conexiones de señales
    connect(btnOk, &QPushButton::clicked, this, &AddDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &AddDialog::onCancel);

    // Asignación de nombres de objeto para búsqueda dinámica (findChild)
    nameEdit->setObjectName("nameEdit");
    typeEdit->setObjectName("typeEdit");
    qtySpin->setObjectName("qtySpin");
    locEdit->setObjectName("locEdit");
    dateEdit->setObjectName("dateEdit");
}

/**
 * @brief Obtiene el nombre ingresado por el usuario.
 * @return QString con el texto limpio de espacios al inicio y final (trimmed).
 */
QString AddDialog::getName() const { return nameEdit->text().trimmed(); }

/**
 * @brief Obtiene el tipo de componente ingresado.
 * @return QString con el tipo (trimmed).
 */
QString AddDialog::getType() const { return typeEdit->text().trimmed(); }

/**
 * @brief Obtiene la cantidad seleccionada.
 * @return Entero con el valor del QSpinBox.
 */
int AddDialog::getQuantity() const { return qtySpin->value(); }

/**
 * @brief Obtiene la ubicación física del componente.
 * @return QString con la ubicación.
 */
QString AddDialog::getLocation() const { return locEdit->text().trimmed(); }

/**
 * @brief Obtiene la fecha de adquisición seleccionada.
 * @return Objeto QDate con la fecha.
 */
QDate AddDialog::getDate() const { return dateEdit->date(); }

/**
 * @brief Slot llamado al presionar "Agregar/Aceptar".
 * @details Emite la señal @a accepted() para notificar a la ventana principal.
 */
void AddDialog::onOk() { emit accepted(); }

/**
 * @brief Slot llamado al presionar "Cancelar".
 * @details Emite la señal @a cancelled() para cerrar el diálogo sin cambios.
 */
void AddDialog::onCancel() { emit cancelled(); }


// ============================================================================
// CLASE MAINWINDOW
// ============================================================================

/**
 * @class MainWindow
 * @brief Controlador principal de la aplicación de gestión de inventario.
 *
 * Esta clase orquesta la interacción entre el usuario y la base de datos.
 * Sus responsabilidades incluyen:
 * - Inicializar la base de datos y la interfaz de usuario.
 * - Gestionar el modelo de datos (QSqlQueryModel) y el filtrado (QSortFilterProxyModel).
 * - Manejar eventos de botones (CRUD, exportación, restauración).
 * - Aplicar delegados personalizados para la visualización (LowStockDelegate).
 */

/**
 * @brief Constructor de la ventana principal.
 *
 * Inicializa la conexión a la base de datos, configura el layout principal,
 * crea los modelos de datos y conecta todas las señales de la interfaz.
 *
 * @param db Referencia a la conexión de base de datos QSqlDatabase ya inicializada.
 * @param parent Widget padre (opcional).
 */
MainWindow::MainWindow(QSqlDatabase db, QWidget *parent)
    : QWidget(parent), manager(db)
{
    setWindowTitle("Gestión de Inventario");
    resize(1000, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();

    // -- Inicialización de Widgets --
    searchEdit = new QLineEdit(); searchEdit->setPlaceholderText("Buscar...");
    QPushButton *btnAdd = new QPushButton("Agregar");
    QPushButton *btnDelete = new QPushButton("Eliminar seleccionado");
    QPushButton *btnExport = new QPushButton("Exportar CSV");
    QPushButton *btnLowStock = new QPushButton("Revisar stock bajo");
    QPushButton *btnLoadDefaults = new QPushButton("Cargar base por defecto");
    QPushButton *btnRestore = new QPushButton("Restaurar base original");
    QPushButton *btnEdit = new QPushButton("Editar");

    // -- Construcción del Layout Superior --
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

    // -- Configuración del Modelo MVC --
    model = new QSqlQueryModel(this);
    
    // Configuración del Proxy para filtrado y ordenamiento
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1); // Filtrar en todas las columnas

    // -- Configuración de la Tabla (Vista) --
    tableView = new QTableView();
    tableView->setModel(proxy);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Edición solo vía diálogo
    
    // Inyección del delegado para resaltar stock bajo
    tableView->setItemDelegate(new LowStockDelegate(lowStockThreshold, this));
    mainLayout->addWidget(tableView);

    // -- Conexiones de Señales y Slots --
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExport);
    connect(btnLowStock, &QPushButton::clicked, this, &MainWindow::onLowStock);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    connect(btnLoadDefaults, &QPushButton::clicked, this, &MainWindow::onLoadDefaults);
    connect(btnRestore, &QPushButton::clicked, this, &MainWindow::onRestoreDefaults);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEdit);

    // Inicialización de la tabla en BD si no existe
    if (!manager.createTable()) {
        QMessageBox::critical(this, "Error Crítico", "No se pudo crear o verificar la tabla de inventario en la base de datos.");
    }

    refreshModel();
}

/**
 * @brief Inicia el proceso de edición del componente seleccionado.
 *
 * Recupera el ID del item seleccionado en la tabla, consulta sus datos actuales
 * desde la base de datos, y abre un @ref AddDialog precargado con dicha información.
 *
 * @note Utiliza una función lambda para manejar la señal `accepted` del diálogo,
 * capturando el ID para realizar el `updateItem`.
 */
void MainWindow::onEdit()
{
    QModelIndexList sel = tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Editar", "Por favor, selecciona una fila primero.");
        return;
    }

    // Mapeo del índice del proxy al índice del modelo fuente para obtener el ID real
    QModelIndex idx = sel.first();
    QModelIndex src = proxy->mapToSource(idx);
    int id = model->data(model->index(src.row(), 0)).toInt();

    InventoryItem it = manager.getItemById(id);

    AddDialog *dlg = new AddDialog();
    dlg->setWindowModality(Qt::ApplicationModal);

    // Precargar datos en los widgets del diálogo usando findChild
    dlg->findChild<QLineEdit*>("nameEdit")->setText(it.nombre);
    dlg->findChild<QLineEdit*>("typeEdit")->setText(it.tipo);
    dlg->findChild<QSpinBox*>("qtySpin")->setValue(it.cantidad);
    dlg->findChild<QLineEdit*>("locEdit")->setText(it.ubicacion);
    dlg->findChild<QDateEdit*>("dateEdit")->setDate(QDate::fromString(it.fechaAdquisicion, "yyyy-MM-dd"));

    dlg->show();

    // Lógica al aceptar el diálogo
    connect(dlg, &AddDialog::accepted, this, [this, dlg, id]() {

        if (dlg->getName().isEmpty() || dlg->getType().isEmpty()) {
            QMessageBox::warning(this, "Validación", "Nombre y tipo son campos obligatorios.");
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
            QMessageBox::critical(this, "Error", "Fallo al actualizar el componente en la base de datos.");
        } else {
            refreshModel();
        }

        dlg->close();
        dlg->deleteLater();
    });

    connect(dlg, &AddDialog::cancelled, dlg, &AddDialog::close);
}

/**
 * @brief Abre el diálogo para agregar un nuevo componente al inventario.
 * @details Crea una instancia de @ref AddDialog y conecta su señal de aceptación
 * para insertar el nuevo registro mediante @ref InventoryManager.
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
            QMessageBox::critical(this, "Error", "No se pudo insertar el componente en la base de datos.");
        } else {
            refreshModel();
        }
        dlg->close();
        dlg->deleteLater();
    });

    connect(dlg, &AddDialog::cancelled, dlg, &AddDialog::close);
}

/**
 * @brief Elimina el registro seleccionado en la tabla.
 * @details Muestra un cuadro de confirmación antes de proceder. Si se confirma,
 * solicita al InventoryManager la eliminación por ID.
 */
void MainWindow::onDelete()
{
    QModelIndexList sel = tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona una fila para eliminar.");
        return;
    }

    QModelIndex idx = sel.first();
    QModelIndex src = proxy->mapToSource(idx);
    int id = model->data(model->index(src.row(), 0)).toInt();

    if (QMessageBox::question(this, "Confirmar Eliminación",
                              QString("¿Estás seguro de eliminar el registro con ID %1?").arg(id)) == QMessageBox::Yes)
    {
        if (!manager.removeItem(id)) {
            QMessageBox::critical(this, "Error", "No se pudo eliminar el registro.");
        } else {
            refreshModel();
        }
    }
}

/**
 * @brief Carga un conjunto de datos de prueba (Seed Data).
 * @details Inserta aproximadamente 50 ítems predefinidos en la base de datos.
 * Útil para pruebas y demostraciones iniciales.
 */
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
    QMessageBox::information(this, "Carga Completa", "Se han cargado exitosamente los componentes por defecto.");
}

/**
 * @brief Restaura la base de datos a su estado original (vacía y luego recargada).
 * @warning Esta acción ejecuta un `DELETE FROM inventario`, borrando todos los datos existentes.
 */
void MainWindow::onRestoreDefaults()
{
    if (QMessageBox::question(this, "Restaurar Fábrica",
                              "¿Seguro que deseas borrar TODO el inventario y restaurar los datos por defecto?\nEsta acción no se puede deshacer.")
        != QMessageBox::Yes)
        return;
    QSqlQuery q;
    q.exec("DELETE FROM inventario");


    onLoadDefaults();
    QMessageBox::information(this, "Restauración", "La base de datos ha sido restaurada.");
}

/**
 * @brief Genera y exporta un reporte del inventario en formato CSV.
 * @details Abre un diálogo de sistema para seleccionar la ruta de guardado y utiliza
 * la clase helper @ref CSVReport para generar el archivo.
 */
void MainWindow::onExport()
{
    QString filename = QFileDialog::getSaveFileName(
        this, "Guardar Reporte CSV", "reporte.csv", "Archivos CSV (*.csv)");

    if (filename.isEmpty()) return;

    CSVReport report;
    // Obtiene todos los items mediante el manager y genera el archivo
    if (report.generate(manager.getAllItems(), filename)) {
        QMessageBox::information(this, "Éxito", "El reporte ha sido exportado correctamente.");
    } else {
        QMessageBox::critical(this, "Error de Exportación", "No se pudo escribir el archivo en la ruta seleccionada.");
    }
}

/**
 * @brief Analiza el stock actual y resalta visualmente los ítems críticos.
 * @details
 * 1. Itera sobre todas las filas del modelo proxy.
 * 2. Compara la cantidad con `lowStockThreshold`.
 * 3. Si es menor, modifica el `Qt::BackgroundRole` de la celda para pintarla de rojo claro.
 * 4. Genera una lista de nombres de productos bajos en stock y la muestra en un MessageBox.
 */
void MainWindow::onLowStock()
{
    QStringList lowStockItems;

    for (int r = 0; r < proxy->rowCount(); r++) {
        // Índice de la columna 'Cantidad' (columna 3)
        QModelIndex idxQty = proxy->index(r, 3);
        int qty = idxQty.data().toInt();

        if (qty < lowStockThreshold) {
            // Pintar toda la fila (iterando columnas)
            for (int c = 0; c < proxy->columnCount(); c++) {
                tableView->model()->setData(
                    proxy->index(r, c),
                    QColor(255, 200, 200), // Rojo claro
                    Qt::BackgroundRole
                );
            }

            // Agregar nombre a la lista de alerta
            QModelIndex idxName = proxy->index(r, 1);
            lowStockItems << idxName.data().toString();
        }
    }

    if (!lowStockItems.isEmpty()) {
        QMessageBox::warning(this, "Alerta de Stock Bajo", 
            "Los siguientes ítems están por debajo del mínimo:\n\n" + lowStockItems.join("\n"));
    } else {
        QMessageBox::information(this, "Stock Saludable", "No hay elementos con stock bajo.");
    }
}

/**
 * @brief Filtra la tabla en tiempo real según el texto ingresado.
 * @param text Cadena de búsqueda. Se busca coincidencia en cualquier columna.
 */
void MainWindow::onSearch(const QString &text)
{
    proxy->setFilterFixedString(text);
}

/**
 * @brief Actualiza la vista recargando los datos desde la base de datos SQL.
 * @details Ejecuta de nuevo la consulta `SELECT` sobre el modelo y reasigna
 * los nombres de las cabeceras para una visualización amigable.
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
    model->setHeaderData(5, Qt::Horizontal, "Fecha Adquisición");

    tableView->resizeColumnsToContents();
}
