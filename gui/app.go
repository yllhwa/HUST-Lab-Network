package main

import (
	"context"
	"os"
	"strings"
	"syscall"
	"unsafe"

	"github.com/wailsapp/wails/v2/pkg/runtime"
)

var (
	// function: set_remote_ip, get_file, put_file
	set_remote_ip uintptr
	get_file      uintptr
	put_file      uintptr
)

// App struct
type App struct {
	ctx context.Context
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx
	// load tftp_client.dll
	dllRef := syscall.MustLoadDLL("tftp_client.dll")
	// get function address
	set_remote_ip = dllRef.MustFindProc("set_remote_ip").Addr()
	get_file = dllRef.MustFindProc("get_file").Addr()
	put_file = dllRef.MustFindProc("put_file").Addr()
	println("set_remote_ip:", set_remote_ip)
	println("get_file:", get_file)
	println("put_file:", put_file)
}

func (a *App) SetRemoteIp(ip string) {
	// call set_remote_ip
	// set_remote_ip(char *ip);
	syscall.Syscall(set_remote_ip, 1, uintptr(unsafe.Pointer(syscall.StringBytePtr(ip))), 0, 0)
}

func (a *App) GetFile(filename, local_name string, mode int) int {
	// call get_file
	// int get_file(char *filename, char *local_name, int mode)
	// x64 dll
	ret, _, _ := syscall.Syscall(get_file, 3, uintptr(unsafe.Pointer(syscall.StringBytePtr(filename))), uintptr(unsafe.Pointer(syscall.StringBytePtr(local_name))), uintptr(mode))
	println("ret:", ret)
	return int(ret)
}

func (a *App) PutFile(filename, local_name string, mode int) int {
	// call put_file
	// int put_file(char *filename, char *local_name, int mode)
	ret, _, _ := syscall.Syscall(put_file, 3, uintptr(unsafe.Pointer(syscall.StringBytePtr(filename))), uintptr(unsafe.Pointer(syscall.StringBytePtr(local_name))), uintptr(mode))
	println("ret:", ret)
	return int(ret)
}

func (a *App) changeWorkingDir(path string) {
	os.Chdir(path)
}

func (a *App) SelectWorkingDir() string {
	// OpenDirectoryDialog
	path, err := runtime.OpenDirectoryDialog(a.ctx, runtime.OpenDialogOptions{
		Title: "选择目录",
	})
	if err != nil {
		println("OpenDirectoryDialog error:", err)
		return ""
	}
	println("path:", path)
	a.changeWorkingDir(path)
	return path
}

func (a *App) GetCurrentDir() string {
	dir, err := os.Getwd()
	if err != nil {
		println("GetCurrentDir error:", err)
		return ""
	}
	return dir
}

func (a *App) ReadLogFile() string {
	// 读取文件
	file, err := os.Open("log.txt")
	if err != nil {
		println("ReadLogFile error:", err)
		return ""
	}
	defer file.Close()
	// 获取文件大小
	fileInfo, err := file.Stat()
	if err != nil {
		println("ReadLogFile error:", err)
		return ""
	}
	fileSize := fileInfo.Size()
	// 读取文件
	buffer := make([]byte, fileSize)
	_, err = file.Read(buffer)
	if err != nil {
		println("ReadLogFile error:", err)
		return ""
	}
	return string(buffer)
}

func (a *App) GetLastError() string {
	log_content := a.ReadLogFile()
	if log_content == "" {
		return ""
	}
	// 分行
	lines := strings.Split(log_content, "\n")
	println("GetLastError()")
	// 倒序查找
	for i := len(lines) - 1; i >= 0; i-- {
		line := lines[i]
		if strings.Contains(line, "ERROR") {
			println("last error:", line)
			return line
		}
	}
	return ""
}

func (a *App) GetLastSpeed() string {
	log_content := a.ReadLogFile()
	if log_content == "" {
		return ""
	}
	// 分行
	lines := strings.Split(log_content, "\n")
	println("GetLastSpeed()")
	// 倒序查找
	for i := len(lines) - 1; i >= 0; i-- {
		line := lines[i]
		if strings.Contains(line, "speed:") {
			// 取出speed:后面的内容
			speed := line[strings.Index(line, "speed:")+6:]
			println("last speed:", speed)
			return speed
		}
	}
	return ""
}
