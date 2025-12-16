#include "studentwindow.h"
#include "datamanager.h"
#include "order.h"
#include "meal.h"
#include "category.h"
#include "sortstrategy.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QCloseEvent>

StudentWindow::StudentWindow(User *user, QWidget *parent)
    : QMainWindow(parent)
    , m_user(user)
    , m_orderObserver(new OrderObserver(this))
    , m_sortStrategy(nullptr)
{
    setWindowTitle("Столовая - " + user->getUsername());
    setupUI();
    refreshMeals();
    updateBalance();
    refreshOrders();
    
    connect(m_orderObserver, &OrderObserver::balanceUpdated, this, &StudentWindow::onBalanceUpdated);
}

StudentWindow::~StudentWindow()
{
    delete m_sortStrategy;
}

void StudentWindow::closeEvent(QCloseEvent *event)
{
    DataManager::getInstance().saveData();
    event->accept();
}

void StudentWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);
    
    setupMenuTab();
    setupOrdersTab();
    
    resize(900, 650);
}

void StudentWindow::setupMenuTab()
{
    m_menuTab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_menuTab);
    
    // Баланс
    m_balanceLabel = new QLabel();
    updateBalance();
    QFont balanceFont = m_balanceLabel->font();
    balanceFont.setPointSize(12);
    balanceFont.setBold(true);
    m_balanceLabel->setFont(balanceFont);
    mainLayout->addWidget(m_balanceLabel);
    
    // Поиск и фильтры
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Поиск:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Название блюда");
    m_searchEdit->setMinimumWidth(200);
    searchLayout->addWidget(m_searchEdit);
    
    searchLayout->addWidget(new QLabel("Категория:"));
    m_categoryFilterCombo = new QComboBox();
    m_categoryFilterCombo->addItem("Все категории", -1);
    loadCategories();
    searchLayout->addWidget(m_categoryFilterCombo);
    
    searchLayout->addWidget(new QLabel("Цена до:"));
    m_priceFilterCombo = new QComboBox();
    m_priceFilterCombo->addItem("Любая", -1);
    m_priceFilterCombo->addItem("До 100 руб.", 100);
    m_priceFilterCombo->addItem("До 200 руб.", 200);
    m_priceFilterCombo->addItem("До 300 руб.", 300);
    m_priceFilterCombo->addItem("До 500 руб.", 500);
    searchLayout->addWidget(m_priceFilterCombo);
    
    searchLayout->addWidget(new QLabel("Сортировка:"));
    m_sortCombo = new QComboBox();
    m_sortCombo->addItem("По названию", 0);
    m_sortCombo->addItem("По цене (возрастание)", 1);
    m_sortCombo->addItem("По цене (убывание)", 2);
    searchLayout->addWidget(m_sortCombo);
    
    mainLayout->addLayout(searchLayout);
    
    // Автоматический поиск при вводе
    connect(m_searchEdit, &QLineEdit::textChanged, this, &StudentWindow::onSearchMeals);
    connect(m_categoryFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentWindow::onFilterByCategory);
    connect(m_priceFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentWindow::onFilterByCategory);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentWindow::onSortMealsChanged);
    
    // Таблица меню (без столбца ID)
    m_mealsTable = new QTableWidget(0, 3, this);
    m_mealsTable->setHorizontalHeaderLabels({"Название", "Цена", "Категория"});
    m_mealsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_mealsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mealsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
    m_mealsTable->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем минимальную ширину столбца "Название" побольше
    m_mealsTable->setColumnWidth(0, 250);
    mainLayout->addWidget(m_mealsTable);
    
    // Двойной щелчок для добавления в корзину
    connect(m_mealsTable, &QTableWidget::cellDoubleClicked, this, &StudentWindow::onAddToCart);
    
    m_addToCartButton = new QPushButton("Добавить в корзину");
    connect(m_addToCartButton, &QPushButton::clicked, this, &StudentWindow::onAddToCart);
    mainLayout->addWidget(m_addToCartButton);
    
    // Корзина
    QLabel *cartLabel = new QLabel("Корзина:");
    QFont cartFont = cartLabel->font();
    cartFont.setBold(true);
    cartLabel->setFont(cartFont);
    mainLayout->addWidget(cartLabel);
    
    m_cartTable = new QTableWidget(0, 3, this);
    m_cartTable->setHorizontalHeaderLabels({"Название", "Цена", "Количество"});
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cartTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
    m_cartTable->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем минимальную ширину столбца "Название" побольше
    m_cartTable->setColumnWidth(0, 250);
    mainLayout->addWidget(m_cartTable);
    
    QHBoxLayout *cartButtonLayout = new QHBoxLayout();
    m_removeFromCartButton = new QPushButton("Удалить из корзины");
    m_placeOrderButton = new QPushButton("Оформить заказ");
    m_totalLabel = new QLabel("Итого: 0.00 руб.");
    QFont totalFont = m_totalLabel->font();
    totalFont.setBold(true);
    m_totalLabel->setFont(totalFont);
    
    cartButtonLayout->addWidget(m_removeFromCartButton);
    cartButtonLayout->addWidget(m_placeOrderButton);
    cartButtonLayout->addStretch();
    cartButtonLayout->addWidget(m_totalLabel);
    
    mainLayout->addLayout(cartButtonLayout);
    
    connect(m_removeFromCartButton, &QPushButton::clicked, this, &StudentWindow::onRemoveFromCart);
    connect(m_placeOrderButton, &QPushButton::clicked, this, &StudentWindow::onPlaceOrder);
    
    m_tabWidget->addTab(m_menuTab, "Меню и заказ");
}

