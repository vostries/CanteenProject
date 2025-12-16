#include "adminwindow.h"
#include "datamanager.h"
#include "reportstrategy.h"
#include "categorydelegate.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPixmap>
#include <QFrame>
#include <QFileInfo>
#include <QLineEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QIODevice>
#include <QDate>
#include <QMouseEvent>
#include <QApplication>

AdminWindow::AdminWindow(User *user, QWidget *parent)
    : QMainWindow(parent)
    , m_user(user)
    , m_reportManager(new ReportManager())
    , m_sortStrategy(nullptr)
    , m_isLoadingMeals(false)
{
    setWindowTitle("Панель администратора - " + user->getUsername());
    setupUI();
    loadCategories();
    loadMeals();
    loadOrders();
}

AdminWindow::~AdminWindow()
{
    delete m_reportManager;
    delete m_sortStrategy;
}

void AdminWindow::closeEvent(QCloseEvent *event)
{
    DataManager::getInstance().saveData();
    event->accept();
}

void AdminWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);
    
    setupMenuTab();
    setupOrdersTab();
    setupReportsTab();
    
    // Устанавливаем фильтр событий для снятия выделения при клике вне таблицы
    qApp->installEventFilter(this);
    
    resize(1000, 700);
}

void AdminWindow::setupMenuTab()
{
    m_menuTab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_menuTab);
    
    // Сортировка
    QHBoxLayout *sortLayout = new QHBoxLayout();
    sortLayout->addWidget(new QLabel("Сортировка:"));
    m_sortCombo = new QComboBox();
    m_sortCombo->addItem("По названию", 0);
    m_sortCombo->addItem("По цене (возрастание)", 1);
    m_sortCombo->addItem("По цене (убывание)", 2);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdminWindow::onSortMealsChanged);
    sortLayout->addWidget(m_sortCombo);
    sortLayout->addStretch();
    mainLayout->addLayout(sortLayout);
    
    // Таблица блюд
    m_mealsTable = new QTableWidget(0, 4, this);
    m_mealsTable->setHorizontalHeaderLabels({"ID", "Название", "Цена (руб.)", "Категория"});
    m_mealsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_mealsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mealsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_mealsTable->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем минимальную ширину столбца "Название" побольше
    m_mealsTable->setColumnWidth(1, 250);
    
    // Делегат для категории
    CategoryDelegate *categoryDelegate = new CategoryDelegate(this);
    m_mealsTable->setItemDelegateForColumn(3, categoryDelegate);
    
    connect(m_mealsTable, &QTableWidget::itemSelectionChanged, this, &AdminWindow::onMealSelectionChanged);
    connect(m_mealsTable, &QTableWidget::cellChanged, this, &AdminWindow::onMealCellChanged);
    mainLayout->addWidget(m_mealsTable);
    
    // Форма добавления/редактирования
    QHBoxLayout *formLayout = new QHBoxLayout();
    
    formLayout->addWidget(new QLabel("Название:"));
    m_mealNameEdit = new QLineEdit();
    formLayout->addWidget(m_mealNameEdit);
    
    formLayout->addWidget(new QLabel("Цена:"));
    m_mealPriceSpin = new QDoubleSpinBox();
    m_mealPriceSpin->setMinimum(0.0);
    m_mealPriceSpin->setMaximum(10000.0);
    m_mealPriceSpin->setDecimals(2);
    formLayout->addWidget(m_mealPriceSpin);
    
    formLayout->addWidget(new QLabel("Категория:"));
    m_mealCategoryCombo = new QComboBox();
    formLayout->addWidget(m_mealCategoryCombo);
    
    // Выбор изображения для блюда
    formLayout->addWidget(new QLabel("Фото:"));
    m_mealImagePathEdit = new QLineEdit();
    m_mealImagePathEdit->setReadOnly(true);
    formLayout->addWidget(m_mealImagePathEdit);
    
    m_selectImageButton = new QPushButton("Выбрать...");
    formLayout->addWidget(m_selectImageButton);
    
    mainLayout->addLayout(formLayout);
    
    // Превью изображения
    m_imagePreview = new QLabel();
    m_imagePreview->setFixedSize(80, 80);
    m_imagePreview->setScaledContents(true);
    m_imagePreview->setFrameShape(QFrame::StyledPanel);
    mainLayout->addWidget(m_imagePreview);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_addMealButton = new QPushButton("Добавить");
    m_editMealButton = new QPushButton("Изменить");
    m_editMealButton->setEnabled(false);
    m_deleteMealButton = new QPushButton("Удалить");
    m_deleteMealButton->setEnabled(false);
    m_exportMenuButton = new QPushButton("Экспорт меню");
    m_importMenuButton = new QPushButton("Импорт меню");
    
    buttonLayout->addWidget(m_addMealButton);
    buttonLayout->addWidget(m_editMealButton);
    buttonLayout->addWidget(m_deleteMealButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exportMenuButton);
    buttonLayout->addWidget(m_importMenuButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(m_addMealButton, &QPushButton::clicked, this, &AdminWindow::onAddMeal);
    connect(m_editMealButton, &QPushButton::clicked, this, &AdminWindow::onEditMeal);
    connect(m_deleteMealButton, &QPushButton::clicked, this, &AdminWindow::onDeleteMeal);
    connect(m_exportMenuButton, &QPushButton::clicked, this, &AdminWindow::onExportMenu);
    connect(m_importMenuButton, &QPushButton::clicked, this, &AdminWindow::onImportMenu);
    connect(m_selectImageButton, &QPushButton::clicked, [this]() {
        QString filename = QFileDialog::getOpenFileName(this, "Выбор изображения блюда", "", "Изображения (*.png *.jpg *.jpeg *.bmp)");
        if (!filename.isEmpty()) {
            m_mealImagePathEdit->setText(filename);
            QPixmap pix(filename);
            if (!pix.isNull()) {
                m_imagePreview->setPixmap(pix);
            }
        }
    });
    
    m_tabWidget->addTab(m_menuTab, "Управление меню");
}

