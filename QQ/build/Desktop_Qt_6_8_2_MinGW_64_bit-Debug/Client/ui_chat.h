/********************************************************************************
** Form generated from reading UI file 'chat.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Chat
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLineEdit *searchEdit;
    QListWidget *friendListWidget;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *friendNameLabel;
    QTextBrowser *messageBrowser;
    QPushButton *sendFileButton;
    QTextEdit *messageEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QMenu *menu;

    void setupUi(QMainWindow *Chat)
    {
        if (Chat->objectName().isEmpty())
            Chat->setObjectName("Chat");
        Chat->resize(477, 444);
        Chat->setStyleSheet(QString::fromUtf8(""));
        centralwidget = new QWidget(Chat);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setMaximumSize(QSize(130, 16777215));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        searchEdit = new QLineEdit(widget);
        searchEdit->setObjectName("searchEdit");

        verticalLayout->addWidget(searchEdit);

        friendListWidget = new QListWidget(widget);
        friendListWidget->setObjectName("friendListWidget");
        friendListWidget->setStyleSheet(QString::fromUtf8("QListWidget {\n"
"    background-color: white;\n"
"    border: none;\n"
"}\n"
"QListWidget::item {\n"
"    padding: 5px;\n"
"}\n"
"QListWidget::item:hover {\n"
"    background-color: #e6e6e6;\n"
"}\n"
"QListWidget::item:selected {\n"
"    background-color: #cce5ff;\n"
"}"));

        verticalLayout->addWidget(friendListWidget);


        horizontalLayout_2->addWidget(widget);

        widget_2 = new QWidget(centralwidget);
        widget_2->setObjectName("widget_2");
        verticalLayout_2 = new QVBoxLayout(widget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        friendNameLabel = new QLabel(widget_2);
        friendNameLabel->setObjectName("friendNameLabel");
        QFont font;
        font.setPointSize(14);
        font.setBold(false);
        friendNameLabel->setFont(font);

        verticalLayout_2->addWidget(friendNameLabel);

        messageBrowser = new QTextBrowser(widget_2);
        messageBrowser->setObjectName("messageBrowser");

        verticalLayout_2->addWidget(messageBrowser);

        sendFileButton = new QPushButton(widget_2);
        sendFileButton->setObjectName("sendFileButton");
        sendFileButton->setMaximumSize(QSize(60, 16777215));

        verticalLayout_2->addWidget(sendFileButton);

        messageEdit = new QTextEdit(widget_2);
        messageEdit->setObjectName("messageEdit");
        messageEdit->setMaximumSize(QSize(16777215, 80));
        messageEdit->setStyleSheet(QString::fromUtf8("QTextEdit {\n"
"    border: 1px solid #ccc;\n"
"    border-radius: 5px;\n"
"    padding: 5px;\n"
"}"));

        verticalLayout_2->addWidget(messageEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(widget_2);
        pushButton->setObjectName("pushButton");
        pushButton->setMaximumSize(QSize(60, 16777215));

        horizontalLayout->addWidget(pushButton);


        verticalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_2->addWidget(widget_2);

        Chat->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Chat);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 477, 18));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        Chat->setMenuBar(menubar);

        menubar->addAction(menu->menuAction());

        retranslateUi(Chat);

        QMetaObject::connectSlotsByName(Chat);
    } // setupUi

    void retranslateUi(QMainWindow *Chat)
    {
        Chat->setWindowTitle(QCoreApplication::translate("Chat", "MainWindow", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("Chat", "\346\220\234\347\264\242\345\245\275\345\217\213", nullptr));
        friendNameLabel->setText(QCoreApplication::translate("Chat", "TextLabel", nullptr));
        sendFileButton->setText(QCoreApplication::translate("Chat", "\345\217\221\351\200\201\346\226\207\344\273\266", nullptr));
        messageEdit->setPlaceholderText(QCoreApplication::translate("Chat", "\350\257\267\350\276\223\345\205\245\346\226\207\346\234\254", nullptr));
        pushButton->setText(QCoreApplication::translate("Chat", "\345\217\221\351\200\201", nullptr));
        menu->setTitle(QCoreApplication::translate("Chat", "\351\200\200\345\207\272\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Chat: public Ui_Chat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHAT_H