void StudentWindow::setupOrdersTab()
{
    m_ordersTab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_ordersTab);
    
    QLabel *titleLabel = new QLabel("Мои заказы:");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    m_myOrdersTable = new QTableWidget(0, 4, this);
    m_myOrdersTable->setHorizontalHeaderLabels({"ID", "Дата", "Блюда", "Сумма"});
    m_myOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрет редактирования
    m_myOrdersTable->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем минимальную ширину столбца "Блюда" побольше
    m_myOrdersTable->setColumnWidth(2, 300);
    mainLayout->addWidget(m_myOrdersTable);
    
    m_tabWidget->addTab(m_ordersTab, "Мои заказы");
}

void StudentWindow::loadCategories()
{
    DataManager &dm = DataManager::getInstance();
    m_categoryFilterCombo->clear();
    m_categoryFilterCombo->addItem("Все категории", -1);
    
    for (const Category &cat : dm.getCategories()) {
        m_categoryFilterCombo->addItem(cat.getName(), cat.getId());
    }
}

void StudentWindow::refreshMeals()
{
    DataManager &dm = DataManager::getInstance();
    QList<Meal> meals = dm.getMeals();
    
    // Применяем сортировку
    if (m_sortStrategy) {
        meals = m_sortStrategy->sort(meals);
    }
    
    m_mealsTable->setRowCount(meals.size());
    
    for (int i = 0; i < meals.size(); ++i) {
        const Meal &meal = meals[i];
        // Сохраняем ID в UserRole для использования в getSelectedMealId
        QTableWidgetItem *nameItem = new QTableWidgetItem(meal.getName());
        nameItem->setData(Qt::UserRole, meal.getId());
        m_mealsTable->setItem(i, 0, nameItem);
        
        m_mealsTable->setItem(i, 1, new QTableWidgetItem(QString::number(meal.getPrice(), 'f', 2) + " руб."));
        
        Category *cat = dm.getCategoryById(meal.getCategoryId());
        QString catName = cat ? cat->getName() : "Неизвестно";
        m_mealsTable->setItem(i, 2, new QTableWidgetItem(catName));
    }
}

void StudentWindow::refreshCart()
{
    DataManager &dm = DataManager::getInstance();
    m_cartTable->setRowCount(m_cart.size());
    
    for (int i = 0; i < m_cart.size(); ++i) {
        int mealId = m_cart[i].first;
        int quantity = m_cart[i].second;
        
        Meal *meal = dm.getMealById(mealId);
        if (meal) {
            // Сохраняем ID в UserRole первой ячейки для использования в onRemoveFromCart
            QTableWidgetItem *nameItem = new QTableWidgetItem(meal->getName());
            nameItem->setData(Qt::UserRole, mealId);
            m_cartTable->setItem(i, 0, nameItem);
            
            m_cartTable->setItem(i, 1, new QTableWidgetItem(QString::number(meal->getPrice(), 'f', 2) + " руб."));
            m_cartTable->setItem(i, 2, new QTableWidgetItem(QString::number(quantity)));
        }
    }
    
    // Восстанавливаем минимальную ширину столбца "Название" после обновления корзины
    m_cartTable->setColumnWidth(0, 250);
    
    double total = calculateCartTotal();
    m_totalLabel->setText(QString("Итого: %1 руб.").arg(total, 0, 'f', 2));
}

