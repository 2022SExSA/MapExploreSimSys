package model

const (
	UserType = iota
	AdminType
)

type User struct {
	ID       string `json:"id"`
	Password string `json:"password"`
	Name     string `json:"name"`
	Type     int    `json:"type"`
}
