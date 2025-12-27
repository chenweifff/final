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

    emit logMessage(QString("收到客户端消息: %1").arg(message));

    // 解析消息格式：命令|参数1|参数2|...
    QStringList parts = message.split("|");
    if (parts.size() > 0) {
        QString command = parts[0];

        if (command == "LOGIN" && parts.size() == 3) {
            QString username = parts[1];
            QString password = parts[2];

            emit logMessage(QString("收到登录请求: 用户名=%1").arg(username));

            handleLoginRequest(client, username, password);
        } else if (command == "REGISTER" && parts.size() >= 4) {
            QString username = parts[1];
            QString password = parts[2];
            QString nickname = parts[3];
            QString avatarPath = parts.size() > 4 ? parts[4] : "default_avatar.png";

            emit logMessage(QString("收到注册请求: 用户名=%1, 昵称=%2").arg(username).arg(nickname));

            handleRegisterRequest(client, username, password, nickname, avatarPath);
        } else if (command == "GET_FRIENDS" && parts.size() == 2) {
            int userId = parts[1].toInt();
            emit logMessage(QString("收到好友列表请求: 用户ID=%1").arg(userId));
            handleFriendListRequest(client, userId);
        } else if (command == "LOGOUT" && parts.size() == 2) {
            int userId = parts[1].toInt();
            handleLogoutRequest(client, userId);
        } else if (command == "GET_MESSAGES" && parts.size() == 3) {
            int user1Id = parts[1].toInt();
            int user2Id = parts[2].toInt();
            emit logMessage(QString("收到聊天记录请求: 用户1=%1, 用户2=%2").arg(user1Id).arg(user2Id));
            handleMessageListRequest(client, user1Id, user2Id);
        } else if (command == "SAVE_MESSAGE" && parts.size() >= 4) {
            int senderId = parts[1].toInt();
            int receiverId = parts[2].toInt();
            int contentType = parts[3].toInt();

            if (contentType == 1 && parts.size() >= 5) {
                // 文本消息
                QString content = parts[4];
                emit logMessage(QString("收到保存消息请求: 发送者=%1, 接收者=%2, 内容=%3")
                                    .arg(senderId).arg(receiverId).arg(content));
                handleSaveMessageRequest(client, senderId, receiverId, contentType, content);
            } else if (contentType == 2 && parts.size() >= 7) {
                // 文件消息
                QString fileName = parts[4];
                qint64 fileSize = parts[5].toLongLong();
                QString filePath = parts[6];
                QString content = QString("文件: %1").arg(fileName);
                emit logMessage(QString("收到保存文件消息请求: 发送者=%1, 接收者=%2, 文件名=%3")
                                    .arg(senderId).arg(receiverId).arg(fileName));
                handleSaveMessageRequest(client, senderId, receiverId, contentType, content, fileName, fileSize);
            }
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
        QString response = QString("LOGIN_SUCCESS|%1|%2|%3|%4|%5")
                               .arg(QString::number(userInfo.userId))
                               .arg(userInfo.username)
                               .arg(userInfo.nickname)
                               .arg(userInfo.avatarPath)
                               .arg(userInfo.status);
        sendResponse(client, response);

        emit logMessage(QString("用户 '%1'(ID:%2) 登录成功").arg(userInfo.nickname).arg(userInfo.userId));
        emit userLoginSuccess(userInfo.nickname);

        // 等待客户端请求好友列表（由客户端主动请求）
    } else {
        // 登录失败
        sendResponse(client, "LOGIN_FAIL|用户名或密码错误");
        emit logMessage(QString("登录失败: 用户名=%1").arg(username));
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
        QString successMsg = QString("用户注册成功: 用户名=%1, 昵称=%2")
                                 .arg(username)
                                 .arg(nickname);
        emit logMessage(successMsg);
        emit userRegisterSuccess(username, nickname);
    } else {
        // 注册失败
        sendResponse(client, "REGISTER_FAIL|注册失败，用户名可能已存在");
        emit userRegisterFailed("注册失败，用户名可能已存在");
    }
}

