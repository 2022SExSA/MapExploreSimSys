package merr

type MessErr struct {
	msg string
}

func (err MessErr) Error() string {
	return err.msg
}

func MErr(msg string) MessErr {
	return MessErr{msg: msg}
}
