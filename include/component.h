#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>

/**
 * @class Component
 * @brief Representa un componente almacenado en el inventario.
 *
 * Esta clase modela un componente físico dentro del sistema de inventario.
 * Incluye información básica como su identificador, nombre, tipo, cantidad,
 * ubicación y fecha de compra. Provee métodos para obtener y modificar cada
 * uno de estos atributos.
 */
class Component
{
public:
    /**
     * @brief Constructor por defecto.
     *
     * Inicializa todos los campos a valores base (0 o vacío).
     */
    Component();

    /**
     * @brief Constructor con parámetros.
     * @param id Identificador único del componente.
     * @param name Nombre del componente.
     * @param type Tipo o categoría del componente.
     * @param quantity Cantidad disponible en inventario.
     * @param location Ubicación física dentro del almacén.
     * @param purchase_date Fecha de compra del componente.
     */
    Component(int id,
              const QString &name,
              const QString &type,
              int quantity,
              const QString &location,
              const QString &purchase_date);

    /** @brief Obtiene el ID del componente. */
    int getId() const;

    /** @brief Obtiene el nombre del componente. */
    QString getName() const;

    /** @brief Obtiene el tipo del componente. */
    QString getType() const;

    /** @brief Obtiene la cantidad disponible del componente. */
    int getQuantity() const;

    /** @brief Obtiene la ubicación del componente en el almacén. */
    QString getLocation() const;

    /** @brief Obtiene la fecha de compra del componente. */
    QString getPurchaseDate() const;

    /** @brief Establece el ID del componente. */
    void setId(int id);

    /** @brief Establece el nombre del componente. */
    void setName(const QString &name);

    /** @brief Establece el tipo del componente. */
    void setType(const QString &type);

    /** @brief Establece la cantidad del componente. */
    void setQuantity(int quantity);

    /** @brief Establece la ubicación del componente. */
    void setLocation(const QString &location);

    /** @brief Establece la fecha de compra del componente. */
    void setPurchaseDate(const QString &purchase_date);

private:
    int m_id;                ///< Identificador único del componente.
    QString m_name;          ///< Nombre del componente.
    QString m_type;          ///< Tipo o categoría del componente.
    int m_quantity;          ///< Cantidad disponible en inventario.
    QString m_location;      ///< Ubicación física del componente.
    QString m_purchase_date; ///< Fecha de compra del componente.
};

#endif // COMPONENT_H