#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

signals:
    void registrationSuccess();

private slots:
    void on_PathpushButton_clicked();
    void on_RegisterpushButton_clicked();
    void on_BackpushButton_clicked();

private:
    Ui::Register *ui;
    bool m_fileDialogOpen = false;
};

#endif // REGISTER_H
