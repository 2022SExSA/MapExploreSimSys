package middleware

import (
	jwt "github.com/dgrijalva/jwt-go"
	"github.com/gin-gonic/gin"
	"net/http"
	"UserServer/utils"
	"UserServer/merr"
)

var jwt_secret string = "PGZXB_MESS_USER2022SExSA"

type claims struct {
	ID       string  `json:"id"`
	Type int `json:"type"`
	jwt.StandardClaims
}

func Jwt() gin.HandlerFunc {
	return func(c *gin.Context) {
		// 获取token
		token := c.Request.Header.Get("token")
		if token == "" {
			token = c.Query("t")
			if token == "" {
				utils.Response(c, http.StatusBadRequest, merr.MErr("验证失败"), nil)
				c.Abort()
				return
			}
		}

		// 解析token
		claims, err := ParseToken(token)
		if err == nil {
			c.Set("id", claims.ID)
			c.Set("type", claims.Type)
			c.Next()
		} else {
			utils.Response(c, http.StatusUnauthorized, merr.MErr("验证失败"), nil)
			c.Abort()
		}
	}
}

func ParseToken(token string) (*claims, error) {
	tok, err := jwt.ParseWithClaims(token, &claims{}, func(t *jwt.Token) (interface{}, error) {
		return []byte(jwt_secret), nil
	})

	if err != nil {
		return nil, merr.MErr("解析Token失败")
	}

	if claims, ok := tok.Claims.(*claims); ok && tok.Valid {
		return claims, nil
	}

	return nil, merr.MErr("解析Token失败")
}
