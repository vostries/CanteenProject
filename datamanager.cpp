#include "datamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

DataManager& DataManager::getInstance()
{
    static DataManager instance;
    return instance;
}

DataManager::DataManager()
    : m_nextUserId(1)
    , m_nextMealId(1)
    , m_nextOrderId(1)
    , m_nextCategoryId(1)
{
    // Определяем путь к файлу данных - сохраняем в корне проекта
    // Ищем директорию проекта (там где CMakeLists.txt)
    QDir dir(QCoreApplication::applicationDirPath());
    
    // Поднимаемся от исполняемого файла до корня проекта
    // Ищем директорию с CMakeLists.txt, поднимаясь вверх по дереву
    QFileInfo cmakeFile(dir, "CMakeLists.txt");
    while (!cmakeFile.exists() && dir.cdUp() && !dir.isRoot()) {
        cmakeFile = QFileInfo(dir, "CMakeLists.txt");
    }
    
    m_dataFile = dir.absoluteFilePath("cafeteria_data.json");
    
    // Инициализация базовых категорий
    m_categories.append(Category(1, "Завтрак"));
    m_categories.append(Category(2, "Обед"));
    m_categories.append(Category(3, "Перекус"));
    m_nextCategoryId = 4;
    
    loadData();
}

void DataManager::loadData()
{
    QFile file(m_dataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        // Создаем администратора по умолчанию
        User admin(1, "admin", "admin", UserType::Admin, 0.0);
        m_users.append(admin);
        m_nextUserId = 2;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // Загрузка пользователей
    m_users.clear();
    QJsonArray usersArray = root["users"].toArray();
    for (const auto &value : usersArray) {
        QJsonObject userObj = value.toObject();
        User user(
            userObj["id"].toInt(),
            userObj["username"].toString(),
            userObj["password"].toString(),
            static_cast<UserType>(userObj["type"].toInt()),
            userObj["balance"].toDouble()
        );
        m_users.append(user);
        if (user.getId() >= m_nextUserId) {
            m_nextUserId = user.getId() + 1;
        }
    }
    
    // Загрузка категорий
    if (root.contains("categories")) {
        m_categories.clear();
        QJsonArray categoriesArray = root["categories"].toArray();
        for (const auto &value : categoriesArray) {
            QJsonObject catObj = value.toObject();
            Category cat(catObj["id"].toInt(), catObj["name"].toString());
            m_categories.append(cat);
            if (cat.getId() >= m_nextCategoryId) {
                m_nextCategoryId = cat.getId() + 1;
            }
        }
    }
    
    // Загрузка блюд
    m_meals.clear();
    QJsonArray mealsArray = root["meals"].toArray();
    for (const auto &value : mealsArray) {
        QJsonObject mealObj = value.toObject();
        Meal meal(
            mealObj["id"].toInt(),
            mealObj["name"].toString(),
            mealObj["price"].toDouble(),
            mealObj["categoryId"].toInt(),
            mealObj.contains("imagePath") ? mealObj["imagePath"].toString() : QString()
        );
        m_meals.append(meal);
        if (meal.getId() >= m_nextMealId) {
            m_nextMealId = meal.getId() + 1;
        }
    }
    
    // Загрузка заказов
    m_orders.clear();
    QJsonArray ordersArray = root["orders"].toArray();
    for (const auto &value : ordersArray) {
        QJsonObject orderObj = value.toObject();
        QDate date = QDate::fromString(orderObj["date"].toString(), Qt::ISODate);
        
        QList<QPair<int, int>> meals;
        QJsonArray mealsArray = orderObj["meals"].toArray();
        for (const auto &mealValue : mealsArray) {
            QJsonObject mealObj = mealValue.toObject();
            meals.append(qMakePair(mealObj["mealId"].toInt(), mealObj["quantity"].toInt()));
        }
        
        Order order(
            orderObj["id"].toInt(),
            orderObj["userId"].toInt(),
            date,
            meals
        );
        order.setTotalPrice(orderObj["totalPrice"].toDouble());
        m_orders.append(order);
        if (order.getId() >= m_nextOrderId) {
            m_nextOrderId = order.getId() + 1;
        }
    }
    
    if (m_users.isEmpty()) {
        User admin(1, "admin", "admin", UserType::Admin, 0.0);
        m_users.append(admin);
        m_nextUserId = 2;
    }
}

void DataManager::saveData()
{
    QJsonObject root;
    
    // Сохранение пользователей
    QJsonArray usersArray;
    for (const User &user : m_users) {
        QJsonDocument doc = QJsonDocument::fromJson(user.toJson().toUtf8());
        usersArray.append(doc.object());
    }
    root["users"] = usersArray;
    
    // Сохранение категорий
    QJsonArray categoriesArray;
    for (const Category &cat : m_categories) {
        QJsonDocument doc = QJsonDocument::fromJson(cat.toJson().toUtf8());
        categoriesArray.append(doc.object());
    }
    root["categories"] = categoriesArray;
    
    // Сохранение блюд
    QJsonArray mealsArray;
    for (const Meal &meal : m_meals) {
        QJsonDocument doc = QJsonDocument::fromJson(meal.toJson().toUtf8());
        mealsArray.append(doc.object());
    }
    root["meals"] = mealsArray;
    
    // Сохранение заказов
    QJsonArray ordersArray;
    for (const Order &order : m_orders) {
        QJsonDocument doc = QJsonDocument::fromJson(order.toJson().toUtf8());
        ordersArray.append(doc.object());
    }
    root["orders"] = ordersArray;
    
    QJsonDocument doc(root);
    QFile file(m_dataFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

User* DataManager::findUser(const QString &username, const QString &password)
{
    for (auto &user : m_users) {
        if (user.getUsername() == username && user.verifyPassword(password)) {
            return &user;
        }
    }
    return nullptr;
}

User* DataManager::getUserById(int id)
{
    for (auto &user : m_users) {
        if (user.getId() == id) {
            return &user;
        }
    }
    return nullptr;
}

void DataManager::addUser(const User &user)
{
    m_users.append(user);
    saveData();
}

void DataManager::updateUser(const User &user)
{
    for (auto &u : m_users) {
        if (u.getId() == user.getId()) {
            u = user;
            saveData();
            break;
        }
    }
}

Meal* DataManager::getMealById(int id)
{
    for (auto &meal : m_meals) {
        if (meal.getId() == id) {
            return &meal;
        }
    }
    return nullptr;
}

void DataManager::addMeal(const Meal &meal)
{
    m_meals.append(meal);
    saveData();
}

void DataManager::updateMeal(const Meal &meal)
{
    for (auto &m : m_meals) {
        if (m.getId() == meal.getId()) {
            m = meal;
            saveData();
            break;
        }
    }
}

void DataManager::removeMeal(int id)
{
    for (int i = 0; i < m_meals.size(); ++i) {
        if (m_meals[i].getId() == id) {
            m_meals.removeAt(i);
            saveData();
            break;
        }
    }
}

void DataManager::addOrder(const Order &order)
{
    m_orders.append(order);
    saveData();
}

QList<Order> DataManager::getOrdersByUserId(int userId) const
{
    QList<Order> result;
    for (const Order &order : m_orders) {
        if (order.getUserId() == userId) {
            result.append(order);
        }
    }
    return result;
}

QList<Order> DataManager::getOrdersByDate(const QDate &date) const
{
    QList<Order> result;
    for (const Order &order : m_orders) {
        if (order.getDate() == date) {
            result.append(order);
        }
    }
    return result;
}

Category* DataManager::getCategoryById(int id)
{
    for (auto &cat : m_categories) {
        if (cat.getId() == id) {
            return &cat;
        }
    }
    return nullptr;
}

void DataManager::addCategory(const Category &category)
{
    m_categories.append(category);
    saveData();
}

int DataManager::getNextUserId()
{
    return m_nextUserId++;
}

int DataManager::getNextMealId()
{
    return m_nextMealId++;
}

int DataManager::getNextOrderId()
{
    return m_nextOrderId++;
}

int DataManager::getNextCategoryId()
{
    return m_nextCategoryId++;
}

bool DataManager::exportMenu(const QString &filename)
{
    QJsonObject root;
    
    QJsonArray categoriesArray;
    for (const Category &cat : m_categories) {
        QJsonDocument doc = QJsonDocument::fromJson(cat.toJson().toUtf8());
        categoriesArray.append(doc.object());
    }
    root["categories"] = categoriesArray;
    
    QJsonArray mealsArray;
    for (const Meal &meal : m_meals) {
        QJsonDocument doc = QJsonDocument::fromJson(meal.toJson().toUtf8());
        mealsArray.append(doc.object());
    }
    root["meals"] = mealsArray;
    
    QJsonDocument doc(root);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        return true;
    }
    return false;
}

bool DataManager::importMenu(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    // Импорт категорий
    if (root.contains("categories")) {
        QJsonArray categoriesArray = root["categories"].toArray();
        for (const auto &value : categoriesArray) {
            QJsonObject catObj = value.toObject();
            int id = catObj["id"].toInt();
            bool exists = false;
            for (const Category &cat : m_categories) {
                if (cat.getId() == id) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                Category cat(id, catObj["name"].toString());
                m_categories.append(cat);
                if (cat.getId() >= m_nextCategoryId) {
                    m_nextCategoryId = cat.getId() + 1;
                }
            }
        }
    }
    
    // Импорт блюд
    if (root.contains("meals")) {
        QJsonArray mealsArray = root["meals"].toArray();
        for (const auto &value : mealsArray) {
            QJsonObject mealObj = value.toObject();
            Meal meal(
                mealObj["id"].toInt(),
                mealObj["name"].toString(),
                mealObj["price"].toDouble(),
                mealObj["categoryId"].toInt(),
                mealObj.contains("imagePath") ? mealObj["imagePath"].toString() : QString()
            );
            
            bool exists = false;
            for (int i = 0; i < m_meals.size(); ++i) {
                if (m_meals[i].getId() == meal.getId()) {
                    m_meals[i] = meal;
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                m_meals.append(meal);
            }
            if (meal.getId() >= m_nextMealId) {
                m_nextMealId = meal.getId() + 1;
            }
        }
    }
    
    saveData();
    return true;
}

