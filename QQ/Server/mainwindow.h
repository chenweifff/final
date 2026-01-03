#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "database.h"
#include "userinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    ~ChatServer();

    void stopServer();
    bool startServer(quint16 port = 1967);

    void setDatabaseManager(DatabaseManager* dbManager);

signals:
    void logMessage(const QString &msg);
    void userLoginSuccess(const QString &nickname);
    void userLoginFailed();
    void userRegisterSuccess(const QString &username, const QString &nickname);
    void userRegisterFailed(const QString &reason);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();
    void onReadyRead();

private:
    QList<QTcpSocket*> clients;
    DatabaseManager* m_dbManager;

    // 原有处理函数...
    void handleLoginRequest(QTcpSocket* client, const QString& username, const QString& password);
    void handleRegisterRequest(QTcpSocket* client, const QString& username, const QString& password,
                               const QString& nickname, const QString& avatarPath);
    void handleFriendListRequest(QTcpSocket* client, int userId);
    void handleLogoutRequest(QTcpSocket* client, int userId);
    void handleMessageListRequest(QTcpSocket* client, int user1Id, int user2Id);
    void handleSaveMessageRequest(QTcpSocket* client, int senderId, int receiverId,
                                  int contentType, const QString& content,
                                  const QString& fileName = "", qint64 fileSize = 0);
    void handleSearchUsersRequest(QTcpSocket* client, int userId, const QString& keyword);

    // 新增：处理添加好友请求
    void handleAddFriendRequest(QTcpSocket* client, int userId, int friendId);

    // 发送函数...
    void sendFriendList(QTcpSocket* client, int userId, const QList<UserInfo>& friendList);
    void sendMessageList(QTcpSocket* client, int user1Id, int user2Id, const QList<MessageInfo>& messageList);
    void sendSearchResults(QTcpSocket* client, int userId, const QList<UserInfo>& userList);
    // 新增：发送添加好友结果
    void sendAddFriendResult(QTcpSocket* client, int userId, int friendId, bool success, const QString& message);

    void sendResponse(QTcpSocket* client, const QString& response);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startstopButton_clicked();
    void logMessage(const QString &msg);
    void onUserLoginSuccess(const QString &nickname);
    void onUserLoginFailed();
    void onUserRegisterSuccess(const QString &username, const QString &nickname);
    void onUserRegisterFailed(const QString &reason);

private:
    Ui::MainWindow *ui;
    ChatServer *m_chatServer;
    DatabaseManager* m_dbManager;
};

#endif // MAINWINDOW_H
