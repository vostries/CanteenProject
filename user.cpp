#include "user.h"
#include <QJsonObject>
#include <QJsonDocument>

User::User(int id, const QString &username, const QString &password, UserType type, double balance)
    : m_id(id), m_username(username), m_password(password), m_type(type), m_balance(balance)
{
}

bool User::deductBalance(double amount)
{
    if (m_balance >= amount) {
        m_balance -= amount;
        return true;
    }
    return false;
}

bool User::verifyPassword(const QString &password) const
{
    return m_password == password;
}

QString User::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["username"] = m_username;
    obj["password"] = m_password;
    obj["type"] = static_cast<int>(m_type);
    obj["balance"] = m_balance;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

User User::fromJson(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    
    return User(
        obj["id"].toInt(),
        obj["username"].toString(),
        obj["password"].toString(),
        static_cast<UserType>(obj["type"].toInt()),
        obj["balance"].toDouble()
    );
}


