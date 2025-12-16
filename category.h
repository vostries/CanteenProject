#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>

class Category
{
public:
    Category(int id, const QString &name);
    
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    
    QString toJson() const;
    static Category fromJson(const QString &json);
    
private:
    int m_id;
    QString m_name;
};

#endif // CATEGORY_H










