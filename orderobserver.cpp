#include "orderobserver.h"
#include "user.h"

OrderObserver::OrderObserver(QObject *parent)
    : QObject(parent)
{
}

void OrderObserver::notifyOrderPlaced(Order *order, User *user, double totalPrice)
{
    if (user && user->deductBalance(totalPrice)) {
        emit balanceUpdated(user->getId(), user->getBalance());
    }
}










