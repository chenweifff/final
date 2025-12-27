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

// 添加新函数：用户退出时更新状态
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
