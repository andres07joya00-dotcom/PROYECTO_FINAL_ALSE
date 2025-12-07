#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>

class Component
{
public:
    Component();
    Component(int id,
              const QString &name,
              const QString &type,
              int quantity,
              const QString &location,
              const QString &purchase_date);

    int getId() const;
    QString getName() const;
    QString getType() const;
    int getQuantity() const;
    QString getLocation() const;
    QString getPurchaseDate() const;

    void setId(int id);
    void setName(const QString &name);
    void setType(const QString &type);
    void setQuantity(int quantity);
    void setLocation(const QString &location);
    void setPurchaseDate(const QString &purchase_date);

private:
    int m_id;
    QString m_name;
    QString m_type;
    int m_quantity;
    QString m_location;
    QString m_purchase_date;
};

#endif // COMPONENT_H
