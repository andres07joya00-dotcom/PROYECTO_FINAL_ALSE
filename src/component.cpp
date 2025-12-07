#include "component.h"

Component::Component()
    : m_id(0),
    m_quantity(0)
{
}

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

int Component::getId() const { return m_id; }
QString Component::getName() const { return m_name; }
QString Component::getType() const { return m_type; }
int Component::getQuantity() const { return m_quantity; }
QString Component::getLocation() const { return m_location; }
QString Component::getPurchaseDate() const { return m_purchase_date; }

void Component::setId(int id) { m_id = id; }
void Component::setName(const QString &name) { m_name = name; }
void Component::setType(const QString &type) { m_type = type; }
void Component::setQuantity(int quantity) { m_quantity = quantity; }
void Component::setLocation(const QString &location) { m_location = location; }
void Component::setPurchaseDate(const QString &purchase_date) { m_purchase_date = purchase_date; }
