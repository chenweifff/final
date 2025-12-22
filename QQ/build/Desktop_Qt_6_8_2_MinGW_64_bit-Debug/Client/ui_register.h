/********************************************************************************
** Form generated from reading UI file 'register.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTER_H
#define UI_REGISTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Register
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *horizontalSpacer;
    QLabel *label_5;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLineEdit *ResUsernameEdit;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QLineEdit *ResPasswordEdit;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *ResNicknameEdit;
    QHBoxLayout *horizontalLayout;
    QLabel *label_4;
    QLineEdit *ResAvatarEdit;
    QPushButton *PathpushButton;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *BackpushButton;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *RegisterpushButton;
    QSpacerItem *horizontalSpacer_7;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *Register)
    {
        if (Register->objectName().isEmpty())
            Register->setObjectName("Register");
        Register->resize(569, 364);
        gridLayout = new QGridLayout(Register);
        gridLayout->setObjectName("gridLayout");
        verticalSpacer_3 = new QSpacerItem(20, 43, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 0, 4, 1, 1);

        horizontalSpacer = new QSpacerItem(239, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 0, 1, 3);

        label_5 = new QLabel(Register);
        label_5->setObjectName("label_5");
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label_5->setFont(font);
        label_5->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(label_5, 1, 3, 1, 2);

        horizontalSpacer_2 = new QSpacerItem(238, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 1, 5, 1, 3);

        verticalSpacer_2 = new QSpacerItem(20, 43, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 2, 4, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(168, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 3, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label = new QLabel(Register);
        label->setObjectName("label");
        label->setMinimumSize(QSize(50, 0));

        horizontalLayout_3->addWidget(label);

        ResUsernameEdit = new QLineEdit(Register);
        ResUsernameEdit->setObjectName("ResUsernameEdit");
        ResUsernameEdit->setMinimumSize(QSize(150, 0));

        horizontalLayout_3->addWidget(ResUsernameEdit);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label_2 = new QLabel(Register);
        label_2->setObjectName("label_2");
        label_2->setMinimumSize(QSize(50, 0));

        horizontalLayout_4->addWidget(label_2);

        ResPasswordEdit = new QLineEdit(Register);
        ResPasswordEdit->setObjectName("ResPasswordEdit");

        horizontalLayout_4->addWidget(ResPasswordEdit);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_3 = new QLabel(Register);
        label_3->setObjectName("label_3");
        label_3->setMinimumSize(QSize(50, 0));

        horizontalLayout_2->addWidget(label_3);

        ResNicknameEdit = new QLineEdit(Register);
        ResNicknameEdit->setObjectName("ResNicknameEdit");

        horizontalLayout_2->addWidget(ResNicknameEdit);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label_4 = new QLabel(Register);
        label_4->setObjectName("label_4");
        label_4->setMinimumSize(QSize(50, 0));
        label_4->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout->addWidget(label_4);

        ResAvatarEdit = new QLineEdit(Register);
        ResAvatarEdit->setObjectName("ResAvatarEdit");

        horizontalLayout->addWidget(ResAvatarEdit);

        PathpushButton = new QPushButton(Register);
        PathpushButton->setObjectName("PathpushButton");
        PathpushButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(PathpushButton);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 3, 1, 1, 6);

        horizontalSpacer_4 = new QSpacerItem(167, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 3, 7, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 43, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_4, 4, 4, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(203, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_6, 5, 0, 1, 2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        BackpushButton = new QPushButton(Register);
        BackpushButton->setObjectName("BackpushButton");

        horizontalLayout_5->addWidget(BackpushButton);

        horizontalSpacer_5 = new QSpacerItem(13, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);

        RegisterpushButton = new QPushButton(Register);
        RegisterpushButton->setObjectName("RegisterpushButton");

        horizontalLayout_5->addWidget(RegisterpushButton);


        gridLayout->addLayout(horizontalLayout_5, 5, 2, 1, 4);

        horizontalSpacer_7 = new QSpacerItem(202, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_7, 5, 6, 1, 2);

        verticalSpacer = new QSpacerItem(20, 43, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer, 6, 4, 1, 1);


        retranslateUi(Register);

        QMetaObject::connectSlotsByName(Register);
    } // setupUi

    void retranslateUi(QDialog *Register)
    {
        Register->setWindowTitle(QCoreApplication::translate("Register", "Dialog", nullptr));
        label_5->setText(QCoreApplication::translate("Register", "\347\224\250\346\210\267\346\263\250\345\206\214", nullptr));
        label->setText(QCoreApplication::translate("Register", "\347\224\250\346\210\267\345\220\215", nullptr));
        label_2->setText(QCoreApplication::translate("Register", "\345\257\206\347\240\201", nullptr));
        label_3->setText(QCoreApplication::translate("Register", "\346\230\265\347\247\260", nullptr));
        label_4->setText(QCoreApplication::translate("Register", "\345\244\264\345\203\217\350\256\276\347\275\256", nullptr));
        PathpushButton->setText(QCoreApplication::translate("Register", "...", nullptr));
        BackpushButton->setText(QCoreApplication::translate("Register", "\351\200\200\345\207\272", nullptr));
        RegisterpushButton->setText(QCoreApplication::translate("Register", "\346\263\250\345\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Register: public Ui_Register {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTER_H
