#pragma once


typedef struct TASK_REMINDER_PARAM 
{
    int		nSeconds; // 提前多少秒提醒
    int		nTaskID; // 任务id
    QTime	tmExec; // 任务的执行时间
    CStdString sSound;	// 声音文件路径
    CStdString sMsg; // 提示信息
}*LPTASK_REMINDER_PARAM;

class QTimerEventHandlerBase
{
public:
    virtual void OnTimerReminderSetted(TASK_REMINDER_PARAM* pParam)
    {

    }
};

