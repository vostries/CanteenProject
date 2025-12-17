#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QCloseEvent>
#include "user.h"
#include "meal.h"
#include "order.h"
#include "reportmanager.h"
#include "sortstrategy.h"
#include "categorydelegate.h"

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(User *user, QWidget *parent = nullptr);
    ~AdminWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onAddMeal();
    void onEditMeal();
    void onDeleteMeal();
    void onMealSelectionChanged();
    void refreshMeals();
    void refreshOrders();
    void onFilterOrders();
    void onGenerateRevenueReport();
    void onGeneratePopularDishesReport();
    void onGenerateOrdersByDateReport();
    void onExportMenu();
    void onImportMenu();
    void onExportOrders();
    void onSortMealsChanged();
    void onMealCellChanged(int row, int column);

private:
    User *m_user;
    ReportManager *m_reportManager;
    SortStrategy *m_sortStrategy;
    bool m_isLoadingMeals;
    
    QTabWidget *m_tabWidget;
    
    // Tab 1: Управление меню
    QWidget *m_menuTab;
    QTableWidget *m_mealsTable;
    QLineEdit *m_mealNameEdit;
    QDoubleSpinBox *m_mealPriceSpin;
    QComboBox *m_mealCategoryCombo;
    QLineEdit *m_mealImagePathEdit;
    QPushButton *m_selectImageButton;
    QLabel *m_imagePreview;
    QPushButton *m_addMealButton;
    QPushButton *m_editMealButton;
    QPushButton *m_deleteMealButton;
    QPushButton *m_exportMenuButton;
    QPushButton *m_importMenuButton;
    QComboBox *m_sortCombo;
    
    // Tab 2: Заказы
    QWidget *m_ordersTab;
    QTableWidget *m_ordersTable;
    QDateEdit *m_filterDateEdit;
    QLineEdit *m_filterUserEdit;
    QPushButton *m_clearFilterButton;
    QPushButton *m_exportOrdersButton;
    QList<Order> m_filteredOrders;
    
    // Tab 3: Отчеты
    QWidget *m_reportsTab;
    QTextEdit *m_reportText;
    QPushButton *m_revenueReportButton;
    QPushButton *m_popularDishesReportButton;
    QPushButton *m_ordersByDateReportButton;
    
    void setupUI();
    void setupMenuTab();
    void setupOrdersTab();
    void setupReportsTab();
    void loadCategories();
    void loadMeals();
    void loadOrders();
    int getSelectedMealId();
    void clearMealForm();
};

#endif // ADMINWINDOW_H

