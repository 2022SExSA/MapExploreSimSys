#include "da_utils.h"

#include "utils.h"
#include <QVector>
#include <QNetworkAccessManager>

static QNetworkAccessManager *http{nullptr};
static QString http_url;
static std::string auth_token;

struct Auth {
    std::string id;
    std::string password;
    int type;

    XPACK(O(id, password, type));
};

struct RM {
    std::string id;

    XPACK(O(id));
};

struct FU {
    std::string column;
    std::string infix;

    XPACK(O(column, infix));
};

#define DEFINE_POST(relurl, body_object) \
    syncHttpPOST(http, http_url + (relurl) + "?t=" + ::auth_token.c_str(), QByteArray::fromStdString(xpack::json::encode(body_object)), \
        [&rd](const QString &body) { \
            try { qDebug() << body; \
                xpack::json::decode(body.toStdString(), rd); \
            } catch (const std::exception &e) { \
                rd.code = -1; \
                rd.msg = e.what(); \
            } \
        }, \
        [&rd](const QString &msg) { \
            rd.code = -1; \
            rd.msg = msg.toStdString(); \
        });

void InitDAUtils(const QString & http_url) {
    ::http = new QNetworkAccessManager;
    ::http_url = http_url;
}

QVector<UserInfo> GetUserInfoWithFilter(QString infix, QString column, RD &rd) {
    FU f = {column.toStdString(), infix.toStdString()};
    DEFINE_POST("/find_users", f);
    std::vector<UserInfo> vec;
    try {
        if (rd.data["users"])
            rd.data["users"].decode(vec);
    } catch (const std::exception &e) {
        qDebug() << e.what();
    }
    return QVector(vec.begin(), vec.end());
}

QVector<UserInfo> GetAllUserInfo(RD &rd) {
    return GetUserInfoWithFilter("", "id", rd); // infix column
}

void InsertUser(UserInfo userInfo, RD &rd) {
    AddUser(userInfo, rd);
}

void DeleteUser(QString id, RD &rd) {
    RM rm = {id.toStdString()};
    DEFINE_POST("/remove_user", rm);
}

void UpdatingUser(UserInfo userInfo, RD &rd) {
    DEFINE_POST("/update_user", userInfo);
}

void AuthUser(QString ID, QString Password, UserType type, RD &rd) {
    Auth a = {ID.toStdString(), Password.toStdString(), (int)type};
    DEFINE_POST("/auth_user", a);
}

void AddUser(const UserInfo &u, RD &rd) {
    DEFINE_POST("/add_user", u);
}

void SetAuthToken(const std::string & token) {
    ::auth_token = token;
}
