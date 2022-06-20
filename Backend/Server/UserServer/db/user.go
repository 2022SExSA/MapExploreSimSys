package db

import (
	"UserServer/merr"
	"UserServer/model"
	"fmt"
	"time"

	"github.com/dgrijalva/jwt-go"
)

func AuthUser(id, pwd string, ty int) (string, error) {
	rows, err := conn.Query("select password, type from user_table where id = ?", id)
	if err != nil {
		panic(err)
	}
	defer rows.Close()
	if !rows.Next() {
		return "", merr.MErr("用户不存在")
	}
	var p string
	var t int
	rows.Scan(&p, &t)
	if p != pwd {
		return "", merr.MErr("密码错误")
	}
	if t != ty {
		return "", merr.MErr("用户类型不匹配")
	}
	token, err := GetToken(id)
	return token, err
}

func GetUserInfo(id string) (model.User, error) {
	rows, err := conn.Query("select id, password, name, type from user_table where id = ?", id)
	if err != nil {
		panic(err)
	}
	defer rows.Close()
	if !rows.Next() {
		return model.User{}, merr.MErr("用户不存在")
	}

	user := model.User{}
	rows.Scan(&user.ID, &user.Password, &user.Name, &user.Type)
	return user, nil
}

func AddUser(u model.User) error {
	fmt.Printf("Add user=%v\n", u)
	if _, err := GetUserInfo(u.ID); err == nil {
		return merr.MErr("用户已存在")
	}
	_, err := conn.Exec("insert into user_table(id, password, name, type) values(?, ?, ?, ?)", u.ID, u.Password, u.Name, u.Type)
	if err != nil {
		return merr.MErr("内部错误(联系开发人员)：" + err.Error())
	}
	return nil
}

func RemoveUser(id string) error {
	fmt.Printf("Remove id=%v\n", id)
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
	rows, err := conn.Query("select id, password, name, type from user_table where "+column+" like ?", pattern)
	res := make([]model.User, 0, 10)
	defer rows.Close()
	if err != nil {
		return res, err
	}
	for rows.Next() {
		user := model.User{}
		rows.Scan(&user.ID, &user.Password, &user.Name, &user.Type)
		res = append(res, user)
	}
	return res, nil
}

var jwt_secret string = "PGZXB_MESS_USER2022SExSA"

type claims struct {
	ID   string `json:"id"`
	Type int    `json:"type"`
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
