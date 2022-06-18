package db

import (
	"database/sql"
	_ "github.com/mattn/go-sqlite3"
)

var conn *sql.DB

func Init(url string) {
	conn, _ = sql.Open("sqlite3", url)
}
