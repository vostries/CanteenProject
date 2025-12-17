#include "sortstrategy.h"
#include <algorithm>

QList<Meal> SortByNameStrategy::sort(const QList<Meal> &meals)
{
    QList<Meal> result = meals;
    std::sort(result.begin(), result.end(), 
              [](const Meal &a, const Meal &b) {
                  return a.getName() < b.getName();
              });
    return result;
}

QList<Meal> SortByPriceAscendingStrategy::sort(const QList<Meal> &meals)
{
    QList<Meal> result = meals;
    std::sort(result.begin(), result.end(),
              [](const Meal &a, const Meal &b) {
                  return a.getPrice() < b.getPrice();
              });
    return result;
}

QList<Meal> SortByPriceDescendingStrategy::sort(const QList<Meal> &meals)
{
    QList<Meal> result = meals;
    std::sort(result.begin(), result.end(),
              [](const Meal &a, const Meal &b) {
                  return a.getPrice() > b.getPrice();
              });
    return result;
}















