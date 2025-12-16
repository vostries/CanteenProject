#ifndef MEAL_H
#define MEAL_H

#include <QString>

class Meal
{
public:
    Meal(int id, const QString &name, double price, int categoryId);
    
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    double getPrice() const { return m_price; }
    int getCategoryId() const { return m_categoryId; }
    
    void setName(const QString &name) { m_name = name; }
    void setPrice(double price) { m_price = price; }
    void setCategoryId(int categoryId) { m_categoryId = categoryId; }
    
    QString toJson() const;
    static Meal fromJson(const QString &json);
    
private:
    int m_id;
    QString m_name;
    double m_price;
    int m_categoryId;
};

#endif // MEAL_H


