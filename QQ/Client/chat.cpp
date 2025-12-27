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
#include <QStandardItem>
#include <QBuffer>
#include <QApplication>
#include <QStyle>
#include <QPainterPath>
#include <QTimer>

// FriendItemDelegate 实现
FriendItemDelegate::FriendItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FriendItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // 绘制背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(220, 240, 255));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, QColor(240, 245, 250));
    } else {
        painter->fillRect(option.rect, Qt::white);
    }

    // 获取数据
    QString nickname = index.data(Qt::DisplayRole).toString();
    QString avatarPath = index.data(Qt::UserRole + 1).toString();  // 头像路径
    int status = index.data(Qt::UserRole + 2).toInt();  // 在线状态
    int userId = index.data(Qt::UserRole + 3).toInt();  // 用户ID

    // 绘制头像区域
    QRect avatarRect = option.rect;
    avatarRect.setWidth(40);
    avatarRect.setHeight(40);
    avatarRect.moveTop(option.rect.top() + (option.rect.height() - avatarRect.height()) / 2);
    avatarRect.moveLeft(option.rect.left() + 10);

    // 绘制头像（圆形）
    QPainterPath clipPath;
    clipPath.addEllipse(avatarRect);
    painter->setClipPath(clipPath);

    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        // 加载头像图片
        QPixmap avatar(avatarPath);
        if (!avatar.isNull()) {
            painter->drawPixmap(avatarRect, avatar.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // 加载失败，使用默认头像
            painter->fillRect(avatarRect, QColor(100, 149, 237));
            painter->setPen(Qt::white);
            painter->setFont(QFont("Arial", 16, QFont::Bold));
            painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());
        }
    } else {
        // 使用默认头像
        painter->fillRect(avatarRect, QColor(100, 149, 237));
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 16, QFont::Bold));
        painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());
    }

    painter->setClipping(false);

    // 绘制在线状态指示器
    QColor statusColor = (status == 1) ? QColor(0, 200, 0) : QColor(150, 150, 150);
    painter->setBrush(statusColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(avatarRect.right() - 10, avatarRect.bottom() - 10, 10, 10);

    // 绘制昵称
    QRect textRect = option.rect;
    textRect.setLeft(avatarRect.right() + 15);
    textRect.setTop(option.rect.top() + (option.rect.height() - painter->fontMetrics().height()) / 2);
    textRect.setHeight(painter->fontMetrics().height());

    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPointSize(10);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, nickname);

    // 绘制状态文本
    if (status == 1) {
        painter->setPen(QColor(0, 150, 0));
        painter->drawText(textRect.right() - 50, textRect.top(), 50, textRect.height(),
                          Qt::AlignVCenter | Qt::AlignRight, "在线");
    } else {
        painter->setPen(QColor(150, 150, 150));
        painter->drawText(textRect.right() - 50, textRect.top(), 50, textRect.height(),
                          Qt::AlignVCenter | Qt::AlignRight, "离线");
    }

    painter->restore();
}

QSize FriendItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 60);  // 固定高度为60
}

// Chat 实现
Chat::Chat(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Chat)
    , m_tcpSocket(nullptr)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("聊天系统");

    // 初始化currentUser
    currentUser.userId = -1;
    currentUser.status = 0;

    // 初始化Model/View
    friendListModel = new QStandardItemModel(this);
    friendItemDelegate = new FriendItemDelegate(this);

    ui->friendListView->setModel(friendListModel);
    ui->friendListView->setItemDelegate(friendItemDelegate);
    ui->friendListView->setSpacing(2);
    ui->friendListView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置ListView样式
    ui->friendListView->setStyleSheet(
        "QListView {"
        "    background-color: white;"
        "    border: none;"
        "    outline: none;"
        "}"
        "QListView::item {"
        "    border-bottom: 1px solid #f0f0f0;"
        "}"
        "QListView::item:hover {"
        "    background-color: #f5f5f5;"
        "}"
        "QListView::item:selected {"
        "    background-color: #e3f2fd;"
        "    color: black;"
        "}"
        );

    // 连接信号槽
    connect(ui->pushButton, &QPushButton::clicked, this, &Chat::onSendButtonClicked);
    connect(ui->sendFileButton, &QPushButton::clicked, this, &Chat::onSendFileButtonClicked);
    connect(ui->friendListView, &QListView::clicked, this, &Chat::onFriendItemClicked);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &Chat::onSearchTextChanged);

    // 连接菜单项
    QAction *logoutAction = new QAction("退出登录", this);
    ui->menu->addAction(logoutAction);
    connect(logoutAction, &QAction::triggered, this, &Chat::onMenuTriggered);

    // 初始化网络
    setupNetwork();
}

