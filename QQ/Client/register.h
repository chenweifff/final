#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QTcpSocket>
#include <QAbstractSocket>

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

    void setTcpSocket(QTcpSocket* socket);
    void connectToServer();

signals:
    void registrationSuccess();

private slots:
    void on_PathpushButton_clicked();
    void on_RegisterpushButton_clicked();
    void on_BackpushButton_clicked();
    void onSocketReadyRead();
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    Ui::Register *ui;
    QTcpSocket *m_tcpSocket = nullptr;
    bool m_fileDialogOpen = false;
    bool m_serverConnected = false;

    void disconnectFromServer();
};

#endif // REGISTER_H
