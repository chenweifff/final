#include "register.h"
#include "ui_register.h"
#include <QFileDialog>
#include <QMessageBox>
#include "database.h"

Register::Register(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Register)
    , m_fileDialogOpen(false)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("用户注册");

    // 设置密码输入框为密码模式
    ui->ResPasswordEdit->setEchoMode(QLineEdit::Password);
}

Register::~Register()
{
    delete ui;
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

    if (avatarPath.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择头像！");
        return;
    }

    // 调用数据库注册用户
    if (DatabaseManager::instance().registerUser(username, password, nickname, avatarPath)) {
        // 发出注册成功信号
        emit registrationSuccess();

        // 关闭注册窗口
        this->accept();
    } else {
        QMessageBox::critical(this, "注册失败", "用户名已存在或注册失败！");
    }
}

void Register::on_BackpushButton_clicked()
{
    // 关闭注册窗口，返回登录界面
    this->reject();
}
