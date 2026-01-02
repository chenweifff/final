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
#include <QTextDocument>
#include <QDir>
#include <QSettings>

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

    // 绘制头像区域（缩小为36x36）
    QRect avatarRect = option.rect;
    avatarRect.setWidth(36);
    avatarRect.setHeight(36);
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
            painter->drawPixmap(avatarRect, avatar.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // 加载失败，使用默认头像
            painter->fillRect(avatarRect, QColor(100, 149, 237));
            painter->setPen(Qt::white);
            painter->setFont(QFont("Arial", 14, QFont::Bold));
            painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());
        }
    } else {
        // 使用默认头像
        painter->fillRect(avatarRect, QColor(100, 149, 237));
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 14, QFont::Bold));
        painter->drawText(avatarRect, Qt::AlignCenter, nickname.left(1).toUpper());
    }

    painter->setClipping(false);

    // 绘制在线状态指示器
    QColor statusColor = (status == 1) ? QColor(0, 200, 0) : QColor(150, 150, 150);
    painter->setBrush(statusColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(avatarRect.right() - 8, avatarRect.bottom() - 8, 8, 8);

    // 绘制昵称
    QRect textRect = option.rect;
    textRect.setLeft(avatarRect.right() + 12);
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

    // 加载CSS样式
    loadCSSStyles();  // 确保调用这个函数

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

void Chat::loadCSSStyles()
{
    QString cssPath = "E:/qt/final/QQ/css/chat.css";
    QFile cssFile(cssPath);

    if (cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QString::fromUtf8(cssFile.readAll());
        // 将样式表设置为文档的默认样式
        ui->messageBrowser->document()->setDefaultStyleSheet(styleSheet);
        cssFile.close();
    }

    // 保持 QTextBrowser 的背景样式（不影响 document 内部样式）
    ui->messageBrowser->setStyleSheet(
        "QTextBrowser { background-color: #f5f5f5; border: none; }"
        );
}

void Chat::displayMessage(const MessageInfo& message)
{
    bool isMy = (message.senderId == currentUser.userId);

    /* ===== 头像 ===== */
    QString avatarPath;
    if (isMy) {
        avatarPath = currentUser.avatarPath;
    } else if (m_friendMap.contains(message.senderId)) {
        avatarPath = m_friendMap[message.senderId].avatarPath;
    }

    QString avatarHtml;
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QImage img(avatarPath);
        img = img.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");

        avatarHtml = QString(
                         "<div class='avatar'>"
                         "<img class='avatar-img' src='data:image/png;base64,%1'>"
                         "</div>"
                         ).arg(QString::fromLatin1(bytes.toBase64()));
    } else {
        QString initial = isMy
                              ? currentUser.nickname.left(1)
                              : m_friendMap.value(message.senderId).nickname.left(1);

        avatarHtml = QString("<div class='avatar'>%1</div>").arg(initial);
    }

    /* ===== 消息内容 ===== */
    QString bubbleClass = isMy ? "bubble my-bubble" : "bubble other-bubble";

    QString bubbleHtml = QString(
                             "<div class='%1'>%2</div>"
                             ).arg(
                                 bubbleClass,
                                 message.content.toHtmlEscaped().replace("\n", "<br>")
                                 );

    /* ===== 表格结构（核心） ===== */
    QString tableHtml;

    if (isMy) {
        tableHtml = QString(
                        "<table class='message-table'><tr>"
                        "<td class='spacer'></td>"
                        "<td class='cell'>%1</td>"
                        "<td class='avatar-cell'>%2</td>"
                        "</tr></table>"
                        ).arg(bubbleHtml, avatarHtml);
    } else {
        tableHtml = QString(
                        "<table class='message-table'><tr>"
                        "<td class='avatar-cell'>%1</td>"
                        "<td class='cell'>%2</td>"
                        "<td class='spacer'></td>"
                        "</tr></table>"
                        ).arg(avatarHtml, bubbleHtml);
    }

    QString rowClass = isMy
                           ? "message-row my-row"
                           : "message-row other-row";

    QString messageHtml =
        QString("<div class='%1'>%2</div>").arg(rowClass, tableHtml);

    /* ===== 插入 ===== */
    QString html = ui->messageBrowser->toHtml();
    if (!html.contains("<body")) {
        html = "<html><body></body></html>";
    }

    int pos = html.lastIndexOf("</body>");
    html.insert(pos, messageHtml);

    ui->messageBrowser->setHtml(html);

    QTimer::singleShot(0, this, [this]() {
        auto *bar = ui->messageBrowser->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
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
    m_friendMap.clear(); // 清空好友映射

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

            // 添加到好友映射
            m_friendMap.insert(friendInfo.userId, friendInfo);

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
    qDebug() << "好友列表加载完成，好友映射大小：" << m_friendMap.size();
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

    // 清空聊天记录缓存
    chatHistory.clear();

    // 加载聊天历史
    requestChatHistory(friendId);
}

void Chat::requestChatHistory(int friendId)
{
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString request = QString("GET_MESSAGES|%1|%2\n")
        .arg(currentUser.userId)
            .arg(friendId);
        m_tcpSocket->write(request.toUtf8());
        m_tcpSocket->flush();
        qDebug() << "已发送聊天记录请求：" << request.trimmed();

        // 先显示系统消息
        addSystemMessage("正在加载聊天记录...");
    } else {
        qDebug() << "TCP连接不可用，无法请求聊天记录";
        addSystemMessage("网络连接异常，无法加载聊天记录");
    }
}

void Chat::addMessageToUI(const MessageInfo& message)
{
    // 检查是否已经在聊天记录中
    for (const auto& msg : chatHistory) {
        if (msg.messageId == message.messageId) {
            return; // 消息已存在，不重复添加
        }
    }

    // 添加到聊天记录
    chatHistory.append(message);

    // 显示消息
    displayMessage(message);
}

void Chat::addSystemMessage(const QString& content)
{
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm");

    // 使用CSS类来设置系统消息样式
    QString messageHtml = QString("<div class='system-message'>"
                                  "%1 系统消息: %2"
                                  "</div>")
                              .arg(timeStr, content);

    QString currentHtml = ui->messageBrowser->toHtml();
    ui->messageBrowser->setHtml(currentHtml + messageHtml);
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

    // 创建消息对象
    MessageInfo newMessage;
    newMessage.senderId = currentUser.userId;
    newMessage.receiverId = currentFriendId;
    newMessage.content = message;
    newMessage.contentType = 1; // 文本消息
    newMessage.sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 添加到聊天记录并显示
    addMessageToUI(newMessage);

    // 清空输入框
    ui->messageEdit->clear();
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

    // 同时通过TCP发送到服务器保存到数据库
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString saveRequest = QString("SAVE_MESSAGE|%1|%2|1|%3\n")
        .arg(currentUser.userId)
            .arg(currentFriendId)
            .arg(message);
        m_tcpSocket->write(saveRequest.toUtf8());
        m_tcpSocket->flush();
    }
}