void AdminWindow::setupOrdersTab()
{
    m_ordersTab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_ordersTab);
    
    // Фильтры
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Дата:"));
    m_filterDateEdit = new QDateEdit();
    m_filterDateEdit->setDate(QDate::currentDate());
    m_filterDateEdit->setCalendarPopup(true);
    filterLayout->addWidget(m_filterDateEdit);
    
    filterLayout->addWidget(new QLabel("ID или логин ученика:"));
    m_filterUserEdit = new QLineEdit();
    m_filterUserEdit->setPlaceholderText("Введите ID или логин");
    filterLayout->addWidget(m_filterUserEdit);
    
    m_clearFilterButton = new QPushButton("Очистить");
    filterLayout->addWidget(m_clearFilterButton);
    m_exportOrdersButton = new QPushButton("Экспорт в JSON");
    filterLayout->addWidget(m_exportOrdersButton);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);
    
    // Таблица заказов
    m_ordersTable = new QTableWidget(0, 6, this);
    m_ordersTable->setHorizontalHeaderLabels({"ID", "Дата", "ID ученика", "Логин", "Блюда", "Сумма"});
    m_ordersTable->horizontalHeader()->setStretchLastSection(true);
    // Устанавливаем минимальную ширину столбца "Блюда" побольше
    m_ordersTable->setColumnWidth(4, 300);
    mainLayout->addWidget(m_ordersTable);
    
    // Автоматическое применение фильтра при изменении даты и вводе текста
    connect(m_filterDateEdit, &QDateEdit::dateChanged, this, &AdminWindow::onFilterOrders);
    connect(m_filterUserEdit, &QLineEdit::textChanged, this, &AdminWindow::onFilterOrders);
    connect(m_clearFilterButton, &QPushButton::clicked, this, &AdminWindow::refreshOrders);
    connect(m_exportOrdersButton, &QPushButton::clicked, this, &AdminWindow::onExportOrders);
    
    m_tabWidget->addTab(m_ordersTab, "Заказы");
}

