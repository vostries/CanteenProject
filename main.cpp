#include "loginwindow.h"
#include "adminwindow.h"
#include "studentwindow.h"
#include "user.h"
#include "datamanager.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Инициализация менеджера данных
    DataManager::getInstance();
    
    LoginWindow loginWindow;
    if (loginWindow.exec() == QDialog::Accepted) {
        User *user = loginWindow.getLoggedInUser();
        if (user) {
            if (user->getType() == UserType::Admin) {
                AdminWindow adminWindow(user);
                adminWindow.show();
                return a.exec();
            } else if (user->getType() == UserType::Student) {
                StudentWindow studentWindow(user);
                studentWindow.show();
                return a.exec();
            }
        }
    }
    
    return 0;
}
