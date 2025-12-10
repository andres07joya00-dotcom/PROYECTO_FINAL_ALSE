#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QObject>
#include <QList>
#include <QSqlDatabase>

/*
 * Estructura que representa un ítem dentro del inventario.
 * Contiene toda la información necesaria para leer o escribir
 * un registro desde la base de datos.
 */
struct InventoryItem {
    int id;                     // Identificador único del ítem (PRIMARY KEY)
    QString nombre;             // Nombre del componente o elemento
    QString tipo;               // Tipo o categoría del ítem
    int cantidad;               // Cantidad disponible en inventario
    QString ubicacion;          // Ubicación física dentro del almacén
    QString fechaAdquisicion;   // Fecha en que se adquirió el ítem
};

/*
 * Clase InventoryManager
 * ----------------------
 * Administra la comunicación entre la aplicación y la base de datos SQLite.
 * Permite crear la tabla, agregar, editar, eliminar y consultar ítems.
 * Se utiliza como capa intermedia entre la lógica del programa y el motor SQL.
 */
class InventoryManager : public QObject
{
    Q_OBJECT

public:
    /*
     * Constructor del administrador de inventario.
     * Recibe una referencia a la base de datos ya inicializada.
     */
    explicit InventoryManager(QSqlDatabase database,
                              QObject *parent = nullptr);

    /*
     * Crea la tabla del inventario si no existe.
     * Retorna true si la operación es exitosa.
     */
    bool createTable();

    /*
     * Inserta un nuevo item en la base de datos.
     * Los parámetros corresponden a cada columna de la tabla.
     */
    bool addItem(const QString &nombre,
                 const QString &tipo,
                 int cantidad,
                 const QString &ubicacion,
                 const QString &fechaAdquisicion);

    /*
     * Actualiza únicamente la cantidad de un ítem según su ID.
     */
    bool updateQuantity(int id, int newQuantity);

    /*
     * Elimina un ítem del inventario por ID.
     */
    bool removeItem(int id);

    /*
     * Obtiene todos los ítems de la tabla en forma de lista.
     */
    QList<InventoryItem> getAllItems();

    /*
     * Actualiza un ítem completo según su ID.
     */
    bool updateItem(int id,
                    const QString &nombre,
                    const QString &tipo,
                    int cantidad,
                    const QString &ubicacion,
                    const QString &fechaAdquisicion);

    /*
     * Devuelve un solo ítem según su ID.
     * Si no existe, retorna un struct vacío.
     */
    InventoryItem getItemById(int id);

private:
    QSqlDatabase db;    // Conexión activa a la base de datos SQLite
};

#endif // INVENTORYMANAGER_H
