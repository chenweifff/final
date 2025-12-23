#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "userinfo.h"  // 包含统一的UserInfo定义

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    void onSocketConnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
    QTcpSocket *m_tcpSocket;
    bool m_serverConnected;

    void connectToServer();
    void disconnectFromServer();
};

#endif // LOGIN_H
