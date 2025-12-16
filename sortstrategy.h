#ifndef SORTSTRATEGY_H
#define SORTSTRATEGY_H

#include <QList>
#include "meal.h"

// Strategy pattern - разные стратегии сортировки
class SortStrategy
{
public:
    virtual ~SortStrategy() = default;
    virtual QList<Meal> sort(const QList<Meal> &meals) = 0;
};

class SortByNameStrategy : public SortStrategy
{
public:
    QList<Meal> sort(const QList<Meal> &meals) override;
};

class SortByPriceAscendingStrategy : public SortStrategy
{
public:
    QList<Meal> sort(const QList<Meal> &meals) override;
};

class SortByPriceDescendingStrategy : public SortStrategy
{
public:
    QList<Meal> sort(const QList<Meal> &meals) override;
};

#endif // SORTSTRATEGY_H










