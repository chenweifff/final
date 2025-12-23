#include "Login.h"
#include "ui_Login.h"
#include "register.h"
#include "chat.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcpSocket(nullptr)
    , m_serverConnected(false)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("聊天系统登录");

    // 初始化TCP Socket
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
// 修复错误信号连接
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, &MainWindow::onSocketError);
#else
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &MainWindow::onSocketError);
#endif

    // 连接按钮信号和槽函数
    connect(ui->LoginButton, &QPushButton::clicked, this, &MainWindow::onLoginButtonClicked);
    connect(ui->RegisterButton, &QPushButton::clicked, this, &MainWindow::onRegisterButtonClicked);

    // 设置密码输入框为密码模式
    ui->PasswordEdit->setEchoMode(QLineEdit::Password);

    // 尝试连接服务器（静默连接，不显示任何提示）
    connectToServer();
}

MainWindow::~MainWindow()
{
    delete ui;
    disconnectFromServer();
}

void MainWindow::connectToServer()
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        m_tcpSocket->connectToHost(QHostAddress::LocalHost, 1967);
        m_tcpSocket->waitForConnected(1000);
    }
}

void MainWindow::disconnectFromServer()
{
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
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

    // 尝试连接服务器（静默连接，不显示任何提示）
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        disconnectFromServer();
        connectToServer();
    }

    // 发送登录请求到服务器
    QString loginRequest = QString("LOGIN|%1|%2").arg(username).arg(password);
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->write(loginRequest.toUtf8());
        m_tcpSocket->flush();
    }
    // 移除了禁用登录按钮的代码，按钮始终保持可点击状态
}

void MainWindow::onRegisterButtonClicked()
{
    // 创建注册窗口
    Register *registerDialog = new Register(this);

    // 使用独立的TCP连接进行注册
    QTcpSocket *registerSocket = new QTcpSocket(registerDialog);
    registerDialog->setTcpSocket(registerSocket);
    registerDialog->connectToServer();

    // 连接注册成功信号 - 移除弹窗，只执行必要的清理操作
    connect(registerDialog, &Register::registrationSuccess, this, [this]() {
        // 注册成功后，清空登录界面的用户名和密码输入框
        ui->UsernameEdit->clear();
        ui->PasswordEdit->clear();
        // 不在这里弹窗，注册窗口已经弹窗了
    });

    // 显示注册窗口
    registerDialog->exec();

    // 清理
    registerDialog->deleteLater();
}

void MainWindow::onSocketConnected()
{
    m_serverConnected = true;
    // 移除了启用登录按钮的代码，按钮始终保持可点击状态
}

void MainWindow::onSocketReadyRead()
{
    while (m_tcpSocket->canReadLine()) {
        QByteArray data = m_tcpSocket->readLine();
        QString response = QString::fromUtf8(data).trimmed();

        // 解析服务器响应
        QStringList parts = response.split("|");
        if (parts.size() > 0) {
            QString command = parts[0];

            if (command == "LOGIN_SUCCESS" && parts.size() >= 4) {
                // 登录成功
                int userId = parts[1].toInt();
                QString nickname = parts[2];
                QString avatarPath = parts[3];

                // 使用与chat.h中一致的结构体
                UserInfo userInfo;
                userInfo.userId = userId;
                userInfo.username = ui->UsernameEdit->text();
                userInfo.nickname = nickname;
                userInfo.avatarPath = avatarPath;
                userInfo.status = 1;

                // 隐藏登录窗口
                this->hide();

                // 创建并显示聊天窗口
                Chat *chatWindow = new Chat();
                chatWindow->setCurrentUser(userInfo);
                chatWindow->show();

                // 连接聊天窗口关闭信号
                connect(chatWindow, &Chat::windowClosed, this, [this]() {
                    this->show();
                    ui->UsernameEdit->clear();
                    ui->PasswordEdit->clear();
                    // 移除了启用登录按钮的代码，按钮始终保持可点击状态
                });

            } else if (command == "LOGIN_FAIL") {
                // 登录失败
                QString errorMsg = parts.size() > 1 ? parts[1] : "用户名或密码错误";
                QMessageBox::critical(this, "登录失败", errorMsg);
                // 移除了启用登录按钮的代码，按钮始终保持可点击状态
            }
        }
    }
}

void MainWindow::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_serverConnected = false;
}
