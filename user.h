#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>

enum class UserType { Admin, Student };

class User {
public:
  User(int id, const QString &username, const QString &password, UserType type,
       double balance = 0.0);

  int getId() const { return m_id; }
  QString getUsername() const { return m_username; }
  QString getPassword() const { return m_password; }
  UserType getType() const { return m_type; }
  double getBalance() const { return m_balance; }

  void setBalance(double balance) { m_balance = balance; }
  void addBalance(double amount) { m_balance += amount; }
  bool deductBalance(double amount);

  bool verifyPassword(const QString &password) const;
  
  static QString hashPassword(const QString &password);
  static bool isHashed(const QString &password);

  QString toJson() const;
  static User fromJson(const QString &json);

private:
  int m_id;
  QString m_username;
  QString m_password;
  UserType m_type;
  double m_balance;
};

#endif // USER_H