void AdminWindow::setupReportsTab()
{
    m_reportsTab = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_reportsTab);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_revenueReportButton = new QPushButton("Отчет о выручке");
    m_popularDishesReportButton = new QPushButton("Популярные блюда");
    m_ordersByDateReportButton = new QPushButton("Заказы по датам");
    
    buttonLayout->addWidget(m_revenueReportButton);
    buttonLayout->addWidget(m_popularDishesReportButton);
    buttonLayout->addWidget(m_ordersByDateReportButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    m_reportText = new QTextEdit();
    m_reportText->setReadOnly(true);
    mainLayout->addWidget(m_reportText);
    
    connect(m_revenueReportButton, &QPushButton::clicked, this, &AdminWindow::onGenerateRevenueReport);
    connect(m_popularDishesReportButton, &QPushButton::clicked, this, &AdminWindow::onGeneratePopularDishesReport);
    connect(m_ordersByDateReportButton, &QPushButton::clicked, this, &AdminWindow::onGenerateOrdersByDateReport);
    
    m_tabWidget->addTab(m_reportsTab, "Отчеты");
}

void AdminWindow::loadCategories()
{
    DataManager &dm = DataManager::getInstance();
    m_mealCategoryCombo->clear();
    
    for (const Category &cat : dm.getCategories()) {
        m_mealCategoryCombo->addItem(cat.getName(), cat.getId());
    }
}

void AdminWindow::loadMeals()
{
    m_isLoadingMeals = true; // Блокируем обработку изменений
    
    DataManager &dm = DataManager::getInstance();
    QList<Meal> meals = dm.getMeals();
    
    // Применяем сортировку
    if (m_sortStrategy) {
        meals = m_sortStrategy->sort(meals);
    }
    
    m_mealsTable->setRowCount(meals.size());
    
    for (int i = 0; i < meals.size(); ++i) {
        const Meal &meal = meals[i];
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(meal.getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable); // ID не редактируется
        m_mealsTable->setItem(i, 0, idItem);
        
        m_mealsTable->setItem(i, 1, new QTableWidgetItem(meal.getName()));
        
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(meal.getPrice(), 'f', 2));
        m_mealsTable->setItem(i, 2, priceItem);
        
        Category *cat = dm.getCategoryById(meal.getCategoryId());
        QString catName = cat ? cat->getName() : "Неизвестно";
        QTableWidgetItem *catItem = new QTableWidgetItem(catName);
        catItem->setData(Qt::UserRole, meal.getCategoryId()); // Сохраняем ID категории
        m_mealsTable->setItem(i, 3, catItem);
    }
    
    // Восстанавливаем минимальную ширину столбца "Название" после загрузки данных
    m_mealsTable->setColumnWidth(1, 250);
    
    m_isLoadingMeals = false; // Разблокируем
}

void AdminWindow::loadOrders()
{
    DataManager &dm = DataManager::getInstance();
    QList<Order> orders = dm.getOrders();
    
    // Сохраняем все заказы для экспорта
    m_filteredOrders = orders;
    
    m_ordersTable->setRowCount(orders.size());
    
    for (int i = 0; i < orders.size(); ++i) {
        const Order &order = orders[i];
        m_ordersTable->setItem(i, 0, new QTableWidgetItem(QString::number(order.getId())));
        m_ordersTable->setItem(i, 1, new QTableWidgetItem(order.getDate().toString("dd.MM.yyyy")));
        m_ordersTable->setItem(i, 2, new QTableWidgetItem(QString::number(order.getUserId())));
        
        // Получаем логин пользователя
        User *user = dm.getUserById(order.getUserId());
        QString username = user ? user->getUsername() : "Неизвестно";
        m_ordersTable->setItem(i, 3, new QTableWidgetItem(username));
        
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
        m_ordersTable->setItem(i, 4, new QTableWidgetItem(mealsStr));
        m_ordersTable->setItem(i, 5, new QTableWidgetItem(QString::number(order.getTotalPrice(), 'f', 2) + " руб."));
    }
    
    // Восстанавливаем минимальную ширину столбца "Блюда" после загрузки данных
    m_ordersTable->setColumnWidth(4, 300);
}

int AdminWindow::getSelectedMealId()
{
    int row = m_mealsTable->currentRow();
    if (row >= 0) {
        return m_mealsTable->item(row, 0)->text().toInt();
    }
    return -1;
}

