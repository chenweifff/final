#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include <QMap>

// 用户信息结构体
struct UserInfo {
    int userId;
    QString username;
    QString nickname;
    QString avatarPath;
    int status;
};

// 好友信息结构体
struct FriendInfo {
    int friendId;
    QString nickname;
    QString remarkName;
    QString avatarPath;
    int status;
};

// 消息结构体
struct MessageInfo {
    int messageId;
    int senderId;
    int receiverId;
    QString content;
    QString sendTime;
    bool isRead;
};

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

    // 好友操作
    QList<FriendInfo> getFriendsList(int userId);
    bool addFriend(int userId, int friendId);
    bool removeFriend(int userId, int friendId);

    // 消息操作
    QList<MessageInfo> getChatHistory(int user1, int user2, int limit = 100);
    bool saveMessage(int senderId, int receiverId, const QString& content);
    void markMessagesAsRead(int userId, int friendId);

    // 会话操作
    void updateConversation(int userId, int friendId, int lastMessageId);
    int getUnreadCount(int userId, int friendId);

private:
    DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_database;
};

#endif // DATABASE_H
