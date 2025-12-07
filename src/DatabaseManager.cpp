#include "DatabaseManager.h"
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("mi_base.db");   // Nombre del archivo SQLite
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase()
{
    if (!m_db.open()) {
        qDebug() << "Error abriendo la base de datos:" << m_db.lastError();
        return false;
    }

    qDebug() << "Base de datos abierta correctamente.";
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Base de datos cerrada.";
    }
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;

    QString createUserTable =
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nombre TEXT, "
        "edad INTEGER"
        ");";

    if (!query.exec(createUserTable)) {
        qDebug() << "Error creando tabla usuarios:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::insertUser(const QString &name, int age)
{
    QSqlQuery query;
    query.prepare("INSERT INTO usuarios (nombre, edad) VALUES (:name, :age)");
    query.bindValue(":name", name);
    query.bindValue(":age", age);

    if (!query.exec()) {
        qDebug() << "Error insertando usuario:" << query.lastError();
        return false;
    }

    return true;
}
