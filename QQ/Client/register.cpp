#include "register.h"
#include "ui_register.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QHostAddress>


Register::Register(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Register)
    , m_fileDialogOpen(false)
    , m_serverConnected(false)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("用户注册");

    // 设置密码输入框为密码模式
    ui->ResPasswordEdit->setEchoMode(QLineEdit::Password);

    // 初始化TCP Socket
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Register::onSocketConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Register::onSocketReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, &Register::onSocketError);
#else
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &Register::onSocketError);
#endif
}

Register::~Register()
{
    delete ui;
    disconnectFromServer();
}

void Register::setTcpSocket(QTcpSocket* socket)
{
    if (m_tcpSocket) {
        m_tcpSocket->deleteLater();
    }
    m_tcpSocket = socket;

    connect(m_tcpSocket, &QTcpSocket::connected, this, &Register::onSocketConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Register::onSocketReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, &Register::onSocketError);
#else
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &Register::onSocketError);
#endif
}

void Register::connectToServer()
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        m_tcpSocket->connectToHost(QHostAddress::LocalHost, 1967);
        m_tcpSocket->waitForConnected(1000);
    }
}

void Register::disconnectFromServer()
{
    if (m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
    }
}

void Register::on_PathpushButton_clicked()
{
    // 检查是否已经有对话框打开
    if (m_fileDialogOpen) {
        return; // 如果对话框已经打开，直接返回
    }

    m_fileDialogOpen = true;

    // 使用非阻塞方式打开文件对话框
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("选择头像");
    fileDialog->setDirectory(QDir::homePath());
    fileDialog->setNameFilter("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    fileDialog->setViewMode(QFileDialog::Detail);

    // 连接对话框的finished信号，使用成员变量m_fileDialogOpen
    connect(fileDialog, &QFileDialog::finished, this, [this, fileDialog](int result) {
        if (result == QDialog::Accepted) {
            QStringList files = fileDialog->selectedFiles();
            if (!files.isEmpty()) {
                QString filePath = files.first();
                if (!filePath.isEmpty()) {
                    ui->ResAvatarEdit->setText(filePath);
                }
            }
        }

        // 清理对话框
        fileDialog->deleteLater();
        m_fileDialogOpen = false; // 重置标志位
    });

    // 显示对话框（非模态）
    fileDialog->show();
}

void Register::on_RegisterpushButton_clicked()
{
    // 获取输入内容
    QString username = ui->ResUsernameEdit->text().trimmed();
    QString password = ui->ResPasswordEdit->text().trimmed();
    QString nickname = ui->ResNicknameEdit->text().trimmed();
    QString avatarPath = ui->ResAvatarEdit->text().trimmed();

    // 验证输入是否为空
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名不能为空！");
        ui->ResUsernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "密码不能为空！");
        ui->ResPasswordEdit->setFocus();
        return;
    }

    if (nickname.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "昵称不能为空！");
        ui->ResNicknameEdit->setFocus();
        return;
    }

    // 如果头像路径为空，设置为默认值
    if (avatarPath.isEmpty()) {
        avatarPath = "default_avatar.png";
    }

    // 尝试连接服务器
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        disconnectFromServer();
        connectToServer();
    }

    // 发送注册请求到服务器
    QString registerRequest = QString("REGISTER|%1|%2|%3|%4")
                                  .arg(username)
                                  .arg(password)
                                  .arg(nickname)
                                  .arg(avatarPath);

    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->write(registerRequest.toUtf8());
        m_tcpSocket->flush();
        // 移除这里的弹窗，只在收到服务器响应后弹窗
    } else {
        QMessageBox::critical(this, "错误", "无法连接到服务器，请确保服务器已启动！");
    }
}

void Register::on_BackpushButton_clicked()
{
    // 关闭注册窗口，返回登录界面
    this->reject();
}

void Register::onSocketReadyRead()
{
    while (m_tcpSocket->canReadLine()) {
        QByteArray data = m_tcpSocket->readLine();
        QString response = QString::fromUtf8(data).trimmed();

        // 解析服务器响应
        QStringList parts = response.split("|");
        if (parts.size() > 0) {
            QString command = parts[0];

            if (command == "REGISTER_SUCCESS") {
                // 注册成功 - 只在这里弹窗
                QMessageBox::information(this, "注册成功", "创建成功，请返回登录界面登录");
                emit registrationSuccess();
                this->accept();  // 关闭注册窗口

            } else if (command == "REGISTER_FAIL" && parts.size() > 1) {
                // 注册失败
                QString errorMsg = parts[1];
                QMessageBox::critical(this, "注册失败", errorMsg);
            }
        }
    }
}

void Register::onSocketConnected()
{
    m_serverConnected = true;
}

void Register::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_serverConnected = false;
    QMessageBox::critical(this, "连接错误", "无法连接到服务器，请确保服务器已启动！");
}
