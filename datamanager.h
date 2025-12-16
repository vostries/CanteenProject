#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "user.h"
#include "meal.h"
#include "order.h"
#include "category.h"
#include <QString>
#include <QList>

class DataManager
{
public:
    static DataManager& getInstance();
    
    void loadData();
    void saveData();
    
    // Users
    QList<User> getUsers() const { return m_users; }
    User* findUser(const QString &username, const QString &password);
    User* getUserById(int id);
    void addUser(const User &user);
    void updateUser(const User &user);
    
    // Meals
    QList<Meal> getMeals() const { return m_meals; }
    Meal* getMealById(int id);
    void addMeal(const Meal &meal);
    void updateMeal(const Meal &meal);
    void removeMeal(int id);
    
    // Orders
    QList<Order> getOrders() const { return m_orders; }
    void addOrder(const Order &order);
    QList<Order> getOrdersByUserId(int userId) const;
    QList<Order> getOrdersByDate(const QDate &date) const;
    
    // Categories
    QList<Category> getCategories() const { return m_categories; }
    Category* getCategoryById(int id);
    void addCategory(const Category &category);
    
    int getNextUserId();
    int getNextMealId();
    int getNextOrderId();
    int getNextCategoryId();
    
    bool exportMenu(const QString &filename);
    bool importMenu(const QString &filename);

private:
    DataManager();
    ~DataManager() = default;
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;
    
    QString m_dataFile;
    QList<User> m_users;
    QList<Meal> m_meals;
    QList<Order> m_orders;
    QList<Category> m_categories;
    
    int m_nextUserId;
    int m_nextMealId;
    int m_nextOrderId;
    int m_nextCategoryId;
};

#endif // DATAMANAGER_H