void Chat::onSendFileButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath());

    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    qint64 fileSize = fileInfo.size();

    // 创建文件消息
    MessageInfo fileMessage;
    fileMessage.senderId = currentUser.userId;
    fileMessage.receiverId = currentFriendId;
    fileMessage.content = QString("文件: %1").arg(fileName);
    fileMessage.contentType = 2; // 文件消息
    fileMessage.fileName = fileName;
    fileMessage.fileSize = fileSize;
    fileMessage.sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 添加到聊天记录并显示
    addMessageToUI(fileMessage);

    // 通过TCP发送文件消息到服务器保存
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString saveRequest = QString("SAVE_MESSAGE|%1|%2|2|%3|%4|%5\n")
        .arg(currentUser.userId)
            .arg(currentFriendId)
            .arg(fileName)
            .arg(fileSize)
            .arg(filePath);
        m_tcpSocket->write(saveRequest.toUtf8());
        m_tcpSocket->flush();
    }

    QMessageBox::information(this, "提示", QString("已选择文件：%1").arg(filePath));
}

void Chat::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        // 解析UDP消息格式：senderId|receiverId|message
        QString msgData = QString::fromUtf8(datagram);
        QStringList parts = msgData.split("|");

        if (parts.size() >= 3) {
            int senderId = parts[0].toInt();
            int receiverId = parts[1].toInt();
            QString message = parts[2];

            // 如果当前显示的是发送者的聊天窗口，则显示消息
            if (currentFriendId == senderId || (currentFriendId == receiverId && receiverId == currentUser.userId)) {
                // 创建消息对象
                MessageInfo newMessage;
                newMessage.senderId = senderId;
                newMessage.receiverId = receiverId;
                newMessage.content = message;
                newMessage.contentType = 1; // 文本消息
                newMessage.sendTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

                // 添加到聊天记录并显示
                addMessageToUI(newMessage);
            }
        }
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
            } else if (command == "MESSAGES_LIST") {
                // 处理聊天记录响应
                int messageCount = parts[1].toInt();
                qDebug() << "收到聊天记录，数量：" << messageCount;

                // 清空当前显示
                ui->messageBrowser->clear();

                if (messageCount == 0) {
                    addSystemMessage("暂无聊天记录");
                    return;
                }

                int index = 2;
                for (int i = 0; i < messageCount; i++) {
                    if (index + 6 < parts.size()) {  // 确保有足够的数据
                        MessageInfo message;
                        message.messageId = parts[index++].toInt();
                        message.senderId = parts[index++].toInt();
                        message.receiverId = parts[index++].toInt();
                        message.contentType = parts[index++].toInt();
                        message.content = parts[index++];
                        message.fileName = parts[index++];
                        message.fileSize = parts[index++].toLongLong();
                        message.sendTime = parts[index++];

                        // 添加到聊天记录并显示
                        addMessageToUI(message);
                        qDebug() << "解析消息：" << message.content;
                    } else {
                        qDebug() << "数据不完整，跳过剩余消息";
                        break;
                    }
                }

                addSystemMessage("聊天记录加载完成");
            } else if (command == "MESSAGE_SAVED") {
                qDebug() << "消息保存成功";
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
