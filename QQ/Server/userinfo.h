#ifndef USERINFO_H
#define USERINFO_H

#include <QString>
#include <QDateTime>

// 用户信息结构体定义
struct UserInfo {
    int userId;
    QString username;
    QString nickname;
    QString avatarPath;
    int status;
};

// 消息信息结构体定义
struct MessageInfo {
    int messageId;
    int senderId;
    int receiverId;
    QString content;
    QString sendTime;
    int contentType; // 1: 文本消息, 2: 文件消息
    QString fileName;
    qint64 fileSize;
};

#endif // USERINFO_H
