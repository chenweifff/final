#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QBuffer>
#include "userinfo.h"

namespace Ui {
class Chat;
}

class FriendItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FriendItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // 新增：用于处理按钮点击
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void addFriendClicked(int friendId);  // 新增：添加好友信号

private:
    mutable QRect m_addButtonRect;  // 用于记录添加按钮的位置
};

class Chat : public QMainWindow
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

    void setCurrentUser(const UserInfo& userInfo);
    void setTcpSocket(QTcpSocket* socket);
    void requestFriendList();

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFriendItemClicked(const QModelIndex &index);
    void onSendButtonClicked();
    void onSendFileButtonClicked();
    void onReadyRead();
    void onNewConnection();
    void onMenuTriggered();
    void onSocketReadyRead();
    void onSearchTextChanged(const QString &text);
    void onSearchButtonClicked();
    void onAddFriendClicked(int friendId);  // 新增：处理添加好友点击

private:
    void setupNetwork();
    void loadFriendsList(const QList<UserInfo>& friendList);
    void sendMessage(const QString& message);
    void requestChatHistory(int friendId);
    void displayMessage(const MessageInfo& message);
    void addMessageToUI(const MessageInfo& message);
    void addSystemMessage(const QString& content);
    void loadCSSStyles();
    void sendSearchRequest(const QString& keyword);
    void sendAddFriendRequest(int friendId);  // 新增：发送添加好友请求
    void updateFriendList();  // 新增：更新好友列表显示

private:
    Ui::Chat *ui;
    UserInfo currentUser;
    int currentFriendId = -1;
    QString currentFriendName;

    QStandardItemModel *friendListModel;
    FriendItemDelegate *friendItemDelegate;

    QTcpSocket *m_tcpSocket = nullptr;
    QUdpSocket *udpSocket = nullptr;
    QTcpServer *tcpServer = nullptr;

    QList<MessageInfo> chatHistory;
    QMap<int, UserInfo> m_friendMap;

    bool m_isSearchMode = false;
    QList<UserInfo> m_searchResults;

    // 新增：存储搜索结果的友好关系状态
    QMap<int, bool> m_searchResultFriendStatus;  // key: userId, value: 是否是好友
};

#endif // CHAT_H
