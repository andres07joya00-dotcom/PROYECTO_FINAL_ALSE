#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase();
    void closeDatabase();

    bool createTables();

    // Ejemplo: insertar datos
    bool insertUser(const QString &name, int age);

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
