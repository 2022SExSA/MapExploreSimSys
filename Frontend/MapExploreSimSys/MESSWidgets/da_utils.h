#ifndef da_utils_h
#define da_utils_h
#include <QString>

#include "dll_export.h"
#include "xpack/json.h"

enum LoginState {
    UserNameNotFound,
    UserNamePasswordNotMatched,
    LoginSuccess,
    UserTypeNotMatched,
};

enum RegisterState {
    IDrepeat,
    RegSuccess,
};

enum UserType {
    User,
    Admin
};

struct UserInfo {
    std::string id;
    std::string password;
    std::string name;
    int type;

    XPACK(O(id, password, name, type));
};

struct RDStruct {
    int code; // 0: Success
    std::string msg;
    xpack::JsonData data;

    XPACK(O(code, msg, data));
};
using RD = RDStruct;

MESSWIDGETS_EXPORT void InitDAUtils(const QString &http_url);

QVector<UserInfo> GetUserInfoWithFilter(QString infix, QString column, RD &rd);

QVector<UserInfo> GetAllUserInfo(RD &rd);

void InsertUser(UserInfo userInfo, RD &rd);

void DeleteUser(QString id, RD &rd);

void UpdatingUser(UserInfo userInfo, RD &rd);

void AuthUser(QString ID, QString Password, UserType type, RD &rd);

void AddUser(const UserInfo &u, RD &rd);

#endif // !da_utils_h
