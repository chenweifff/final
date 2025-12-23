#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include "userinfo.h"  // 包含完整的UserInfo定义

namespace Ui {
class Chat;
}

class Chat : public QMainWindow
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

    void setCurrentUser(const UserInfo& userInfo);

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFriendItemClicked(QListWidgetItem *item);
    void onSendButtonClicked();
    void onSendFileButtonClicked();
    void onReadyRead();
    void onNewConnection();
    void onMenuTriggered();

private:
    void setupNetwork();
    void loadFriendsList();
    void loadChatHistory(int friendId);
    void sendMessage(const QString& message);

private:
    Ui::Chat *ui;
    UserInfo currentUser;  // 现在有完整类型
    int currentFriendId = -1;
    QString currentFriendName;

    // 网络相关
    QUdpSocket *udpSocket = nullptr;
    QTcpSocket *tcpSocket = nullptr;
    QTcpServer *tcpServer = nullptr;

    // 文件传输相关
    QString currentFilePath;
    qint64 fileTotalBytes = 0;
    qint64 bytesWritten = 0;
    QByteArray outBlock;
};

#endif // CHAT_H