Chat::~Chat()
{
    delete ui;
    if (udpSocket) udpSocket->deleteLater();
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
    }
    if (tcpServer) tcpServer->deleteLater();
}

void Chat::setCurrentUser(const UserInfo& userInfo)
{
    currentUser = userInfo;
    this->setWindowTitle(QString("聊天系统 - %1").arg(userInfo.nickname));
    ui->usernamelabel->setText(userInfo.nickname);

    // 设置用户头像（如果存在）
    if (!userInfo.avatarPath.isEmpty() && QFile::exists(userInfo.avatarPath)) {
        QPixmap avatar(userInfo.avatarPath);
        if (!avatar.isNull()) {
            QPixmap scaledAvatar = avatar.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->usernamelabel->setPixmap(scaledAvatar);
        }
    }

    qDebug() << "设置当前用户：" << userInfo.nickname << "ID:" << userInfo.userId;
}

void Chat::setTcpSocket(QTcpSocket* socket)
{
    if (m_tcpSocket) {
        m_tcpSocket->disconnect(this);
        m_tcpSocket->deleteLater();
    }

    m_tcpSocket = socket;
    if (m_tcpSocket) {
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Chat::onSocketReadyRead);
        connect(m_tcpSocket, &QTcpSocket::connected, this, [this]() {
            qDebug() << "Chat TCP连接已建立";
        });
        connect(m_tcpSocket, &QTcpSocket::disconnected, this, [this]() {
            qDebug() << "Chat TCP连接已断开";
        });

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
            qDebug() << "Chat TCP错误：" << m_tcpSocket->errorString();
        });
#else
        connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this, [this](QAbstractSocket::SocketError error) {
                    qDebug() << "Chat TCP错误：" << m_tcpSocket->errorString();
                });
#endif
    }
}

void Chat::requestFriendList()
{
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString request = QString("GET_FRIENDS|%1\n").arg(currentUser.userId);
        m_tcpSocket->write(request.toUtf8());
        m_tcpSocket->flush();
        qDebug() << "已发送好友列表请求：" << request.trimmed();
    } else {
        qDebug() << "TCP连接不可用，无法请求好友列表";
        if (m_tcpSocket) {
            qDebug() << "Socket状态：" << m_tcpSocket->state();
            qDebug() << "错误信息：" << m_tcpSocket->errorString();
        }
    }
}

void Chat::closeEvent(QCloseEvent *event)
{
    // 发送登出请求
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString logoutRequest = QString("LOGOUT|%1\n").arg(currentUser.userId);
        m_tcpSocket->write(logoutRequest.toUtf8());
        m_tcpSocket->flush();
        m_tcpSocket->waitForBytesWritten(1000);
    }

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
        qDebug() << "TCP Server启动失败：" << tcpServer->errorString();
    } else {
        connect(tcpServer, &QTcpServer::newConnection, this, &Chat::onNewConnection);
    }
}

