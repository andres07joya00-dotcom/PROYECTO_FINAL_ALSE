/**
 * @file main.cpp
 * @brief Punto de entrada principal de la aplicación de inventario.
 *
 * Este archivo inicializa el entorno Qt, establece la conexión con la base
 * de datos mediante DatabaseManager y crea la ventana principal de la aplicación.
 */

#include <QApplication>
#include <QMessageBox>
#include "DatabaseManager.h"
#include "mainwindow.h"

/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa QApplication, intenta abrir la base de datos y crea la ventana
 * principal si la conexión es exitosa.
 *
 * @param argc Número de argumentos de línea de comandos.
 * @param argv Arreglo con los argumentos de línea de comandos.
 * @return int Código de salida de la aplicación.
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Obtener la conexión a la base de datos desde DatabaseManager
    QSqlDatabase db = DatabaseManager::getDatabase();

    // Verificar si la base de datos se abrió correctamente
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(nullptr, "Error", "No se pudo abrir la base de datos.");
        return -1;
    }

    // Crear y mostrar la ventana principal
    MainWindow w(db);
    w.show();

    return app.exec();
}

