#include "order.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

Order::Order(int id, int userId, const QDate &date, const QList<QPair<int, int>> &meals)
    : m_id(id), m_userId(userId), m_date(date), m_meals(meals), m_totalPrice(0.0)
{
}

QString Order::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["userId"] = m_userId;
    obj["date"] = m_date.toString(Qt::ISODate);
    obj["totalPrice"] = m_totalPrice;
    
    QJsonArray mealsArray;
    for (const auto &meal : m_meals) {
        QJsonObject mealObj;
        mealObj["mealId"] = meal.first;
        mealObj["quantity"] = meal.second;
        mealsArray.append(mealObj);
    }
    obj["meals"] = mealsArray;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

Order Order::fromJson(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    
    QDate date = QDate::fromString(obj["date"].toString(), Qt::ISODate);
    
    QList<QPair<int, int>> meals;
    QJsonArray mealsArray = obj["meals"].toArray();
    for (const auto &value : mealsArray) {
        QJsonObject mealObj = value.toObject();
        meals.append(qMakePair(mealObj["mealId"].toInt(), mealObj["quantity"].toInt()));
    }
    
    Order order(
        obj["id"].toInt(),
        obj["userId"].toInt(),
        date,
        meals
    );
    order.setTotalPrice(obj["totalPrice"].toDouble());
    
    return order;
}