void AdminWindow::onMealSelectionChanged()
{
    bool hasSelection = m_mealsTable->currentRow() >= 0;
    m_editMealButton->setEnabled(hasSelection);
    m_deleteMealButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        int mealId = getSelectedMealId();
        DataManager &dm = DataManager::getInstance();
        Meal *meal = dm.getMealById(mealId);
        if (meal) {
            m_mealNameEdit->setText(meal->getName());
            m_mealPriceSpin->setValue(meal->getPrice());
            
            int index = m_mealCategoryCombo->findData(meal->getCategoryId());
            if (index >= 0) {
                m_mealCategoryCombo->setCurrentIndex(index);
            }
            
            // Загружаем изображение
            m_mealImagePathEdit->setText(meal->getImagePath());
            if (!meal->getImagePath().isEmpty() && QFileInfo::exists(meal->getImagePath())) {
                QPixmap pix(meal->getImagePath());
                if (!pix.isNull()) {
                    m_imagePreview->setPixmap(pix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    m_imagePreview->clear();
                }
            } else {
                m_imagePreview->clear();
            }
        }
    } else {
        // Очищаем форму при отсутствии выбора
        clearMealForm();
    }
}

void AdminWindow::onAddMeal()
{
    QString name = m_mealNameEdit->text().trimmed();
    double price = m_mealPriceSpin->value();
    int categoryId = m_mealCategoryCombo->currentData().toInt();
    QString imagePath = m_mealImagePathEdit->text().trimmed();
    
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название блюда");
        return;
    }
    
    DataManager &dm = DataManager::getInstance();
    Meal meal(dm.getNextMealId(), name, price, categoryId, imagePath);
    dm.addMeal(meal);
    
    // Очищаем форму после добавления
    clearMealForm();
    refreshMeals();
}

void AdminWindow::onEditMeal()
{
    int mealId = getSelectedMealId();
    if (mealId < 0) return;
    
    QString name = m_mealNameEdit->text().trimmed();
    double price = m_mealPriceSpin->value();
    int categoryId = m_mealCategoryCombo->currentData().toInt();
    QString imagePath = m_mealImagePathEdit->text().trimmed();
    
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название блюда");
        return;
    }
    
    DataManager &dm = DataManager::getInstance();
    Meal *meal = dm.getMealById(mealId);
    if (meal) {
        meal->setName(name);
        meal->setPrice(price);
        meal->setCategoryId(categoryId);
        meal->setImagePath(imagePath);
        dm.updateMeal(*meal);
        refreshMeals();
        // Очищаем форму после редактирования
        clearMealForm();
    }
}

void AdminWindow::onDeleteMeal()
{
    int mealId = getSelectedMealId();
    if (mealId < 0) return;
    
    int ret = QMessageBox::question(this, "Подтверждение", "Удалить это блюдо?", 
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        DataManager::getInstance().removeMeal(mealId);
        refreshMeals();
    }
}

void AdminWindow::refreshMeals()
{
    loadMeals();
}

void AdminWindow::refreshOrders()
{
    m_filterUserEdit->clear();
    loadOrders();
}

void AdminWindow::onFilterOrders()
{
    DataManager &dm = DataManager::getInstance();
    QList<Order> orders = dm.getOrders();
    
    QDate filterDate = m_filterDateEdit->date();
    QString userFilterStr = m_filterUserEdit->text().trimmed();
    
    QList<Order> filtered;
    for (const Order &order : orders) {
        bool matches = true;
        
        // Фильтр по дате - применяется всегда если установлена дата
        if (filterDate.isValid() && order.getDate() != filterDate) {
            matches = false;
        }
        
        // Фильтр по ID или логину - применяется только если введено значение
        if (matches && !userFilterStr.isEmpty()) {
            bool matchesUser = false;
            
            // Пробуем найти по ID
            bool ok;
            int filterUserId = userFilterStr.toInt(&ok);
            if (ok && order.getUserId() == filterUserId) {
                matchesUser = true;
            }
            
            // Пробуем найти по логину
            if (!matchesUser) {
                User *user = dm.getUserById(order.getUserId());
                if (user && user->getUsername().contains(userFilterStr, Qt::CaseInsensitive)) {
                    matchesUser = true;
                }
            }
            
            if (!matchesUser) {
                matches = false;
            }
        }
        
        if (matches) {
            filtered.append(order);
        }
    }
    
    // Сохраняем отфильтрованные заказы для экспорта
    m_filteredOrders = filtered;
    
    m_ordersTable->setRowCount(filtered.size());
    
    for (int i = 0; i < filtered.size(); ++i) {
        const Order &order = filtered[i];
        m_ordersTable->setItem(i, 0, new QTableWidgetItem(QString::number(order.getId())));
        m_ordersTable->setItem(i, 1, new QTableWidgetItem(order.getDate().toString("dd.MM.yyyy")));
        m_ordersTable->setItem(i, 2, new QTableWidgetItem(QString::number(order.getUserId())));
        
        // Получаем логин пользователя
        User *user = dm.getUserById(order.getUserId());
        QString username = user ? user->getUsername() : "Неизвестно";
        m_ordersTable->setItem(i, 3, new QTableWidgetItem(username));
        
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
        m_ordersTable->setItem(i, 4, new QTableWidgetItem(mealsStr));
        m_ordersTable->setItem(i, 5, new QTableWidgetItem(QString::number(order.getTotalPrice(), 'f', 2) + " руб."));
    }
    
    // Восстанавливаем минимальную ширину столбца "Блюда" после фильтрации
    m_ordersTable->setColumnWidth(4, 300);
}

