#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QHostAddress>

// ChatServer 实现
ChatServer::ChatServer(QObject *parent)
    : QTcpServer(parent)
    , m_dbManager(nullptr)
{
}

ChatServer::~ChatServer()
{
    stopServer();
}

void ChatServer::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
}

void ChatServer::stopServer()
{
    // 关闭所有客户端连接
    for (QTcpSocket *client : clients) {
        client->disconnectFromHost();
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->waitForDisconnected();
        }
        client->deleteLater();
    }
    clients.clear();

    // 停止监听
    if (isListening()) {
        close();
    }
}

bool ChatServer::startServer(quint16 port)
{
    if (isListening()) {
        return true;
    }

    return listen(QHostAddress::Any, port);
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    clients.append(client);

    connect(client, &QTcpSocket::readyRead, this, &ChatServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &ChatServer::onClientDisconnected);

    QString message = QString("客户端已连接: %1:%2")
                          .arg(client->peerAddress().toString())
                          .arg(client->peerPort());
    emit logMessage(message);
}

void ChatServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        QString message = QString("客户端已断开: %1:%2")
                              .arg(client->peerAddress().toString())
                              .arg(client->peerPort());
        emit logMessage(message);
        clients.removeOne(client);
        client->deleteLater();
    }
}

void ChatServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray data = client->readAll();
    QString message = QString::fromUtf8(data).trimmed();

    // 解析消息格式：命令|参数1|参数2|...
    QStringList parts = message.split("|");
    if (parts.size() > 0) {
        QString command = parts[0];

        if (command == "LOGIN" && parts.size() == 3) {
            QString username = parts[1];
            QString password = parts[2];

            emit logMessage(QString("收到登录请求: 用户名=%1").arg(username));
            emit logMessage("查询数据库中...");

            handleLoginRequest(client, username, password);
        } else if (command == "REGISTER" && parts.size() >= 4) {
            QString username = parts[1];
            QString password = parts[2];
            QString nickname = parts[3];
            QString avatarPath = parts.size() > 4 ? parts[4] : "default_avatar.png";

            emit logMessage(QString("收到注册请求: 用户名=%1, 昵称=%2").arg(username).arg(nickname));
            emit logMessage("有新用户注册中...");

            handleRegisterRequest(client, username, password, nickname, avatarPath);
        }
    }
}

void ChatServer::handleLoginRequest(QTcpSocket* client, const QString& username, const QString& password)
{
    if (!m_dbManager) {
        sendResponse(client, "LOGIN_FAIL|数据库未连接");
        emit logMessage("登录失败: 数据库未连接");
        return;
    }

    UserInfo userInfo;
    if (m_dbManager->authenticateUser(username, password, userInfo)) {
        // 登录成功
        QString response = QString("LOGIN_SUCCESS|%1|%2|%3")
                               .arg(QString::number(userInfo.userId))
                               .arg(userInfo.nickname)
                               .arg(userInfo.avatarPath);
        sendResponse(client, response);

        emit userLoginSuccess(userInfo.nickname);
    } else {
        // 登录失败
        sendResponse(client, "LOGIN_FAIL|用户名或密码错误");
        emit userLoginFailed();
    }
}

void ChatServer::handleRegisterRequest(QTcpSocket* client, const QString& username, const QString& password,
                                       const QString& nickname, const QString& avatarPath)
{
    if (!m_dbManager) {
        sendResponse(client, "REGISTER_FAIL|数据库未连接");
        emit userRegisterFailed("数据库未连接");
        return;
    }

    // 调用DatabaseManager的registerUser函数
    if (m_dbManager->registerUser(username, password, nickname, avatarPath)) {
        // 注册成功
        sendResponse(client, "REGISTER_SUCCESS");

        // 记录注册成功信息
        QString successMsg = QString("用户注册成功: 用户名=%1, 密码=%2, 昵称=%3")
                                 .arg(username)
                                 .arg(password)
                                 .arg(nickname);
        emit logMessage(successMsg);
        emit userRegisterSuccess(username, nickname);
    } else {
        // 注册失败
        sendResponse(client, "REGISTER_FAIL|注册失败，用户名可能已存在");
        emit userRegisterFailed("注册失败，用户名可能已存在");
    }
}

void ChatServer::sendResponse(QTcpSocket* client, const QString& response)
{
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = (response + "\n").toUtf8();
        client->write(data);
        client->flush();
    }
}

// MainWindow 实现
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chatServer(new ChatServer(this))
    , m_dbManager(&DatabaseManager::instance())
{
    ui->setupUi(this);

    // 设置数据库管理器
    m_chatServer->setDatabaseManager(m_dbManager);

    // 连接信号
    connect(m_chatServer, &ChatServer::logMessage, this, &MainWindow::logMessage);
    connect(m_chatServer, &ChatServer::userLoginSuccess, this, &MainWindow::onUserLoginSuccess);
    connect(m_chatServer, &ChatServer::userLoginFailed, this, &MainWindow::onUserLoginFailed);
    connect(m_chatServer, &ChatServer::userRegisterSuccess, this, &MainWindow::onUserRegisterSuccess);
    connect(m_chatServer, &ChatServer::userRegisterFailed, this, &MainWindow::onUserRegisterFailed);

    // 设置窗口标题
    setWindowTitle("聊天服务器");

    // 连接数据库
    if (m_dbManager->connectToDatabase()) {
        logMessage("数据库连接成功");
    } else {
        logMessage("数据库连接失败");
        QMessageBox::critical(this, "错误", "无法连接到数据库");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startstopButton_clicked()
{
    if (m_chatServer->isListening()) {
        m_chatServer->stopServer();
        ui->startstopButton->setText("启动服务器");
        logMessage("服务器已经停止");
    } else {
        if (!m_chatServer->startServer(1967)) {
            QMessageBox::critical(this, "错误", "无法启动服务器");
            return;
        }
        logMessage("服务器已经启动，监听端口: 1967");
        logMessage("等待客户端连接...");
        ui->startstopButton->setText("停止服务器");
    }
}

void MainWindow::logMessage(const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("[HH:mm:ss] ");
    ui->logEdit->appendPlainText(timestamp + msg);
}

void MainWindow::onUserLoginSuccess(const QString &nickname)
{
    logMessage(QString("用户 '%1' 登录成功").arg(nickname));
}

void MainWindow::onUserLoginFailed()
{
    logMessage("登录失败");
}

void MainWindow::onUserRegisterSuccess(const QString &username, const QString &nickname)
{
    logMessage(QString("用户注册成功: 用户名='%1', 昵称='%2'").arg(username).arg(nickname));
}

void MainWindow::onUserRegisterFailed(const QString &reason)
{
    logMessage(QString("用户注册失败: %1").arg(reason));
}