void StudentWindow::refreshOrders()
{
    DataManager &dm = DataManager::getInstance();
    QList<Order> orders = dm.getOrdersByUserId(m_user->getId());
    
    // Сортируем по дате (новые сначала)
    std::sort(orders.begin(), orders.end(),
              [](const Order &a, const Order &b) {
                  return a.getDate() > b.getDate();
              });
    
    m_myOrdersTable->setRowCount(orders.size());
    
    for (int i = 0; i < orders.size(); ++i) {
        const Order &order = orders[i];
        m_myOrdersTable->setItem(i, 0, new QTableWidgetItem(QString::number(order.getId())));
        m_myOrdersTable->setItem(i, 1, new QTableWidgetItem(order.getDate().toString("dd.MM.yyyy")));
        
        QString mealsStr;
        for (const auto &mealPair : order.getMeals()) {
            Meal *meal = dm.getMealById(mealPair.first);
            if (meal) {
                mealsStr += QString("%1 (x%2), ").arg(meal->getName()).arg(mealPair.second);
            }
        }
        if (mealsStr.endsWith(", ")) {
            mealsStr.chop(2);
        }
        m_myOrdersTable->setItem(i, 2, new QTableWidgetItem(mealsStr));
        m_myOrdersTable->setItem(i, 3, new QTableWidgetItem(QString::number(order.getTotalPrice(), 'f', 2) + " руб."));
    }
    
    // Восстанавливаем минимальную ширину столбца "Блюда" после загрузки данных
    m_myOrdersTable->setColumnWidth(2, 300);
}

int StudentWindow::getSelectedMealId()
{
    int row = m_mealsTable->currentRow();
    if (row >= 0) {
        QTableWidgetItem *item = m_mealsTable->item(row, 0);
        if (item) {
            return item->data(Qt::UserRole).toInt();
        }
    }
    return -1;
}

double StudentWindow::calculateCartTotal()
{
    DataManager &dm = DataManager::getInstance();
    double total = 0.0;
    
    for (const auto &item : m_cart) {
        Meal *meal = dm.getMealById(item.first);
        if (meal) {
            total += meal->getPrice() * item.second;
        }
    }
    
    return total;
}

void StudentWindow::updateBalance()
{
    m_balanceLabel->setText(QString("Ваш баланс: %1 руб.").arg(m_user->getBalance(), 0, 'f', 2));
}

void StudentWindow::onBalanceUpdated(int userId, double newBalance)
{
    if (userId == m_user->getId()) {
        m_user->setBalance(newBalance);
        updateBalance();
    }
}

void StudentWindow::onSearchMeals()
{
    QString searchText = m_searchEdit->text().trimmed().toLower();
    DataManager &dm = DataManager::getInstance();
    QList<Meal> allMeals = dm.getMeals();
    QList<Meal> filtered;
    
    for (const Meal &meal : allMeals) {
        if (searchText.isEmpty() || meal.getName().toLower().contains(searchText)) {
            filtered.append(meal);
        }
    }
    
    // Применяем сортировку
    if (m_sortStrategy) {
        filtered = m_sortStrategy->sort(filtered);
    }
    
    m_mealsTable->setRowCount(filtered.size());
    
    for (int i = 0; i < filtered.size(); ++i) {
        const Meal &meal = filtered[i];
        // Сохраняем ID в UserRole для использования в getSelectedMealId
        QTableWidgetItem *nameItem = new QTableWidgetItem(meal.getName());
        nameItem->setData(Qt::UserRole, meal.getId());
        m_mealsTable->setItem(i, 0, nameItem);
        
        m_mealsTable->setItem(i, 1, new QTableWidgetItem(QString::number(meal.getPrice(), 'f', 2) + " руб."));
        
        Category *cat = dm.getCategoryById(meal.getCategoryId());
        QString catName = cat ? cat->getName() : "Неизвестно";
        m_mealsTable->setItem(i, 2, new QTableWidgetItem(catName));
    }
}

