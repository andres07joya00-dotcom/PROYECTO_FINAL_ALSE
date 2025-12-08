#include <QApplication>
#include <QMessageBox>
#include "DatabaseManager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSqlDatabase db = DatabaseManager::getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(nullptr, "Error", "No se pudo abrir la base de datos.");
        return -1;
    }

    MainWindow w(db);
    w.show();

    return app.exec();
}