void Chat::loadFriendsList(const QList<UserInfo>& friendList)
{
    friendListModel->clear();
    qDebug() << "开始加载好友列表，好友数量：" << friendList.size();

    if (friendList.isEmpty()) {
        QStandardItem *noFriendsItem = new QStandardItem("暂无好友");
        noFriendsItem->setEnabled(false);
        noFriendsItem->setTextAlignment(Qt::AlignCenter);
        friendListModel->appendRow(noFriendsItem);
        qDebug() << "没有好友，显示'暂无好友'";
    } else {
        for (const UserInfo& friendInfo : friendList) {
            qDebug() << "添加好友到列表：" << friendInfo.nickname
                     << " ID:" << friendInfo.userId
                     << " 状态:" << friendInfo.status
                     << " 头像:" << friendInfo.avatarPath;

            QStandardItem *friendItem = new QStandardItem(friendInfo.nickname);

            // 设置数据
            friendItem->setData(friendInfo.nickname, Qt::DisplayRole);  // 显示名称
            friendItem->setData(friendInfo.avatarPath, Qt::UserRole + 1);  // 头像路径
            friendItem->setData(friendInfo.status, Qt::UserRole + 2);  // 在线状态
            friendItem->setData(friendInfo.userId, Qt::UserRole + 3);  // 用户ID
            friendItem->setData(friendInfo.username, Qt::UserRole + 4);  // 用户名

            // 设置图标（简化版）
            QPixmap iconPixmap(40, 40);
            if (!friendInfo.avatarPath.isEmpty() && QFile::exists(friendInfo.avatarPath)) {
                QPixmap avatar(friendInfo.avatarPath);
                if (!avatar.isNull()) {
                    iconPixmap = avatar.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                } else {
                    iconPixmap.fill(QColor(100, 149, 237));
                }
            } else {
                iconPixmap.fill(QColor(100, 149, 237));
            }

            friendItem->setIcon(QIcon(iconPixmap));
            friendItem->setSizeHint(QSize(200, 60));  // 设置项大小

            friendListModel->appendRow(friendItem);
        }
    }

    // 更新视图
    ui->friendListView->update();
    qDebug() << "好友列表加载完成";
}

void Chat::onFriendItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qDebug() << "无效的索引";
        return;
    }

    QStandardItem *item = friendListModel->itemFromIndex(index);
    if (!item || !item->isEnabled()) {
        qDebug() << "项目无效或已禁用";
        return;  // 跳过提示项
    }

    int friendId = item->data(Qt::UserRole + 3).toInt();  // 获取用户ID
    QString friendName = item->text();

    if (friendId <= 0) {
        qDebug() << "无效的好友ID";
        return;
    }

    currentFriendId = friendId;
    currentFriendName = friendName;

    ui->friendNameLabel->setText(currentFriendName);
    qDebug() << "选中好友：" << currentFriendName << " ID:" << currentFriendId;

    // 加载聊天历史
    loadChatHistory(friendId);
}

void Chat::loadChatHistory(int friendId)
{
    ui->messageBrowser->clear();

    // TODO: 从服务器获取聊天历史
    // 目前显示示例消息
    QString exampleHtml = "<div style='margin: 10px;'>"
                          "<div style='background-color: #e8f5e9; padding: 10px; border-radius: 10px; "
                          "max-width: 70%; float: left; clear: both;'>"
                          "<span style='color: #666; font-size: 10px;'>10:30 系统</span><br>"
                          "开始与好友聊天"
                          "</div></div>";

    ui->messageBrowser->setHtml(exampleHtml);

    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        ui->messageBrowser->verticalScrollBar()->setValue(
            ui->messageBrowser->verticalScrollBar()->maximum()
            );
    });
}

void Chat::onSendButtonClicked()
{
    QString message = ui->messageEdit->toPlainText().trimmed();

    if (message.isEmpty()) {
        QMessageBox::warning(this, "提示", "消息不能为空");
        return;
    }

    if (currentFriendId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择好友");
        return;
    }

    // 发送消息（通过UDP）
    sendMessage(message);

    // 显示到聊天框
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm");
    QString messageHtml = QString("<div style='margin: 10px;'>"
                                  "<div style='background-color: #dcf8c6; padding: 10px; border-radius: 10px; "
                                  "max-width: 70%%; float: right; clear: both;'>"
                                  "<span style='color: #666; font-size: 10px;'>%1 我</span><br>"
                                  "%2"
                                  "</div></div>")
                              .arg(timeStr, message);

    QString currentHtml = ui->messageBrowser->toHtml();
    ui->messageBrowser->setHtml(currentHtml + messageHtml);
    ui->messageEdit->clear();

    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        ui->messageBrowser->verticalScrollBar()->setValue(
            ui->messageBrowser->verticalScrollBar()->maximum()
            );
    });
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
    qDebug() << "发送消息：" << msgData;
}

