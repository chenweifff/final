#include "chat.h"
#include "ui_chat.h"
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QHostAddress>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QAction>

Chat::Chat(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Chat)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("聊天系统");

    // 初始化currentUser
    currentUser.userId = -1;
    currentUser.status = 0;

    // 连接信号槽
    connect(ui->pushButton, &QPushButton::clicked, this, &Chat::onSendButtonClicked);
    connect(ui->sendFileButton, &QPushButton::clicked, this, &Chat::onSendFileButtonClicked);
    connect(ui->friendListWidget, &QListWidget::itemClicked, this, &Chat::onFriendItemClicked);

    // 连接菜单项
    QAction *logoutAction = new QAction("退出登录", this);
    ui->menu->addAction(logoutAction);
    connect(logoutAction, &QAction::triggered, this, &Chat::onMenuTriggered);

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
    ui->usernamelabel->setText(userInfo.nickname);
    loadFriendsList();
}

void Chat::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    event->accept();
}

void Chat::setupNetwork()
{
    // 设置UDP Socket（用于消息传输）
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

    // TODO: 从服务器获取好友列表
    // 目前先显示一个示例好友
    QListWidgetItem *item = new QListWidgetItem("测试好友", ui->friendListWidget);
    item->setData(Qt::UserRole, 2); // 假设好友ID为2

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
}

void Chat::loadChatHistory(int friendId)
{
    ui->messageBrowser->clear();

    // TODO: 从服务器获取聊天历史
    // 目前显示示例消息
    QString exampleHtml = "<div align='left' style='margin: 5px;'>"
                          "<div style='background-color: #ffffff; padding: 8px; border-radius: 10px; "
                          "max-width: 70%; display: inline-block;'>"
                          "<span style='color: #666; font-size: 10px;'>10:30</span><br>"
                          "这是一个示例消息"
                          "</div></div>";

    ui->messageBrowser->append(exampleHtml);

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

    // 发送消息（通过UDP）
    sendMessage(message);

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
    // 通过UDP发送消息
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

    // 发送到服务器（假设服务器在localhost:12346）
    udpSocket->writeDatagram(datagram, QHostAddress::LocalHost, 12346);
}

void Chat::onSendFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath());

    if (filePath.isEmpty()) return;

    QMessageBox::information(this, "提示", "文件发送功能需要在服务器端实现");
}

void Chat::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        // TODO: 处理接收到的消息
        // 显示消息到聊天框
    }
}

void Chat::onNewConnection()
{
    // TODO: 处理文件接收
}

void Chat::onMenuTriggered()
{
    this->close();
}
