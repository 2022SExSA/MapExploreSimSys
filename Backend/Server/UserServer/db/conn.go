package db

import (
	"database/sql"
	_ "github.com/mattn/go-sqlite3"
)

var conn *sql.DB

func Init(url string) {
	conn, _ = sql.Open("sqlite3", url)
	// if _, err := conn.Query("select id, password, name, type from user_table"); err != nil {
	// 	_, _ := conn.Exec(`create table user_table (id char(128) primary key, password char(512) not null, name varchar(64) not null, type int not null)`);
	// }
}
