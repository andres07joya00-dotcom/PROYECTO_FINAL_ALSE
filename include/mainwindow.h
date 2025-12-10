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

/*
 * Clase AddDialog
 * ---------------
 * Ventana pequeña para agregar un nuevo ítem al inventario.
 * Contiene campos de entrada para nombre, tipo, cantidad, ubicación y fecha.
 * Emite señales accepted() y cancelled() según la acción del usuario.
 */
class AddDialog : public QWidget {
    Q_OBJECT
public:
    explicit AddDialog(QWidget *parent = nullptr);

    // Métodos para obtener los valores ingresados por el usuario
    QString getName() const;
    QString getType() const;
    int getQuantity() const;
    QString getLocation() const;
    QDate getDate() const;

signals:
    void accepted();   // Se emite cuando el usuario confirma
    void cancelled();  // Se emite cuando el usuario cancela

private slots:
    void onOk();       // Maneja el botón de OK
    void onCancel();   // Maneja el botón de cancelar

private:
    // Widgets de entrada
    QLineEdit *nameEdit;
    QLineEdit *typeEdit;
    QSpinBox  *qtySpin;
    QLineEdit *locEdit;
    QDateEdit *dateEdit;
};


/*
 * Clase MainWindow
 * ----------------
 * Es la ventana principal del sistema de inventario.
 * Se encarga de mostrar los datos, filtrarlos, agregar, editar,
 * eliminar, exportar reportes y manejar el estado del inventario.
 */
class MainWindow : public QWidget {
    Q_OBJECT
public:
    // Recibe la base de datos ya abierta y lista para usarse
    explicit MainWindow(QSqlDatabase db, QWidget *parent = nullptr);

private slots:
    // Acciones asociadas a los botones de la UI
    void onEdit();
    void onAdd();
    void onDelete();
    void onExport();
    void onLowStock();
    void onSearch(const QString &text);  // Filtro de búsqueda en tiempo real

private:
    /*
     * Refresca los datos del modelo leyendo desde InventoryManager.
     * Se usa después de agregar, editar o eliminar ítems.
     */
    void refreshModel();

    /*
     * Revisa al iniciar si hay items con pocas existencias.
     * Si los hay, muestra una alerta al usuario.
     */
    void checkLowStockOnStart();

    /*
     * Carga datos iniciales por defecto para pruebas.
     * (Dependiendo de tu implementación en el .cpp)
     */
    void onLoadDefaults();

    /*
     * Restaura valores iniciales del sistema.
     */
    void onRestoreDefaults();

private:
    InventoryManager manager;       // Administrador de inventario (capa de BD)
    QSqlQueryModel *model;          // Modelo base conectado a SQL
    QSortFilterProxyModel *proxy;   // Modelo para búsqueda y filtrado
    QTableView *tableView;          // Tabla que muestra los ítems
    QLineEdit *searchEdit;          // Barra de búsqueda

    const int lowStockThreshold = 5;  // Cantidad mínima antes de considerarse "bajo stock"
};

#endif // MAINWINDOW_H
