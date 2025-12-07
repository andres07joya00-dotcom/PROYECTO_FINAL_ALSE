#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>

QSqlDatabase DatabaseManager::db = QSqlDatabase();

QSqlDatabase DatabaseManager::getDatabase()
{
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("inventario.db");
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "ERROR al abrir la base de datos:" << db.lastError();
        } else {
            qDebug() << "Base de datos abierta correctamente.";
        }
    }

    return db;
}
