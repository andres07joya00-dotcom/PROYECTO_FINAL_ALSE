#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

void iniciarBaseDatos() {
    // Conectar al driver SQLite
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    // Nombre del archivo (se crea si no existe)
    db.setDatabaseName("mi_base.db");

    if (!db.open()) {
        qDebug() << "ERROR al abrir DB:" << db.lastError().text();
        return;
    }

    qDebug() << "Base de datos abierta correctamente";

    // Crear una tabla de ejemplo
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS usuarios ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
           "nombre TEXT)");

    qDebug() << "Tabla creada o ya existía";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    iniciarBaseDatos();

    return 0;   // No iniciamos interfaz gráfica por ahora
}
