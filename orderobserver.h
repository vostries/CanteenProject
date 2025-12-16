#ifndef ORDEROBSERVER_H
#define ORDEROBSERVER_H

#include <QObject>
#include "order.h"

class User;

// Observer pattern - обновление баланса при оформлении заказа
class OrderObserver : public QObject
{
    Q_OBJECT

public:
    OrderObserver(QObject *parent = nullptr);
    
    void notifyOrderPlaced(Order *order, User *user, double totalPrice);

signals:
    void balanceUpdated(int userId, double newBalance);
};

#endif // ORDEROBSERVER_H










