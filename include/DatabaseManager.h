#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

/**
 * @class DatabaseManager
 * @brief Clase encargada de gestionar la conexión con la base de datos.
 *
 * Esta clase implementa un patrón similar a Singleton para proporcionar
 * una única instancia de conexión a la base de datos SQLite utilizada
 * por la aplicación. El método estático @ref getDatabase retorna una
 * referencia a dicha conexión evitando crear múltiples instancias.
 *
 * Se utiliza QSqlDatabase para manejar la apertura y configuración de
 * la base de datos según lo requiera Qt.
 */
class DatabaseManager
{
public:
    /**
     * @brief Obtiene la base de datos principal del sistema.
     *
     * Si la conexión aún no se ha inicializado, se configura y se abre.
     * En llamadas posteriores, devuelve la misma instancia ya configurada.
     *
     * @return Instancia global de QSqlDatabase utilizada por el sistema.
     */
    static QSqlDatabase getDatabase();

private:
    /**
     * @brief Instancia estática de la base de datos administrada.
     *
     * Esta variable almacena la conexión única utilizada por toda la
     * aplicación. Es configurada en @ref getDatabase.
     */
    static QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
