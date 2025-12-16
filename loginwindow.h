#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "user.h"

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    
    User* getLoggedInUser() const { return m_loggedInUser; }

signals:
    void loginSuccessful(User* user);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    User *m_loggedInUser;
    
    void setupUI();
};

#endif // LOGINWINDOW_H