void Chat::onSendFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath());

    if (filePath.isEmpty()) return;

    QMessageBox::information(this, "提示", QString("已选择文件：%1\n文件发送功能需要在服务器端实现").arg(filePath));
}

void Chat::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        // TODO: 处理接收到的消息
        qDebug() << "收到UDP消息：" << datagram;

        // 显示消息到聊天框
        QString timeStr = QDateTime::currentDateTime().toString("HH:mm");
        QString messageHtml = QString("<div style='margin: 10px;'>"
                                      "<div style='background-color: #e8f5e9; padding: 10px; border-radius: 10px; "
                                      "max-width: 70%%; float: left; clear: both;'>"
                                      "<span style='color: #666; font-size: 10px;'>%1 好友</span><br>"
                                      "%2"
                                      "</div></div>")
                                  .arg(timeStr, QString::fromUtf8(datagram));

        QString currentHtml = ui->messageBrowser->toHtml();
        ui->messageBrowser->setHtml(currentHtml + messageHtml);
    }
}

void Chat::onSocketReadyRead()
{
    if (!m_tcpSocket) {
        qDebug() << "TCP Socket为空";
        return;
    }

    while (m_tcpSocket->canReadLine()) {
        QByteArray data = m_tcpSocket->readLine();
        QString response = QString::fromUtf8(data).trimmed();
        qDebug() << "Chat收到服务器响应：" << response;

        // 解析服务器响应
        QStringList parts = response.split("|");
        if (parts.size() > 0) {
            QString command = parts[0];

            if (command == "FRIEND_LIST") {
                // 处理好友列表响应
                int friendCount = parts[1].toInt();
                qDebug() << "好友数量：" << friendCount;

                if (friendCount == 0) {
                    qDebug() << "没有好友";
                    loadFriendsList(QList<UserInfo>());
                    return;
                }

                QList<UserInfo> friendList;

                int index = 2;
                for (int i = 0; i < friendCount; i++) {
                    if (index + 4 < parts.size()) {  // 确保有足够的数据
                        UserInfo friendInfo;
                        friendInfo.userId = parts[index++].toInt();
                        friendInfo.username = parts[index++];
                        friendInfo.nickname = parts[index++];
                        friendInfo.avatarPath = parts[index++];
                        friendInfo.status = parts[index++].toInt();

                        friendList.append(friendInfo);
                        qDebug() << "解析好友信息：" << friendInfo.nickname
                                 << " ID:" << friendInfo.userId
                                 << " 头像:" << friendInfo.avatarPath
                                 << " 状态:" << friendInfo.status;
                    } else {
                        qDebug() << "数据不完整，跳过剩余好友";
                        break;
                    }
                }

                // 加载好友列表到界面
                loadFriendsList(friendList);
            } else if (command == "LOGOUT_SUCCESS") {
                qDebug() << "登出成功";
            } else {
                qDebug() << "未知命令：" << command;
            }
        }
    }
}

void Chat::onNewConnection()
{
    // TODO: 处理文件接收
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    if (clientSocket) {
        qDebug() << "新的文件传输连接";
        connect(clientSocket, &QTcpSocket::readyRead, this, [clientSocket]() {
            // 处理文件接收
            QByteArray data = clientSocket->readAll();
            qDebug() << "收到文件数据：" << data.size() << "字节";
        });
    }
}

void Chat::onMenuTriggered()
{
    this->close();
}

void Chat::onSearchTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        // 显示所有好友
        for (int i = 0; i < friendListModel->rowCount(); ++i) {
            friendListModel->item(i)->setEnabled(true);
        }
    } else {
        // 筛选好友
        for (int i = 0; i < friendListModel->rowCount(); ++i) {
            QStandardItem *item = friendListModel->item(i);
            QString nickname = item->text();
            bool match = nickname.contains(text, Qt::CaseInsensitive);
            item->setEnabled(match);
        }
    }
}
