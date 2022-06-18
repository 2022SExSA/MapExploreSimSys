package db

import (
	"UserServer/merr"
	"UserServer/model"
	"github.com/dgrijalva/jwt-go"
	"time"
)

func AuthUser(id, pwd string) (string, error) {
	rows, err := conn.Query("select id, password, name, type from user_table where id = ?", id)
	if err != nil {
		panic(err)
	}
	if !rows.Next() {
		return "", merr.MErr("用户不存在")
	}

	token, err := GetToken(id)
	return token, err
}

func GetUserInfo(id string) (model.User, error) {
	rows, err := conn.Query("select id, password, name, type from user_table where id = ?", id)
	if err != nil {
		panic(err)
	}
	if !rows.Next() {
		return model.User{}, merr.MErr("用户不存在")
	}

	user := model.User{}
	rows.Scan(&user.ID, &user.Password, &user.Name, &user.Type)
	return user, nil
}


func AddUser(u model.User) error {
	_, err := conn.Exec("insert into user_table(id, password, name, type) values(?, ?, ?, ?)", u.ID, u.Password, u.Name, u.Type)
	if err != nil {
		return err
	}
	return nil
}

func RemoveUser(id string) error {
	_, err := conn.Exec("delete from user_table where id = ?", id)
	if err != nil {
		return err
	}
	return nil
}

func UpdateUser(new_user model.User) error {
	err := RemoveUser(new_user.ID)
	if err != nil {
		return err
	}
	return AddUser(new_user)
}

func FindUsers(column, pattern string) ([]model.User, error) {
	rows, err := conn.Query("select id, password, name, type from user_table where " + column + " like ?", pattern)
	if err != nil {
		panic(err)
	}
	res := make([]model.User, 0, 10)
	for rows.Next() {
		user := model.User{}
		rows.Scan(&user.ID, &user.Password, &user.Name, &user.Type)
		res = append(res, user)
	}
	return res, nil
}
var jwt_secret string = "PGZXB_MESS_USER2022SExSA"

type claims struct {
	ID       string  `json:"id"`
	Type int `json:"type"`
	jwt.StandardClaims
}

func GetToken(id string) (string, error) {
	nowTime := time.Now()
	expireTime := nowTime.Add(3 * time.Hour)

	user, err := GetUserInfo(id)
	if err != nil {
		return "", merr.MErr("获取用户失败")
	}
	c := claims{
		user.ID,
		user.Type,
		jwt.StandardClaims{
			ExpiresAt: expireTime.Unix(),
			Issuer:    "pgus",
		},
	}

	tokenClaims := jwt.NewWithClaims(jwt.SigningMethodHS256, c)
	token, err := tokenClaims.SignedString([]byte(jwt_secret))

	if err != nil {
		return "", err
	}

	return token, nil
}

