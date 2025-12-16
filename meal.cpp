#include "meal.h"
#include <QJsonObject>
#include <QJsonDocument>

Meal::Meal(int id, const QString &name, double price, int categoryId, const QString &imagePath)
    : m_id(id), m_name(name), m_price(price), m_categoryId(categoryId), m_imagePath(imagePath)
{
}

QString Meal::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["name"] = m_name;
    obj["price"] = m_price;
    obj["categoryId"] = m_categoryId;
    obj["imagePath"] = m_imagePath;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

Meal Meal::fromJson(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    
    return Meal(
        obj["id"].toInt(),
        obj["name"].toString(),
        obj["price"].toDouble(),
        obj["categoryId"].toInt(),
        obj.contains("imagePath") ? obj["imagePath"].toString() : QString()
    );
}





