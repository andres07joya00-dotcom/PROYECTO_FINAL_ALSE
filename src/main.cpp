#include <QCoreApplication>
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DatabaseManager db;

    if (db.openDatabase()) {

        db.createTables();
        db.insertUser("Andrés", 25);

    }

    return a.exec();
}
