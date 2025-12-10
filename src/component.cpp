#include "component.h"

/**
 * @brief Constructor por defecto.
 *
 * Inicializa el componente con valores neutrales:
 * - id = 0
 * - cantidad = 0
 * 
 * Los demás campos quedan vacíos.
 */
Component::Component()
    : m_id(0),
      m_quantity(0)
{
}

/**
 * @brief Constructor que inicializa todos los campos del componente.
 *
 * @param id Identificador único del componente.
 * @param name Nombre del componente.
 * @param type Tipo o categoría del componente.
 * @param quantity Cantidad disponible en inventario.
 * @param location Ubicación física del componente.
 * @param purchase_date Fecha de adquisición.
 */
Component::Component(int id,
                     const QString &name,
                     const QString &type,
                     int quantity,
                     const QString &location,
                     const QString &purchase_date)
    : m_id(id),
      m_name(name),
      m_type(type),
      m_quantity(quantity),
      m_location(location),
      m_purchase_date(purchase_date)
{
}

/**
 * @brief Obtiene el ID del componente.
 * @return ID almacenado.
 */
int Component::getId() const { return m_id; }

/**
 * @brief Obtiene el nombre del componente.
 * @return Nombre actual.
 */
QString Component::getName() const { return m_name; }

/**
 * @brief Obtiene el tipo del componente.
 * @return Tipo actual.
 */
QString Component::getType() const { return m_type; }

/**
 * @brief Obtiene la cantidad disponible.
 * @return Cantidad en inventario.
 */
int Component::getQuantity() const { return m_quantity; }

/**
 * @brief Obtiene la ubicación física del componente.
 * @return Ubicación almacenada.
 */
QString Component::getLocation() const { return m_location; }

/**
 * @brief Obtiene la fecha de adquisición.
 * @return Fecha en formato QString.
 */
QString Component::getPurchaseDate() const { return m_purchase_date; }

/**
 * @brief Establece un nuevo ID.
 * @param id Nuevo identificador.
 */
void Component::setId(int id) { m_id = id; }

/**
 * @brief Establece un nuevo nombre.
 * @param name Nombre actualizado.
 */
void Component::setName(const QString &name) { m_name = name; }

/**
 * @brief Establece un nuevo tipo de componente.
 * @param type Tipo actualizado.
 */
void Component::setType(const QString &type) { m_type = type; }

/**
 * @brief Cambia la cantidad disponible.
 * @param quantity Nueva cantidad.
 */
void Component::setQuantity(int quantity) { m_quantity = quantity; }

/**
 * @brief Establece una nueva ubicación.
 * @param location Ubicación actualizada.
 */
void Component::setLocation(const QString &location) { m_location = location; }

/**
 * @brief Establece una nueva fecha de adquisición.
 * @param purchase_date Fecha actualizada.
 */
void Component::setPurchaseDate(const QString &purchase_date) { m_purchase_date = purchase_date; }
