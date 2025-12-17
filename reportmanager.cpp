#include "reportmanager.h"
#include "order.h"
#include "meal.h"
#include "user.h"

ReportManager::ReportManager()
    : m_strategy(nullptr)
{
}

ReportManager::~ReportManager()
{
    delete m_strategy;
}

void ReportManager::setStrategy(ReportStrategy *strategy)
{
    if (m_strategy != strategy) {
        delete m_strategy;
        m_strategy = strategy;
    }
}

QString ReportManager::generateReport(const QList<Order> &orders,
                                     const QList<Meal> &meals,
                                     const QList<User> &users)
{
    if (m_strategy) {
        return m_strategy->generateReport(orders, meals, users);
    }
    return "Стратегия не установлена";
}















