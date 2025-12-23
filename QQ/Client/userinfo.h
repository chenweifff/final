#ifndef USERINFO_H
#define USERINFO_H

#include <QString>

// 用户信息结构体定义
struct UserInfo {
    int userId;
    QString username;
    QString nickname;
    QString avatarPath;
    int status;
};

#endif // USERINFO_H
