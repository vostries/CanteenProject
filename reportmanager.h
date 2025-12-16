#ifndef REPORTMANAGER_H
#define REPORTMANAGER_H

#include "reportstrategy.h"
#include <QString>
#include <memory>

class ReportManager
{
public:
    ReportManager();
    ~ReportManager();
    
    void setStrategy(ReportStrategy *strategy);
    QString generateReport(const QList<Order> &orders,
                          const QList<Meal> &meals,
                          const QList<User> &users);
    
private:
    ReportStrategy *m_strategy;
};

#endif // REPORTMANAGER_H










