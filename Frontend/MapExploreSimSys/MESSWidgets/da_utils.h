#ifndef da_utils_h
#define da_utils_h
#include <QString>

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
    QString id;
    QString password;
    QString name;
    UserType type;
};

QVector<UserInfo> GetUserInfoWithFilter(QString infix, QString column);

QVector<UserInfo> GetAllUserInfo();

bool InsertUser(UserInfo userInfo);

bool DeleteUser(QString id);

bool UpdatingUser(UserInfo userInfo);

LoginState AuthUser(QString ID, QString Password, UserType type);

RegisterState AddUser(const UserInfo &u);

#endif // !da_utils_h
