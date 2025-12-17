#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QCloseEvent>
#include "user.h"
#include "orderobserver.h"
#include "sortstrategy.h"

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(User *user, QWidget *parent = nullptr);
    ~StudentWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSearchMeals();
    void onFilterByCategory();
    void onAddToCart();
    void onRemoveFromCart();
    void onPlaceOrder();
    void refreshMeals();
    void refreshCart();
    void refreshOrders();
    void onFilterOrders();
    void updateBalance();
    void onBalanceUpdated(int userId, double newBalance);
    void onSortMealsChanged();

private:
    User *m_user;
    OrderObserver *m_orderObserver;
    SortStrategy *m_sortStrategy;
    
    QTabWidget *m_tabWidget;
    
    // Tab 1: Меню и заказ
    QWidget *m_menuTab;
    QTableWidget *m_mealsTable;
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilterCombo;
    QComboBox *m_priceFilterCombo;
    QComboBox *m_sortCombo;
    QPushButton *m_addToCartButton;
    
    QLabel *m_balanceLabel;
    QTableWidget *m_cartTable;
    QPushButton *m_removeFromCartButton;
    QPushButton *m_placeOrderButton;
    QLabel *m_totalLabel;
    
    QWidget *m_ordersTab;
    QTableWidget *m_myOrdersTable;
    QDateEdit *m_filterDateEdit;
    QPushButton *m_clearFilterButton;
    
    QList<QPair<int, int>> m_cart;
    
    void setupUI();
    void setupMenuTab();
    void setupOrdersTab();
    void loadCategories();
    int getSelectedMealId();
    double calculateCartTotal();
};

#endif // STUDENTWINDOW_H

