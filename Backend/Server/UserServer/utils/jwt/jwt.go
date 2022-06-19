package jwt

import (
	"github.com/dgrijalva/jwt-go"
)

var jwt_secret string = "PGZXB_MESS_USER2022SExSA"

type claims struct {
	ID       string  `json:"id"`
	Type int `json:"type"`
	jwt.StandardClaims
}
