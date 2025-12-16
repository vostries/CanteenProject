#include "reportstrategy.h"
#include "user.h"
#include <QDate>
#include <algorithm>

QString RevenueReportStrategy::generateReport(const QList<Order> &orders,
                                             const QList<Meal> &meals,
                                             const QList<User> &users)
{
    double totalRevenue = 0.0;
    QMap<QDate, double> revenueByDate;
    
    for (const Order &order : orders) {
        totalRevenue += order.getTotalPrice();
        revenueByDate[order.getDate()] += order.getTotalPrice();
    }
    
    QString report = "=== ОТЧЕТ О ВЫРУЧКЕ ===\n\n";
    report += QString("Общая выручка: %1 руб.\n\n").arg(totalRevenue, 0, 'f', 2);
    
    report += "Выручка по датам:\n";
    QList<QDate> dates = revenueByDate.keys();
    std::sort(dates.begin(), dates.end());
    
    for (const QDate &date : dates) {
        report += QString("%1: %2 руб.\n")
                  .arg(date.toString("dd.MM.yyyy"))
                  .arg(revenueByDate[date], 0, 'f', 2);
    }
    
    return report;
}

QString PopularDishesReportStrategy::generateReport(const QList<Order> &orders,
                                                   const QList<Meal> &meals,
                                                   const QList<User> &users)
{
    QMap<int, int> mealCounts; // mealId -> quantity
    
    for (const Order &order : orders) {
        for (const auto &mealPair : order.getMeals()) {
            mealCounts[mealPair.first] += mealPair.second;
        }
    }
    
    QMap<QString, int> mealNameCounts;
    for (const Meal &meal : meals) {
        if (mealCounts.contains(meal.getId())) {
            mealNameCounts[meal.getName()] = mealCounts[meal.getId()];
        }
    }
    
    QString report = "=== ОТЧЕТ О ПОПУЛЯРНЫХ БЛЮДАХ ===\n\n";
    
    // Сортируем по количеству заказов
    QList<QPair<QString, int>> sorted;
    for (auto it = mealNameCounts.begin(); it != mealNameCounts.end(); ++it) {
        sorted.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.second > b.second;
              });
    
    for (const auto &pair : sorted) {
        report += QString("%1: %2 порций\n").arg(pair.first).arg(pair.second);
    }
    
    return report;
}

QString OrdersByDateReportStrategy::generateReport(const QList<Order> &orders,
                                                  const QList<Meal> &meals,
                                                  const QList<User> &users)
{
    QMap<QDate, QList<Order>> ordersByDate;
    
    for (const Order &order : orders) {
        ordersByDate[order.getDate()].append(order);
    }
    
    QString report = "=== ОТЧЕТ ПО ЗАКАЗАМ ПО ДАТАМ ===\n\n";
    
    QList<QDate> dates = ordersByDate.keys();
    std::sort(dates.begin(), dates.end());
    
    for (const QDate &date : dates) {
        report += QString("\nДата: %1\n").arg(date.toString("dd.MM.yyyy"));
        int count = 0;
        double dayRevenue = 0.0;
        
        for (const Order &order : ordersByDate[date]) {
            count++;
            dayRevenue += order.getTotalPrice();
        }
        
        report += QString("Количество заказов: %1\n").arg(count);
        report += QString("Выручка за день: %1 руб.\n").arg(dayRevenue, 0, 'f', 2);
    }
    
    return report;
}










