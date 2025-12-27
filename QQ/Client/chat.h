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
#include "userinfo.h"

namespace Ui {
class Chat;
}

// 自定义委托，用于在QListView中显示头像和昵称
class FriendItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FriendItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
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

private:
    void setupNetwork();
    void loadFriendsList(const QList<UserInfo>& friendList);
    void loadChatHistory(int friendId);
    void sendMessage(const QString& message);

private:
    Ui::Chat *ui;
    UserInfo currentUser;
    int currentFriendId = -1;
    QString currentFriendName;

    // Model/View
    QStandardItemModel *friendListModel;
    FriendItemDelegate *friendItemDelegate;

    // 网络相关
    QTcpSocket *m_tcpSocket = nullptr;
    QUdpSocket *udpSocket = nullptr;
    QTcpServer *tcpServer = nullptr;

    // 文件传输相关
    QString currentFilePath;
    qint64 fileTotalBytes = 0;
    qint64 bytesWritten = 0;
    QByteArray outBlock;
};

#endif // CHAT_H
