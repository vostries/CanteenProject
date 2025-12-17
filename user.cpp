#include "user.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QRegularExpression>

User::User(int id, const QString &username, const QString &password, UserType type, double balance)
    : m_id(id), m_username(username), m_type(type), m_balance(balance)
{
    if (isHashed(password)) {
        m_password = password;
    } else {
        m_password = hashPassword(password);
    }
}

bool User::deductBalance(double amount)
{
    if (m_balance >= amount) {
        m_balance -= amount;
        return true;
    }
    return false;
}

QString User::hashPassword(const QString &password)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    return hash.result().toHex();
}

bool User::isHashed(const QString &password)
{
    if (password.length() != 64) {
        return false;
    }
    QRegularExpression hexRegex("^[0-9a-fA-F]{64}$");
    return hexRegex.match(password).hasMatch();
}

bool User::verifyPassword(const QString &password) const
{
    QString hashedPassword = hashPassword(password);
    return m_password == hashedPassword;
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














