#pragma once

#include "ConstValues.h"
#include "basetype.h"

// 更新程序和主程序之间的协同事件名
#define LITTLET_UPDATER_NEWNAME L"LittleTUpdater.exe.new"

// 本次更新内容文本文件名
//#define LITTLET_UPDATE_CONTENT_FILENEWNAME L"history.txt"
#define LITTLET_UPDATE_HISTORY_FILENEWNAME L"history.txt"

// 更新程序和主程序之间的协同事件名
#define LITTLET_UPDATER_EVENTNAME L"QSOFT_LITTLET_UPDATER"

// LittleT主进程和Shell进程间通信的管道名称
#define LITTLET_SHELL_PIPLENAME L"\\\\.\\pipe\\LittleT_Shell_PipelName"

// LittleT主进程和Shell进程间通信的管道 buffer大小
const int   G_LITTLET_SHELL_PIPEL_BUFFER_SIZE = 4096;

//应用程序名称，系统唯一，每次只能运行一个实例
#define LITTLET_APP_NAME L"QSoft_LittleT"

//LIttleT主窗口启动后，将会创建一个事件，
// 其它进程通过检查此事件是否存在来判断主进程的主窗口是否已经创建完成
#define LITTLET_MAINWND_STARTUP_EVENTNAME L"QSoft_LittleT_MainWndEvent"

// LITTLET_SHELL_PIPLENAME
typedef struct _LittleTShell_CopyData_Param
{
    ENUM_AUTOTASK_DOWHAT eDoWhat; 
    wchar_t     szDoWhatParam[256];
    wchar_t     szWhenDo[256];
    wchar_t     szRemindExp[256];
    wchar_t     szResult[256];
    double      tmBegin;
    double      tmEnd;
}*LP_LittleTShell_CopyData_Param;

// littleTShell 和 LittleT之间的通信消息
//  消息 WM_COPYDATA 的wParam 为此枚举中的一个值 
enum ENUM_LITTLET_SHELL_COMMANDCODE
{
    LITTLET_SHELL_COMMANDCODE_ADDAUTOTASK = 1, 
};

// littleTShell 和 LittleT之间的通信消息
//  消息 WM_COPYDATA 的返回值，需要通过SendMessage发送消息 
enum ENUM_LITTLET_SHELL_COMMANDRESULT
{
    LITTLET_SHELL_COMMANDRESULT_ERROR = 0,  // 错误
    LITTLET_SHELL_COMMANDRESULT_OK = 1, // 成功处理
};