void AdminWindow::onGenerateRevenueReport()
{
    DataManager &dm = DataManager::getInstance();
    m_reportManager->setStrategy(new RevenueReportStrategy());
    QString report = m_reportManager->generateReport(dm.getOrders(), dm.getMeals(), dm.getUsers());
    m_reportText->setPlainText(report);
}

void AdminWindow::onGeneratePopularDishesReport()
{
    DataManager &dm = DataManager::getInstance();
    m_reportManager->setStrategy(new PopularDishesReportStrategy());
    QString report = m_reportManager->generateReport(dm.getOrders(), dm.getMeals(), dm.getUsers());
    m_reportText->setPlainText(report);
}

void AdminWindow::onGenerateOrdersByDateReport()
{
    DataManager &dm = DataManager::getInstance();
    m_reportManager->setStrategy(new OrdersByDateReportStrategy());
    QString report = m_reportManager->generateReport(dm.getOrders(), dm.getMeals(), dm.getUsers());
    m_reportText->setPlainText(report);
}

void AdminWindow::onExportMenu()
{
    QString filename = QFileDialog::getSaveFileName(this, "Экспорт меню", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        DataManager &dm = DataManager::getInstance();
        if (dm.exportMenu(filename)) {
            QMessageBox::information(this, "Успех", "Меню успешно экспортировано");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось экспортировать меню");
        }
    }
}

void AdminWindow::onExportOrders()
{
    if (m_filteredOrders.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет заказов для экспорта");
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(this, "Экспорт заказов", "", "JSON Files (*.json)");
    if (filename.isEmpty()) {
        return;
    }
    
    DataManager &dm = DataManager::getInstance();
    QJsonArray ordersArray;
    
    for (const Order &order : m_filteredOrders) {
        QJsonObject orderObj;
        orderObj["id"] = order.getId();
        orderObj["userId"] = order.getUserId();
        orderObj["date"] = order.getDate().toString(Qt::ISODate);
        orderObj["totalPrice"] = order.getTotalPrice();
        
        // Добавляем информацию о пользователе
        User *user = dm.getUserById(order.getUserId());
        if (user) {
            QJsonObject userObj;
            userObj["id"] = user->getId();
            userObj["username"] = user->getUsername();
            orderObj["user"] = userObj;
        }
        
        // Добавляем информацию о блюдах
        QJsonArray mealsArray;
        for (const auto &mealPair : order.getMeals()) {
            Meal *meal = dm.getMealById(mealPair.first);
            if (meal) {
                QJsonObject mealObj;
                mealObj["id"] = meal->getId();
                mealObj["name"] = meal->getName();
                mealObj["price"] = meal->getPrice();
                mealObj["quantity"] = mealPair.second;
                
                // Добавляем информацию о категории
                Category *category = dm.getCategoryById(meal->getCategoryId());
                if (category) {
                    QJsonObject categoryObj;
                    categoryObj["id"] = category->getId();
                    categoryObj["name"] = category->getName();
                    mealObj["category"] = categoryObj;
                }
                
                mealsArray.append(mealObj);
            }
        }
        orderObj["meals"] = mealsArray;
        
        ordersArray.append(orderObj);
    }
    
    QJsonObject rootObj;
    rootObj["orders"] = ordersArray;
    rootObj["totalOrders"] = ordersArray.size();
    rootObj["exportDate"] = QDate::currentDate().toString(Qt::ISODate);
    
    QJsonDocument doc(rootObj);
    
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, "Успех", 
            QString("Заказы успешно экспортированы (%1 заказов)").arg(m_filteredOrders.size()));
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
    }
}

