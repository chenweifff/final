#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include "database.h"
#include "userinfo.h"  // 包含统一的UserInfo定义

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 聊天服务器类
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

    // 处理登录请求
    void handleLoginRequest(QTcpSocket* client, const QString& username, const QString& password);
    // 处理注册请求
    void handleRegisterRequest(QTcpSocket* client, const QString& username, const QString& password,
                               const QString& nickname, const QString& avatarPath);
    // 发送响应给客户端
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
