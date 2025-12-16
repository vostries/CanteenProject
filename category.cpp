#include "category.h"
#include <QJsonObject>
#include <QJsonDocument>

Category::Category(int id, const QString &name)
    : m_id(id), m_name(name)
{
}

QString Category::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["name"] = m_name;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

Category Category::fromJson(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    
    return Category(
        obj["id"].toInt(),
        obj["name"].toString()
    );
}










