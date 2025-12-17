#ifndef REPORTSTRATEGY_H
#define REPORTSTRATEGY_H

#include <QString>
#include <QMap>
#include <QList>
#include "order.h"
#include "meal.h"

class User;

class ReportStrategy
{
public:
    virtual ~ReportStrategy() = default;
    virtual QString generateReport(const QList<Order> &orders, 
                                   const QList<Meal> &meals,
                                   const QList<User> &users) = 0;
};

class RevenueReportStrategy : public ReportStrategy
{
public:
    QString generateReport(const QList<Order> &orders,
                          const QList<Meal> &meals,
                          const QList<User> &users) override;
};

class PopularDishesReportStrategy : public ReportStrategy
{
public:
    QString generateReport(const QList<Order> &orders,
                          const QList<Meal> &meals,
                          const QList<User> &users) override;
};

class OrdersByDateReportStrategy : public ReportStrategy
{
public:
    QString generateReport(const QList<Order> &orders,
                          const QList<Meal> &meals,
                          const QList<User> &users) override;
};

#endif // REPORTSTRATEGY_H

