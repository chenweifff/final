#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

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

    // 用户注册（服务器端暂时不需要，可以留着）
    bool registerUser(const QString& username, const QString& password,
                      const QString& nickname, const QString& avatarPath);

private:
    DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_database;
};

#endif // DATABASE_H
