package main

import (
	"UserServer/db"
	"UserServer/merr"
	"UserServer/middleware"
	"UserServer/model"
	"UserServer/utils"
	"net/http"

	"github.com/gin-gonic/gin"
)

func init() {
	db.Init("./MessUsers.sqlite3")
}

// auth_user
type auth struct {
	ID       string `json:"id"`
	Password string `json:"password"`
	Type     int    `json:"type"`
}

// add_user model.User

// remove_user
type rm struct {
	ID string `json:"id"`
}

// update_user model.User

// find_users
type find struct {
	Column string `json:"column"`
	Infix  string `json:"infix"`
}

func main() {
	r := gin.Default()

	r.POST("/auth_user", func(c *gin.Context) {
		var a auth
		c.ShouldBind(&a)
		token, err := db.AuthUser(a.ID, a.Password, a.Type)
		if err != nil {
			utils.Response(c, 200, err, nil)
			return
		}
		utils.Response(c, 200, nil, gin.H{
			"token": token,
		})
	})

	r.POST("/add_user", func(c *gin.Context) {
		var u model.User
		c.ShouldBind(&u)
		if u.Type != model.UserType {
			utils.Response(c, 200, merr.MErr("权限不足"), nil)
			return
		}
		err := db.AddUser(u)
		utils.Response(c, 200, err, nil)
	})

	r.Use(middleware.Jwt())

	r.POST("/m_add_user", func(c *gin.Context) {
		var u model.User
		c.ShouldBind(&u)
		err := db.AddUser(u)
		utils.Response(c, 200, err, nil)
	})

	r.Use(func(c *gin.Context) {
		ty, ok := c.Get("type")
		t := ty.(int)
		if ok && t == 1 {
			c.Next()
		} else if ok && t == 0 {
			utils.Response(c, http.StatusUnauthorized, merr.MErr("权限不足"), nil)
			c.Abort()
		}
	})

	r.POST("/remove_user", func(c *gin.Context) {
		var r rm
		c.ShouldBind(&r)
		err := db.RemoveUser(r.ID)
		utils.Response(c, 200, err, nil)
	})

	r.POST("/update_user", func(c *gin.Context) {
		var u model.User
		c.ShouldBind(&u)
		err := db.UpdateUser(u)
		utils.Response(c, 200, err, nil)
	})

	r.POST("/find_users", func(c *gin.Context) {
		var f find
		c.ShouldBind(&f)
		users, err := db.FindUsers(f.Column, "%"+f.Infix+"%")
		utils.Response(c, 200, err, gin.H{
			"users": users,
		})
	})
	r.Run(":9999")
}
