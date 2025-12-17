#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QDate>
#include <QList>
#include <QPair>
#include <QtCore>

class Order
{
public:
    Order(int id, int userId, const QDate &date, const QList<QPair<int, int>> &meals); // mealId, quantity
    
    int getId() const { return m_id; }
    int getUserId() const { return m_userId; }
    QDate getDate() const { return m_date; }
    QList<QPair<int, int>> getMeals() const { return m_meals; }
    double getTotalPrice() const { return m_totalPrice; }
    
    void setTotalPrice(double price) { m_totalPrice = price; }
    
    QString toJson() const;
    static Order fromJson(const QString &json);
    
private:
    int m_id;
    int m_userId;
    QDate m_date;
    QList<QPair<int, int>> m_meals;
    double m_totalPrice;
};

#endif // ORDER_H

