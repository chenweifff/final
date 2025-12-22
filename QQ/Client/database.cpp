#include "database.h"

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::connectToDatabase(const QString& dbPath)
{
    if (m_database.isOpen()) {
        return true;
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qDebug() << "Error: Failed to connect to database:" << m_database.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully!";
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::authenticateUser(const QString& username, const QString& password, UserInfo& userInfo)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT user_id, username, nickname, avatar_path, status FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Authentication query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        userInfo.userId = query.value(0).toInt();
        userInfo.username = query.value(1).toString();
        userInfo.nickname = query.value(2).toString();
        userInfo.avatarPath = query.value(3).toString();
        userInfo.status = query.value(4).toInt();

        // 更新最后登录时间
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET last_login = datetime('now'), status = 1 WHERE user_id = :userId");
        updateQuery.bindValue(":userId", userInfo.userId);
        updateQuery.exec();

        return true;
    }

    return false;
}

bool DatabaseManager::registerUser(const QString& username, const QString& password,
                                   const QString& nickname, const QString& avatarPath)
{
    if (!m_database.isOpen()) {
        return false;
    }

    // 检查用户名是否已存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    checkQuery.bindValue(":username", username);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Check username failed:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.value(0).toInt() > 0) {
        qDebug() << "Username already exists";
        return false;
    }

    // 插入新用户
    QSqlQuery query;
    query.prepare(
        "INSERT INTO users (username, password, nickname, avatar_path, status, created_at) "
        "VALUES (:username, :password, :nickname, :avatarPath, 0, datetime('now'))"
        );
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":nickname", nickname);
    query.bindValue(":avatarPath", avatarPath);

    if (!query.exec()) {
        qDebug() << "Register user failed:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<FriendInfo> DatabaseManager::getFriendsList(int userId)
{
    QList<FriendInfo> friends;

    if (!m_database.isOpen()) {
        return friends;
    }

    // 查询用户的所有好友
    QSqlQuery query;
    query.prepare(
        "SELECT u.user_id, u.nickname, f.remark_name, u.avatar_path, u.status "
        "FROM friendships f "
        "JOIN users u ON (f.user_id2 = u.user_id) "
        "WHERE f.user_id1 = :userId "
        "UNION "
        "SELECT u.user_id, u.nickname, f.remark_name, u.avatar_path, u.status "
        "FROM friendships f "
        "JOIN users u ON (f.user_id1 = u.user_id) "
        "WHERE f.user_id2 = :userId"
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Get friends query failed:" << query.lastError().text();
        return friends;
    }

    while (query.next()) {
        FriendInfo friendInfo;
        friendInfo.friendId = query.value(0).toInt();
        friendInfo.nickname = query.value(1).toString();
        friendInfo.remarkName = query.value(2).toString();
        friendInfo.avatarPath = query.value(3).toString();
        friendInfo.status = query.value(4).toInt();

        // 如果好友有备注名，则显示备注名
        if (!friendInfo.remarkName.isEmpty()) {
            friendInfo.nickname = friendInfo.remarkName;
        }

        friends.append(friendInfo);
    }

    return friends;
}

bool DatabaseManager::addFriend(int userId, int friendId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO friendships (user_id1, user_id2, created_at) "
        "VALUES (:userId1, :userId2, datetime('now'))"
        );
    query.bindValue(":userId1", userId);
    query.bindValue(":userId2", friendId);

    if (!query.exec()) {
        qDebug() << "Add friend failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::removeFriend(int userId, int friendId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "DELETE FROM friendships "
        "WHERE (user_id1 = :userId1 AND user_id2 = :userId2) OR (user_id1 = :userId2 AND user_id2 = :userId1)"
        );
    query.bindValue(":userId1", userId);
    query.bindValue(":userId2", friendId);

    if (!query.exec()) {
        qDebug() << "Remove friend failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<MessageInfo> DatabaseManager::getChatHistory(int user1, int user2, int limit)
{
    QList<MessageInfo> messages;

    if (!m_database.isOpen()) {
        return messages;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT message_id, sender_id, receiver_id, content, send_time, is_read "
        "FROM messages "
        "WHERE (sender_id = :user1 AND receiver_id = :user2) OR (sender_id = :user2 AND receiver_id = :user1) "
        "ORDER BY send_time DESC "
        "LIMIT :limit"
        );
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qDebug() << "Get chat history query failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        MessageInfo message;
        message.messageId = query.value(0).toInt();
        message.senderId = query.value(1).toInt();
        message.receiverId = query.value(2).toInt();
        message.content = query.value(3).toString();
        message.sendTime = query.value(4).toString();
        message.isRead = query.value(5).toBool();

        messages.append(message);
    }

    // 按时间顺序排序（最新的在后面）
    std::reverse(messages.begin(), messages.end());

    return messages;
}

bool DatabaseManager::saveMessage(int senderId, int receiverId, const QString& content)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO messages (sender_id, receiver_id, content_type, content, send_time) "
        "VALUES (:senderId, :receiverId, 1, :content, datetime('now'))"
        );
    query.bindValue(":senderId", senderId);
    query.bindValue(":receiverId", receiverId);
    query.bindValue(":content", content);

    if (!query.exec()) {
        qDebug() << "Save message failed:" << query.lastError().text();
        return false;
    }

    // 获取最后插入的message_id
    int lastMessageId = query.lastInsertId().toInt();

    // 更新会话表（双方）
    updateConversation(senderId, receiverId, lastMessageId);
    updateConversation(receiverId, senderId, lastMessageId);

    return true;
}

void DatabaseManager::updateConversation(int userId, int friendId, int lastMessageId)
{
    QSqlQuery query;
    query.prepare(
        "INSERT OR REPLACE INTO conversations (user_id, friend_id, last_message_id, updated_at) "
        "VALUES (:userId, :friendId, :lastMessageId, datetime('now'))"
        );
    query.bindValue(":userId", userId);
    query.bindValue(":friendId", friendId);
    query.bindValue(":lastMessageId", lastMessageId);

    if (!query.exec()) {
        qDebug() << "Update conversation failed:" << query.lastError().text();
    }
}

void DatabaseManager::markMessagesAsRead(int userId, int friendId)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE messages SET is_read = 1 "
        "WHERE receiver_id = :userId AND sender_id = :friendId AND is_read = 0"
        );
    query.bindValue(":userId", userId);
    query.bindValue(":friendId", friendId);

    if (!query.exec()) {
        qDebug() << "Mark messages as read failed:" << query.lastError().text();
    }
}

int DatabaseManager::getUnreadCount(int userId, int friendId)
{
    if (!m_database.isOpen()) {
        return 0;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM messages "
        "WHERE receiver_id = :userId AND sender_id = :friendId AND is_read = 0"
        );
    query.bindValue(":userId", userId);
    query.bindValue(":friendId", friendId);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}
