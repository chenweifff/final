#include "chat.h"
#include "ui_chat.h"
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QHostAddress>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollBar>

Chat::Chat(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Chat)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("聊天系统");

    // 连接信号槽
    connect(ui->pushButton, &QPushButton::clicked, this, &Chat::onSendButtonClicked);
    connect(ui->sendFileButton, &QPushButton::clicked, this, &Chat::onSendFileButtonClicked);
    connect(ui->friendListWidget, &QListWidget::itemClicked, this, &Chat::onFriendItemClicked);

    // 连接菜单项
    connect(ui->menu->addAction("退出登录"), &QAction::triggered, this, &Chat::onMenuTriggered);

    // 设置网络
    setupNetwork();
}

Chat::~Chat()
{
    delete ui;
    if (udpSocket) udpSocket->deleteLater();
    if (tcpSocket) tcpSocket->deleteLater();
    if (tcpServer) tcpServer->deleteLater();
}

void Chat::setCurrentUser(const UserInfo& userInfo)
{
    currentUser = userInfo;
    this->setWindowTitle(QString("聊天系统 - %1").arg(userInfo.nickname));
    loadFriendsList();
}

void Chat::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    event->accept();
}

void Chat::setupNetwork()
{
    // 设置UDP Socket
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 12345);
    connect(udpSocket, &QUdpSocket::readyRead, this, &Chat::onReadyRead);

    // 设置TCP Server用于接收文件
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::LocalHost, 54321)) {
        qDebug() << "TCP Server启动失败";
    } else {
        connect(tcpServer, &QTcpServer::newConnection, this, &Chat::onNewConnection);
    }
}

void Chat::loadFriendsList()
{
    ui->friendListWidget->clear();

    QList<FriendInfo> friends = DatabaseManager::instance().getFriendsList(currentUser.userId);

    for (const FriendInfo& friendInfo : friends) {
        QListWidgetItem *item = new QListWidgetItem(friendInfo.nickname, ui->friendListWidget);
        item->setData(Qt::UserRole, friendInfo.friendId);

        // 根据在线状态设置颜色
        if (friendInfo.status == 1) {
            item->setForeground(Qt::green);
        } else {
            item->setForeground(Qt::gray);
        }
    }

    // 如果没有好友，显示提示
    if (ui->friendListWidget->count() == 0) {
        QListWidgetItem *item = new QListWidgetItem("暂无好友", ui->friendListWidget);
        item->setFlags(Qt::NoItemFlags);
    }
}

void Chat::onFriendItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int friendId = item->data(Qt::UserRole).toInt();
    if (friendId <= 0) return; // 跳过提示项

    currentFriendId = friendId;
    currentFriendName = item->text();

    ui->friendNameLabel->setText(currentFriendName);

    // 加载聊天历史
    loadChatHistory(friendId);

    // 标记消息为已读
    DatabaseManager::instance().markMessagesAsRead(currentUser.userId, friendId);
}

void Chat::loadChatHistory(int friendId)
{
    ui->messageBrowser->clear();

    QList<MessageInfo> messages = DatabaseManager::instance().getChatHistory(currentUser.userId, friendId);

    for (const MessageInfo& message : messages) {
        QString timeStr = QDateTime::fromString(message.sendTime, "yyyy-MM-dd HH:mm:ss")
        .toString("HH:mm");

        QString messageHtml;
        if (message.senderId == currentUser.userId) {
            // 自己发送的消息，靠右显示
            messageHtml = QString("<div align='right' style='margin: 5px;'>"
                                  "<div style='background-color: #95ec69; padding: 8px; border-radius: 10px; "
                                  "max-width: 70%%; display: inline-block;'>"
                                  "<span style='color: #666; font-size: 10px;'>%1</span><br>"
                                  "%2"
                                  "</div></div>")
                              .arg(timeStr, message.content);
        } else {
            // 好友发送的消息，靠左显示
            messageHtml = QString("<div align='left' style='margin: 5px;'>"
                                  "<div style='background-color: #ffffff; padding: 8px; border-radius: 10px; "
                                  "max-width: 70%%; display: inline-block;'>"
                                  "<span style='color: #666; font-size: 10px;'>%1</span><br>"
                                  "%2"
                                  "</div></div>")
                              .arg(timeStr, message.content);
        }

        ui->messageBrowser->append(messageHtml);
    }

    // 滚动到底部
    ui->messageBrowser->verticalScrollBar()->setValue(
        ui->messageBrowser->verticalScrollBar()->maximum()
        );
}

