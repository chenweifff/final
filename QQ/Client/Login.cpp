#include "Login.h"
#include "ui_Login.h"

              MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
, ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("聊天系统登录");

    // 连接按钮信号和槽函数
    connect(ui->LoginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
    connect(ui->RegisterButton, &QPushButton::clicked, this, &MainWindow::onRegisterButtonClicked);

    // 连接数据库
    if (!DatabaseManager::instance().connectToDatabase()) {
        QMessageBox::critical(this, "数据库错误", "无法连接到数据库！");
    }

    // 设置密码输入框为密码模式
    ui->PasswordEdit->setEchoMode(QLineEdit::Password);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (chatWindow) {
        delete chatWindow;
    }
}

void MainWindow::onLoginButtonClicked()
{
    QString username = ui->UsernameEdit->text().trimmed();
    QString password = ui->PasswordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
        return;
    }

    UserInfo userInfo;
    if (DatabaseManager::instance().authenticateUser(username, password, userInfo)) {
        // 登录成功，隐藏登录窗口，不显示成功弹窗
        this->hide();

        // 创建并显示聊天窗口，传递用户信息
        chatWindow = new Chat(this);
        chatWindow->setCurrentUser(userInfo);
        chatWindow->show();

        // 连接聊天窗口关闭信号
        connect(chatWindow, &Chat::windowClosed, this, [this]() {
            this->show();
            delete chatWindow;
            chatWindow = nullptr;

            // 清空登录信息
            ui->UsernameEdit->clear();
            ui->PasswordEdit->clear();
        });

    } else {
        QMessageBox::critical(this, "登录失败", "用户名或密码错误！");
    }
}

void MainWindow::onRegisterButtonClicked()
{
    // 简单实现注册功能
    QString username = ui->UsernameEdit->text().trimmed();
    QString password = ui->PasswordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
        return;
    }


    QMessageBox::information(this, "注册", "注册功能需要连接到服务器实现！");
}
