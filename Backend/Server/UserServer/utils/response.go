package utils

import (
	"github.com/gin-gonic/gin"
)

type RespMsg struct {
	Code int         `json:"code"`
	Msg  string      `json:"msg"`
	Data interface{} `json:"data"`
}

func Response(c *gin.Context, httpCode int, err error, data interface{}) {
	code := -1
	if err == nil {
		code = 0
	}
	msg := ""
	if err != nil {
		msg = err.Error()
	}
	c.JSON(httpCode, RespMsg{
		code,
		msg,
		data,
	})
}