void Chat::onSendButtonClicked()
{
    QString message = ui->messageEdit->toPlainText().trimmed();

    if (message.isEmpty() || currentFriendId <= 0) {
        return;
    }

    // 发送消息
    sendMessage(message);

    // 保存到数据库
    saveMessageToDatabase(currentFriendId, message);

    // 显示到聊天框
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm");
    QString messageHtml = QString("<div align='right' style='margin: 5px;'>"
                                  "<div style='background-color: #95ec69; padding: 8px; border-radius: 10px; "
                                  "max-width: 70%%; display: inline-block;'>"
                                  "<span style='color: #666; font-size: 10px;'>%1</span><br>"
                                  "%2"
                                  "</div></div>")
                              .arg(timeStr, message);

    ui->messageBrowser->append(messageHtml);
    ui->messageEdit->clear();

    // 滚动到底部
    ui->messageBrowser->verticalScrollBar()->setValue(
        ui->messageBrowser->verticalScrollBar()->maximum()
        );
}

void Chat::sendMessage(const QString& message)
{
    // 这里实现UDP消息发送
    if (!udpSocket) return;

    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    // 构造消息格式：senderId|receiverId|message
    QString msgData = QString("%1|%2|%3")
                          .arg(currentUser.userId)
                          .arg(currentFriendId)
                          .arg(message);

    out << msgData;

    // 发送到服务器（这里简单假设服务器在localhost:12346）
    udpSocket->writeDatagram(datagram, QHostAddress::LocalHost, 12346);
}

void Chat::saveMessageToDatabase(int friendId, const QString& message)
{
    DatabaseManager::instance().saveMessage(currentUser.userId, friendId, message);
}

void Chat::onSendFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath());

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件！");
        return;
    }

    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();
    qint64 fileSize = fileInfo.size();

    // 创建TCP连接
    tcpSocket = new QTcpSocket(this);
    tcpSocket->connectToHost(QHostAddress::LocalHost, 54321);

    if (!tcpSocket->waitForConnected(3000)) {
        QMessageBox::warning(this, "错误", "无法连接到服务器！");
        return;
    }

    // 发送文件头信息
    QByteArray header;
    QDataStream out(&header, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << qint64(0) << qint64(0) << fileName << currentUser.userId << currentFriendId;

    // 发送实际数据
    fileTotalBytes = fileSize + header.size();
    out.device()->seek(0);
    out << fileTotalBytes << qint64(header.size());

    bytesWritten = tcpSocket->write(header);

    // 发送文件内容
    while (!file.atEnd()) {
        outBlock = file.read(1024 * 4); // 4KB
        bytesWritten += tcpSocket->write(outBlock);
    }

    file.close();

    QMessageBox::information(this, "成功", QString("文件 '%1' 已发送").arg(fileName));
}

void Chat::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        QDataStream in(&datagram, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15);

        QString msgData;
        in >> msgData;

        // 解析消息：senderId|receiverId|message
        QStringList parts = msgData.split("|");
        if (parts.size() >= 3) {
            int senderId = parts[0].toInt();
            int receiverId = parts[1].toInt();
            QString message = parts[2];

            // 如果消息是发给当前用户的
            if (receiverId == currentUser.userId) {
                // 保存到数据库
                DatabaseManager::instance().saveMessage(senderId, currentUser.userId, message);

                // 如果正在和发送者聊天，显示消息
                if (senderId == currentFriendId) {
                    QString timeStr = QDateTime::currentDateTime().toString("HH:mm");
                    QString messageHtml = QString("<div align='left' style='margin: 5px;'>"
                                                  "<div style='background-color: #ffffff; padding: 8px; border-radius: 10px; "
                                                  "max-width: 70%%; display: inline-block;'>"
                                                  "<span style='color: #666; font-size: 10px;'>%1</span><br>"
                                                  "%2"
                                                  "</div></div>")
                                              .arg(timeStr, message);

                    ui->messageBrowser->append(messageHtml);

                    // 滚动到底部
                    ui->messageBrowser->verticalScrollBar()->setValue(
                        ui->messageBrowser->verticalScrollBar()->maximum()
                        );
                }
            }
        }
    }
}

void Chat::onNewConnection()
{
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
        // 处理文件接收
        QDataStream in(clientSocket);
        in.setVersion(QDataStream::Qt_5_15);

        if (fileTotalBytes == 0) {
            if (clientSocket->bytesAvailable() < sizeof(qint64) * 2) return;

            in >> fileTotalBytes >> bytesWritten;
        }

        if (clientSocket->bytesAvailable() < fileTotalBytes - sizeof(qint64) * 2) return;

        // 读取文件名和用户信息
        QString fileName;
        int senderId, receiverId;
        in >> fileName >> senderId >> receiverId;

        // 如果是发给当前用户的文件
        if (receiverId == currentUser.userId) {
            // 保存文件
            QString savePath = QDir::homePath() + "/Downloads/" + fileName;
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                // 读取剩余的文件内容
                QByteArray fileData = clientSocket->readAll();
                file.write(fileData);
                file.close();

                QMessageBox::information(this, "文件接收",
                                         QString("收到来自用户 %1 的文件：%2\n已保存到：%3")
                                             .arg(senderId).arg(fileName).arg(savePath));
            }
        }

        clientSocket->close();
        clientSocket->deleteLater();
        fileTotalBytes = 0;
        bytesWritten = 0;
    });
}

void Chat::onMenuTriggered()
{
    this->close();
}
