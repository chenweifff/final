#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QList>

// 使用统一的UserInfo定义
#include "userinfo.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    bool connectToDatabase(const QString& dbPath = "E:/qt/final/QQChatDB.db");
    void closeDatabase();

    // 用户认证
    bool authenticateUser(const QString& username, const QString& password, UserInfo& userInfo);

    // 用户注册
    bool registerUser(const QString& username, const QString& password,
                      const QString& nickname, const QString& avatarPath);

    // 获取好友列表
    QList<UserInfo> getFriendList(int userId);

    // 更新用户状态
    bool updateUserStatus(int userId, int status);

    // 获取聊天记录
    QList<MessageInfo> getMessageList(int user1Id, int user2Id);

    // 保存消息
    bool saveMessage(int senderId, int receiverId, int contentType,
                     const QString& content, const QString& fileName = "",
                     qint64 fileSize = 0);

private:
    DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_database;
};

#endif // DATABASE_H