void AdminWindow::onImportMenu()
{
    QString filename = QFileDialog::getOpenFileName(this, "Импорт меню", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        DataManager &dm = DataManager::getInstance();
        if (dm.importMenu(filename)) {
            QMessageBox::information(this, "Успех", "Меню успешно импортировано");
            loadCategories();
            refreshMeals();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось импортировать меню");
        }
    }
}

void AdminWindow::onSortMealsChanged()
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

void AdminWindow::onMealCellChanged(int row, int column)
{
    if (m_isLoadingMeals || row < 0) return; // Пропускаем изменения во время загрузки
    
    QTableWidgetItem *idItem = m_mealsTable->item(row, 0);
    if (!idItem) return;
    
    int mealId = idItem->text().toInt();
    DataManager &dm = DataManager::getInstance();
    Meal *meal = dm.getMealById(mealId);
    if (!meal) return;
    
    bool changed = false;
    
    if (column == 1) { // Название
        QTableWidgetItem *nameItem = m_mealsTable->item(row, column);
        if (nameItem) {
            QString newName = nameItem->text().trimmed();
            if (!newName.isEmpty() && newName != meal->getName()) {
                meal->setName(newName);
                changed = true;
            }
        }
    } else if (column == 2) { // Цена
        QTableWidgetItem *priceItem = m_mealsTable->item(row, column);
        if (priceItem) {
            bool ok;
            double newPrice = priceItem->text().toDouble(&ok);
            if (ok && newPrice >= 0 && qAbs(newPrice - meal->getPrice()) > 0.01) {
                meal->setPrice(newPrice);
                changed = true;
            }
        }
    } else if (column == 3) { // Категория
        QTableWidgetItem *catItem = m_mealsTable->item(row, column);
        if (catItem) {
            int newCategoryId = catItem->data(Qt::UserRole).toInt();
            if (newCategoryId > 0 && newCategoryId != meal->getCategoryId()) {
                meal->setCategoryId(newCategoryId);
                changed = true;
            }
        }
    }
    
    if (changed) {
        dm.updateMeal(*meal);
    }
}

bool AdminWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Снимаем выделение с таблицы при клике вне ее и вне формы редактирования
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QWidget *clickedWidget = qApp->widgetAt(mouseEvent->globalPosition().toPoint());
        
        // Проверяем, является ли кликнутый виджет частью таблицы или формы редактирования
        bool isPartOfTableOrForm = false;
        
        // Список виджетов формы редактирования
        QList<QWidget*> formWidgets = {
            m_mealsTable,
            m_mealNameEdit,
            m_mealPriceSpin,
            m_mealCategoryCombo,
            m_mealImagePathEdit,
            m_selectImageButton,
            m_imagePreview,
            m_addMealButton,
            m_editMealButton,
            m_deleteMealButton
        };
        
        // Проверяем, не является ли кликнутый виджет одним из виджетов формы или их дочерним элементом
        QWidget *parent = clickedWidget;
        while (parent) {
            if (formWidgets.contains(parent)) {
                isPartOfTableOrForm = true;
                break;
            }
            parent = parent->parentWidget();
        }
        
        // Если клик был вне таблицы и формы редактирования, снимаем выделение и очищаем форму
        if (!isPartOfTableOrForm) {
            m_mealsTable->clearSelection();
            // Очистка формы произойдет автоматически через onMealSelectionChanged(),
            // но явно вызываем для гарантии
            clearMealForm();
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}

void AdminWindow::clearMealForm()
{
    m_mealNameEdit->clear();
    m_mealPriceSpin->setValue(0.0);
    m_mealCategoryCombo->setCurrentIndex(0);
    m_mealImagePathEdit->clear();
    m_imagePreview->clear();
}

