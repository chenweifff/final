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

        // 更新最后登录时间和状态
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET last_login = datetime('now'), status = 1 WHERE user_id = :userId");
        updateQuery.bindValue(":userId", userInfo.userId);
        if (!updateQuery.exec()) {
            qDebug() << "Update last_login failed:" << updateQuery.lastError().text();
        }

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

QList<UserInfo> DatabaseManager::getFriendList(int userId)
{
    QList<UserInfo> friendList;

    if (!m_database.isOpen()) {
        qDebug() << "Database is not open";
        return friendList;
    }

    // 修复的好友查询逻辑：查找所有与该用户是好友关系的用户
    QSqlQuery query;
    query.prepare(
        "SELECT DISTINCT "
        "CASE WHEN f.user_id1 = :userId THEN f.user_id2 ELSE f.user_id1 END as friend_id, "
        "u.username, u.nickname, u.avatar_path, u.status "
        "FROM friendships f "
        "JOIN users u ON (u.user_id = CASE WHEN f.user_id1 = :userId THEN f.user_id2 ELSE f.user_id1 END) "
        "WHERE (f.user_id1 = :userId OR f.user_id2 = :userId) "
        "AND u.user_id != :userId "
        "ORDER BY u.status DESC, u.nickname"
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Get friend list failed:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
        return friendList;
    }

    qDebug() << "Found friends for user" << userId << ":";
    while (query.next()) {
        UserInfo friendInfo;
        friendInfo.userId = query.value(0).toInt();
        friendInfo.username = query.value(1).toString();
        friendInfo.nickname = query.value(2).toString();
        friendInfo.avatarPath = query.value(3).toString();
        friendInfo.status = query.value(4).toInt();
        friendList.append(friendInfo);

        qDebug() << "Friend:" << friendInfo.nickname << "ID:" << friendInfo.userId << "Status:" << friendInfo.status;
    }

    return friendList;
}

bool DatabaseManager::updateUserStatus(int userId, int status)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE users SET status = :status WHERE user_id = :userId");
    query.bindValue(":status", status);
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Update user status failed:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<MessageInfo> DatabaseManager::getMessageList(int user1Id, int user2Id)
{
    QList<MessageInfo> messageList;

    if (!m_database.isOpen()) {
        qDebug() << "Database is not open";
        return messageList;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT message_id, sender_id, receiver_id, content_type, content, file_name, file_size, "
        "strftime('%Y-%m-%d %H:%M:%S', send_time) as send_time "
        "FROM messages "
        "WHERE (sender_id = :user1Id AND receiver_id = :user2Id) "
        "OR (sender_id = :user2Id AND receiver_id = :user1Id) "
        "ORDER BY send_time ASC"
        );
    query.bindValue(":user1Id", user1Id);
    query.bindValue(":user2Id", user2Id);

    if (!query.exec()) {
        qDebug() << "Get message list failed:" << query.lastError().text();
        return messageList;
    }

    qDebug() << "Found messages between user" << user1Id << "and user" << user2Id << ":";
    while (query.next()) {
        MessageInfo message;
        message.messageId = query.value(0).toInt();
        message.senderId = query.value(1).toInt();
        message.receiverId = query.value(2).toInt();
        message.contentType = query.value(3).toInt();
        message.content = query.value(4).toString();
        message.fileName = query.value(5).toString();
        message.fileSize = query.value(6).toLongLong();
        message.sendTime = query.value(7).toString();
        messageList.append(message);

        qDebug() << "Message:" << message.content << "From:" << message.senderId << "To:" << message.receiverId;
    }

    return messageList;
}

bool DatabaseManager::saveMessage(int senderId, int receiverId, int contentType,
                                  const QString& content, const QString& fileName,
                                  qint64 fileSize)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO messages (sender_id, receiver_id, content_type, content, file_name, file_size, send_time) "
        "VALUES (:senderId, :receiverId, :contentType, :content, :fileName, :fileSize, datetime('now'))"
        );
    query.bindValue(":senderId", senderId);
    query.bindValue(":receiverId", receiverId);
    query.bindValue(":contentType", contentType);
    query.bindValue(":content", content);
    query.bindValue(":fileName", fileName);
    query.bindValue(":fileSize", fileSize);

    if (!query.exec()) {
        qDebug() << "Save message failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 搜索用户函数实现
QList<UserInfo> DatabaseManager::searchUsers(int userId, const QString& keyword, bool excludeFriends)
{
    QList<UserInfo> userList;

    if (!m_database.isOpen()) {
        qDebug() << "Database is not open";
        return userList;
    }

    // 构建基础查询
    QString sql =
        "SELECT u.user_id, u.username, u.nickname, u.avatar_path, u.status "
        "FROM users u "
        "WHERE u.nickname LIKE :keyword "
        "AND u.user_id != :userId ";

    // 如果排除好友，添加排除条件
    if (excludeFriends) {
        sql +=
            "AND NOT EXISTS ("
            "    SELECT 1 FROM friendships f "
            "    WHERE (f.user_id1 = :userId AND f.user_id2 = u.user_id) "
            "       OR (f.user_id1 = u.user_id AND f.user_id2 = :userId)"
            ") ";
    }

    sql += "ORDER BY u.status DESC, u.nickname";

    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":keyword", "%" + keyword + "%");
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Search users failed:" << query.lastError().text();
        qDebug() << "SQL:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        return userList;
    }

    qDebug() << "Search results for user" << userId << "with keyword" << keyword << "(excludeFriends:" << excludeFriends << "):";
    int count = 0;
    while (query.next()) {
        UserInfo userInfo;
        userInfo.userId = query.value(0).toInt();
        userInfo.username = query.value(1).toString();
        userInfo.nickname = query.value(2).toString();
        userInfo.avatarPath = query.value(3).toString();
        userInfo.status = query.value(4).toInt();
        userList.append(userInfo);
        count++;

        qDebug() << "User:" << userInfo.nickname << "ID:" << userInfo.userId << "Status:" << userInfo.status;
    }

    qDebug() << "Total found:" << count << "users";
    return userList;
}

// 新增：检查是否是好友
bool DatabaseManager::isFriend(int userId1, int userId2)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM friendships "
        "WHERE (user_id1 = :userId1 AND user_id2 = :userId2) "
        "OR (user_id1 = :userId2 AND user_id2 = :userId1)"
        );
    query.bindValue(":userId1", userId1);
    query.bindValue(":userId2", userId2);

    if (!query.exec() || !query.next()) {
        qDebug() << "Check friendship failed:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

// 新增：添加好友
bool DatabaseManager::addFriend(int userId1, int userId2, const QString& remarkName)
{
    if (!m_database.isOpen()) {
        return false;
    }

    // 先检查是否已经是好友
    if (isFriend(userId1, userId2)) {
        qDebug() << "Users are already friends";
        return false;
    }

    // 确保 user_id1 < user_id2 以避免重复
    int smallerId = qMin(userId1, userId2);
    int largerId = qMax(userId1, userId2);

    QSqlQuery query;
    query.prepare(
        "INSERT INTO friendships (user_id1, user_id2, remark_name, created_at) "
        "VALUES (:userId1, :userId2, :remarkName, datetime('now'))"
        );
    query.bindValue(":userId1", smallerId);
    query.bindValue(":userId2", largerId);
    query.bindValue(":remarkName", remarkName);

    if (!query.exec()) {
        qDebug() << "Add friend failed:" << query.lastError().text();
        return false;
    }

    qDebug() << "Friendship added between user" << smallerId << "and user" << largerId;
    return true;
}
