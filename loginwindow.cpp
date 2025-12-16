#include "loginwindow.h"
#include "datamanager.h"
#include <QApplication>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , m_loggedInUser(nullptr)
{
    setupUI();
    setWindowTitle("Вход в систему");
    setModal(true);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    QLabel *titleLabel = new QLabel("Школьная столовая", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(10);
    
    QLabel *usernameLabel = new QLabel("Имя пользователя:", this);
    mainLayout->addWidget(usernameLabel);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("Введите имя пользователя");
    m_usernameEdit->setMinimumHeight(30);
    mainLayout->addWidget(m_usernameEdit);
    
    QLabel *passwordLabel = new QLabel("Пароль:", this);
    mainLayout->addWidget(passwordLabel);
    
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("Введите пароль");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(30);
    mainLayout->addWidget(m_passwordEdit);
    
    mainLayout->addSpacing(10);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_loginButton = new QPushButton("Войти", this);
    m_loginButton->setMinimumHeight(35);
    m_loginButton->setStyleSheet("QPushButton { background-color: #007AFF; color: white; border: none; border-radius: 5px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #0056CC; }"
                                 "QPushButton:pressed { background-color: #004499; }");
    
    m_registerButton = new QPushButton("Регистрация", this);
    m_registerButton->setMinimumHeight(35);
    m_registerButton->setStyleSheet("QPushButton { background-color: #34C759; color: white; border: none; border-radius: 5px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #28A745; }"
                                    "QPushButton:pressed { background-color: #1E7E34; }");
    
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);
    mainLayout->addLayout(buttonLayout);
    
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    
    // Поддержка Enter для входа
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    
    setFixedSize(380, 280);
    setStyleSheet("QDialog { background-color: #F5F5F5; }");
}

void LoginWindow::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }
    
    DataManager &dm = DataManager::getInstance();
    User *user = dm.findUser(username, password);
    
    if (user) {
        m_loggedInUser = user;
        emit loginSuccessful(user);
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
        m_passwordEdit->clear();
    }
}

void LoginWindow::onRegisterClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }
    
    DataManager &dm = DataManager::getInstance();
    
    // Проверяем, существует ли уже такой пользователь
    for (const User &u : dm.getUsers()) {
        if (u.getUsername() == username) {
            QMessageBox::warning(this, "Ошибка", "Пользователь с таким именем уже существует");
            return;
        }
    }
    
    // Создаем нового пользователя-ученика
    User newUser(dm.getNextUserId(), username, password, UserType::Student, 1000.0); // Начальный баланс 1000
    dm.addUser(newUser);
    
    QMessageBox::information(this, "Успех", "Регистрация выполнена успешно. Ваш начальный баланс: 1000 руб.");
    m_passwordEdit->clear();
}