void StudentWindow::onFilterByCategory()
{
    DataManager &dm = DataManager::getInstance();
    QList<Meal> allMeals = dm.getMeals();
    QList<Meal> filtered;
    
    int categoryId = m_categoryFilterCombo->currentData().toInt();
    double maxPrice = m_priceFilterCombo->currentData().toDouble();
    
    for (const Meal &meal : allMeals) {
        bool matches = true;
        
        if (categoryId != -1 && meal.getCategoryId() != categoryId) {
            matches = false;
        }
        
        if (maxPrice != -1 && meal.getPrice() > maxPrice) {
            matches = false;
        }
        
        if (matches) {
            filtered.append(meal);
        }
    }
    
    // Применяем сортировку
    if (m_sortStrategy) {
        filtered = m_sortStrategy->sort(filtered);
    }
    
    m_mealsTable->setRowCount(filtered.size());
    
    for (int i = 0; i < filtered.size(); ++i) {
        const Meal &meal = filtered[i];
        // Сохраняем ID в UserRole для использования в getSelectedMealId
        QTableWidgetItem *nameItem = new QTableWidgetItem(meal.getName());
        nameItem->setData(Qt::UserRole, meal.getId());
        m_mealsTable->setItem(i, 0, nameItem);
        
        m_mealsTable->setItem(i, 1, new QTableWidgetItem(QString::number(meal.getPrice(), 'f', 2) + " руб."));
        
        Category *cat = dm.getCategoryById(meal.getCategoryId());
        QString catName = cat ? cat->getName() : "Неизвестно";
        m_mealsTable->setItem(i, 2, new QTableWidgetItem(catName));
    }
}

void StudentWindow::onAddToCart()
{
    int mealId = getSelectedMealId();
    if (mealId < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите блюдо из меню");
        return;
    }
    
    // Проверяем, есть ли уже это блюдо в корзине
    bool found = false;
    for (auto &item : m_cart) {
        if (item.first == mealId) {
            item.second++;
            found = true;
            break;
        }
    }
    
    if (!found) {
        m_cart.append(qMakePair(mealId, 1));
    }
    
    refreshCart();
}

void StudentWindow::onRemoveFromCart()
{
    int row = m_cartTable->currentRow();
    if (row < 0 || row >= m_cart.size()) {
        QMessageBox::warning(this, "Ошибка", "Выберите блюдо из корзины");
        return;
    }
    
    // Получаем ID из UserRole первой ячейки
    QTableWidgetItem *item = m_cartTable->item(row, 0);
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите блюдо из корзины");
        return;
    }
    int mealId = item->data(Qt::UserRole).toInt();
    
    // Уменьшаем количество или удаляем
    for (int i = 0; i < m_cart.size(); ++i) {
        if (m_cart[i].first == mealId) {
            if (m_cart[i].second > 1) {
                m_cart[i].second--;
            } else {
                m_cart.removeAt(i);
            }
            break;
        }
    }
    
    refreshCart();
}

void StudentWindow::onPlaceOrder()
{
    if (m_cart.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Корзина пуста");
        return;
    }
    
    double total = calculateCartTotal();
    
    if (m_user->getBalance() < total) {
        QMessageBox::warning(this, "Ошибка", QString("Недостаточно средств. Необходимо: %1 руб., у вас: %2 руб.")
                            .arg(total, 0, 'f', 2)
                            .arg(m_user->getBalance(), 0, 'f', 2));
        return;
    }
    
    int ret = QMessageBox::question(this, "Подтверждение заказа",
                                    QString("Оформить заказ на сумму %1 руб.?").arg(total, 0, 'f', 2),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        DataManager &dm = DataManager::getInstance();
        Order order(dm.getNextOrderId(), m_user->getId(), QDate::currentDate(), m_cart);
        order.setTotalPrice(total);
        
        // Используем Observer для обновления баланса
        m_orderObserver->notifyOrderPlaced(&order, m_user, total);
        
        dm.addOrder(order);
        dm.updateUser(*m_user);
        
        m_cart.clear();
        refreshCart();
        refreshOrders();
        updateBalance();
        
        QMessageBox::information(this, "Успех", "Заказ успешно оформлен!");
    }
}

void StudentWindow::onSortMealsChanged()
{
    int index = m_sortCombo->currentData().toInt();
    delete m_sortStrategy;
    
    switch (index) {
    case 0:
        m_sortStrategy = new SortByNameStrategy();
        break;
    case 1:
        m_sortStrategy = new SortByPriceAscendingStrategy();
        break;
    case 2:
        m_sortStrategy = new SortByPriceDescendingStrategy();
        break;
    default:
        m_sortStrategy = nullptr;
    }
    
    refreshMeals();
}