void ChatServer::handleFriendListRequest(QTcpSocket* client, int userId)
{
    if (!m_dbManager) {
        sendResponse(client, "FRIEND_LIST|0|数据库未连接");
        return;
    }

    QList<UserInfo> friendList = m_dbManager->getFriendList(userId);
    emit logMessage(QString("为用户ID=%1查询好友列表，找到%2个好友").arg(userId).arg(friendList.size()));

    sendFriendList(client, userId, friendList);
}

void ChatServer::handleLogoutRequest(QTcpSocket* client, int userId)
{
    if (m_dbManager) {
        m_dbManager->updateUserStatus(userId, 0);
        emit logMessage(QString("用户ID=%1已退出").arg(userId));
    }
    sendResponse(client, "LOGOUT_SUCCESS");
}

void ChatServer::handleMessageListRequest(QTcpSocket* client, int user1Id, int user2Id)
{
    if (!m_dbManager) {
        sendResponse(client, "MESSAGES_LIST|0|数据库未连接");
        return;
    }

    QList<MessageInfo> messageList = m_dbManager->getMessageList(user1Id, user2Id);
    emit logMessage(QString("为用户ID=%1和%2查询聊天记录，找到%3条消息")
                        .arg(user1Id).arg(user2Id).arg(messageList.size()));

    sendMessageList(client, user1Id, user2Id, messageList);
}

void ChatServer::handleSaveMessageRequest(QTcpSocket* client, int senderId, int receiverId,
                                          int contentType, const QString& content,
                                          const QString& fileName, qint64 fileSize)
{
    if (!m_dbManager) {
        sendResponse(client, "MESSAGE_SAVED|FAIL|数据库未连接");
        return;
    }

    if (m_dbManager->saveMessage(senderId, receiverId, contentType, content, fileName, fileSize)) {
        sendResponse(client, "MESSAGE_SAVED|SUCCESS");
        emit logMessage(QString("消息保存成功: 发送者=%1, 接收者=%2").arg(senderId).arg(receiverId));
    } else {
        sendResponse(client, "MESSAGE_SAVED|FAIL|保存失败");
        emit logMessage(QString("消息保存失败: 发送者=%1, 接收者=%2").arg(senderId).arg(receiverId));
    }
}

void ChatServer::sendFriendList(QTcpSocket* client, int userId, const QList<UserInfo>& friendList)
{
    QString response = QString("FRIEND_LIST|%1").arg(friendList.size());

    for (const UserInfo& friendInfo : friendList) {
        response += QString("|%1|%2|%3|%4|%5")
        .arg(friendInfo.userId)
            .arg(friendInfo.username)
            .arg(friendInfo.nickname)
            .arg(friendInfo.avatarPath)
            .arg(friendInfo.status);
    }

    sendResponse(client, response);
    emit logMessage(QString("已向用户ID=%1发送好友列表，共%2个好友").arg(userId).arg(friendList.size()));
}

void ChatServer::sendMessageList(QTcpSocket* client, int user1Id, int user2Id, const QList<MessageInfo>& messageList)
{
    QString response = QString("MESSAGES_LIST|%1").arg(messageList.size());

    for (const MessageInfo& message : messageList) {
        response += QString("|%1|%2|%3|%4|%5|%6|%7|%8")
        .arg(message.messageId)
            .arg(message.senderId)
            .arg(message.receiverId)
            .arg(message.contentType)
            .arg(message.content)
            .arg(message.fileName)
            .arg(message.fileSize)
            .arg(message.sendTime);
    }

    sendResponse(client, response);
    emit logMessage(QString("已向用户ID=%1发送聊天记录，共%2条消息").arg(user1Id).arg(messageList.size()));
}

void ChatServer::sendResponse(QTcpSocket* client, const QString& response)
{
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = (response + "\n").toUtf8();
        client->write(data);
        client->flush();
        emit logMessage(QString("发送响应: %1").arg(response));
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

    // 自动滚动到底部
    QTextCursor cursor = ui->logEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->logEdit->setTextCursor(cursor);
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
