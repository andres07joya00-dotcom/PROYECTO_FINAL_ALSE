#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>

/**
 * @brief Inicialización del objeto estático de base de datos.
 *
 * Se inicializa como una instancia vacía. La configuración real
 * (tipo de driver y nombre de archivo) se realiza dentro de getDatabase().
 */
QSqlDatabase DatabaseManager::db = QSqlDatabase();

/**
 * @brief Obtiene y gestiona la conexión a la base de datos SQLite.
 *
 * Este método garantiza que la base de datos:
 * - Sea creada solo una vez (patrón singleton estático).
 * - Se configure con el driver "QSQLITE".
 * - Utilice como archivo local "inventario.db".
 * - Abra la conexión si aún no lo está.
 *
 * Si ocurre un error al abrir la base de datos,
 * el mensaje es mostrado mediante qDebug().
 *
 * @return Objeto QSqlDatabase listo para ser utilizado.
 */
QSqlDatabase DatabaseManager::getDatabase()
{
    // Si la instancia aún no es válida, configurar el driver
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("inventario.db");
    }

    // Abrir la base de datos si aún no está abierta
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "ERROR al abrir la base de datos:" << db.lastError();
        } else {
            qDebug() << "Base de datos abierta correctamente.";
        }
    }

    return db;
}
