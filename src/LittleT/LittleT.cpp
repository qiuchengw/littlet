#include "LittleT.h"
#include "../common/LittleTUIcmn.h"
#include "../common/Worker.h"
#include "../common/QLongplan.h"
#include "../common/QTimerMan.h"

#include "time/QTime.h"
#include "sys/singleinst.h"
#include "deps/sys/UACSElfElevation.h"
#include "deps/img/GDIpInitializer.h"
#include "ui/QUIMgr.h"
#include "ui/QUIDlgs.h"

#include "auto_update.h"

LittleTApp _Module;

#include "LAboutDlg.h"
#include "deps/FileVersion.hpp"
#include "deps/app.h"
#include "qstrex.h"

BOOL LittleTApp::InitRun()
{
    static CSingleInstance _theInst;
    if (_theInst.InstanceAlreadyRun(LITTLET_APP_NAME))
    {
        return FALSE;
    }
    // 初始化gdiplus
    CGDIpInitializer::Get()->Initialize();

    // 有些目录是必须事先存在的
    LittleTConfig *pCfg = (LittleTConfig*)QUIGetConfig();
    // icon dir
    quibase::MakeSureDirExist(pCfg->GetIconsDir());

    SetMainWnd(&m_frame);

    // 信息读取
//     if (!QUIConnectCenter::GetInstance()->Init())
//     {
//         XMsgBox::ErrorMsgBox(L"升级信息读取失败！");
// 
//         // return false;
//     }

    if (!QWorker::GetInstance()->Startup())
    {
        XMsgBox::ErrorMsgBox(L"工作者线程启动失败!");
        return FALSE;
    }

    if (   !QDBEvents::GetInstance()->Startup()
        || !QPlanMan::GetInstance()->Startup()
        || !QResMan::GetInstance()->Startup(pCfg->GetIconsDir()))
    {
        XMsgBox::ErrorMsgBox(L"数据库未能正常加载！");
        return FALSE;
    }

    // 启动时间管理器
    if ( !QTimerMan::GetInstance()->Startup() )
    {
        XMsgBox::ErrorMsgBox(L"时间管理器启动失败！");
        return FALSE;
    }
    // 启动自动任务管理器
    if ( !QAutoTaskMan::GetInstance()->Startup() )
    {
        XMsgBox::ErrorMsgBox(L"事件管理器启动失败！");
        return FALSE;
    }
    
#ifdef _DEBUG
    if (!m_frame.Create(NULL,WS_POPUP|WS_VISIBLE))
#else   // release 最顶层
    if (!m_frame.Create(NULL,WS_POPUP|WS_VISIBLE, WS_EX_TOPMOST))
#endif
    {
        return FALSE;
    }

    // 恢复上次退出时窗口位置
    RestoreWindowPos();

    // 检查更新
    _theInst.SetInstanceMainWnd(m_frame.GetSafeHwnd());
    
    // 进程通信事件
    m_hEventMainWnd = CreateEvent(NULL, TRUE, TRUE,
        LITTLET_MAINWND_STARTUP_EVENTNAME);
    if (NULL == m_hEventMainWnd)
    {
        XMsgBox::ErrorMsgBox(L"程序启动，但是不能由命令行创建自动任务");
    }
    SetEvent(m_hEventMainWnd);
#ifdef _DEBUG
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        htmlayout::debug_output_console()
            .printf("--->LittleTFrame:the event is exist!\n");
    }
#endif

//#ifndef _DEBUG
    // 每次启动的时候都检查是否有更新
    CheckUpdate();
//#endif

    return TRUE;
}

void LittleTApp::CheckUpdaterExe()
{
    // 检查更新程序是否有新版本
    CStdString sDir = quibase::GetModulePath();
    CStdString sNewUpdater = sDir + LITTLET_UPDATER_NEWNAME;
    if ( quibase::IsFileExist( sNewUpdater ) )
    {
        // 提升自己的运行权限，以应对vista或以上系统的UAC限制
        CUACSElfElevations::SelfElevation();

        // 替换掉新的进程
        MoveFileEx(sNewUpdater, sDir + L"LittleTUpdater.exe", 
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
}

BOOL LittleTApp::CheckUpdate()
{
//     QBuffer* pBuf = NULL;
//     if (!QUIMgr::GetInstance()->LoadData(L"qabs:url.x", (QView*)nullptr, pBuf))
//     {
//         XMsgBox::ErrorMsgBox(L"版本太低啦 -_-!");
//     }
// 
//     if (NULL != pBuf)
//     {
//         // 去掉前3个字节的文件编码，剩下的就是文件内容了
//         DWORD dw;
//         pBuf->Read((PBYTE)&dw, 3);
//         CStdString sLines = ATL::CA2WEX<256>((char*)pBuf->GetBuffer());
//         CStdString sUrl = sLines, sRefer;
//         int idx = sLines.Find(L';');
//         if (-1 != idx)
//         {
//             sUrl = sLines.Left(idx);
//             sRefer = sLines.Mid(idx+1);
//         }
//         // 启动自动更新检查
//         _Url url_updation;
//         url_updation.url_ = sUrl;
//         url_updation.domain_ = sRefer;
//         // 每5个小时检查一次更新
//        return QAutoUpdater::GetInstance()->Startup(url_updation, _Url(), 4*60);
// //        return QAutoUpdater::GetInstance()->Startup();
//      }
//    return FALSE;

    // 启动自动更新检查
    _Url url_updation;
    url_updation.url_ = L"http://www.jiubaibu.com/appcast/littlet/littlet.asp";
    url_updation.domain_ = "http://www.jiubaibu.com";
    // 每2个小时检查一次更新
    return QAutoUpdater::GetInstance()->Startup(url_updation, _Url(), 2 * 60);
}

LittleTApp::~LittleTApp()
{
    // 进程即将退出了
    if (NULL != m_hEventMainWnd)
    {
        CloseHandle(m_hEventMainWnd);
        m_hEventMainWnd = NULL;
    }

    if (m_bStartUpdater)
    {
        StartUpdateExe();
    }

    CGDIpInitializer::Get()->Deinitialize();
}

LittleTApp::LittleTApp()
{
    m_bStartUpdater = FALSE;
    m_hEventMainWnd = NULL;
}

void LittleTApp::StartUpdateExe()
{
    // 找到更新程序
    CStdString sCurDir = quibase::GetModulePath();
    sCurDir.Replace(L'/',L'\\');
    CStdString sUpdateExe = sCurDir + L"LittleTUpdater.exe";
    if (!quibase::IsFileExist(sUpdateExe))
    {
        ::MessageBox(NULL, L"更新程序不见了！请重新下载完整程序 -_-!", L"错误", MB_OK);
        // 不要使用下面这句，会造成崩溃
        // XMsgBox::ErrorMsgBox(L"更新程序不见了！您都干啥了？请重新下载完整程序 -_-!");
        return;
    }

    // 创建事件，让更新进程等待
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, LITTLET_UPDATER_EVENTNAME);

    // 创建子进程
    CStdString sCmd = m_sUpdateFile;
    sCmd = L"\"" + sCmd + L"\"";    // 处理空格
    sCmd += L" ";   // 一个空格分隔参数
    sCmd += L"\"" + sCurDir + L"\\\"";  // 最后必须带两个斜杠
    ShellExecute(NULL, L"open", sUpdateExe, sCmd, 
        sCurDir + L"\\", SW_SHOWDEFAULT );

    // ok！
    SetEvent(hEvent);
    CloseHandle(hEvent);
}

CStdString LittleTApp::GetAppVersion() const
{
    // 更新历史：<br/>
    // LittleT v5.1 2015 / 7 / 8 < br / >
    // ----------------------<br / >
    // 增强：便签功能Tab键缩进<br / >
    // 增强：便签功能Ctrl + [Shift] + Tab进行便签导航<br / >
    // <br / >
    // LittleT v5.0 2015 / 6 / 11 < br / >
    // ----------------------<br / >
    // 增强：随机播放图片<br/>
    // 新增：添加便签功能 <br / >
    // <br / >
    // LittleT v4.0 2015 / 3 / 18 < br / >
    // ----------------------<br / >
    // 增强：检测提示信息是否只有一次，如果是则需要手动关闭提示，以免错过重要的提示。
    // 修改：提醒界面重新设计，简洁了一点（好像更难看了？！）。<br / >
    // 修复：计划界面下拉列表不显示滚动条的问题 <br / >
    // <br / >
    // LittleT v3.8 2014 / 12 / 6 < br / >
    // ----------------------<br / >
    // 新增：添加到任务栏快速启动<br / >
    // 修改：提醒界面重新设计，简介了一点（好像更难看了？！）。<br / >
    // 修改：已用提示音保存记录，方便下次选择。
    // 修改：多处界面微调。<br / >
    // <br / >
    // LittleT v3.6 2014/7/6<br/>
    // ----------------------<br/>
    // 新增：意见反馈<br/>
    // 修改：界面微调<br/>
    // 新增：增加了用户量统计的代码<br/>
    // <br/>
    // LittleT v3.4 2014/5/21<br/>
    // ----------------------<br/>
    // 新增：自动任务“每年的某天”执行功能<br/>
    // 修改：修改为每5个小时自动检查更新<br/>
    // 新增：我的微博连接地址<br/>
    // 
    // LittleT v3.2 2013/10/28<br/>
    // ----------------------<br/>
    // 修复：自动任务特殊情况下提示时间不正确<br/>
    // 修复：提示声音不播放/设置无法保存等问题<br/>
    // 新增：开机自启动选项<br/>
    // 增强：计划的阶段可以使用页号导航了，同时修复N个遗留问题<br/>
    // 增强：目前ctrl+shift+h显示窗口快捷键一定会呼出窗体，无论之前在哪儿显示<br/>
    // 增强：任务提示可以使用html代码。如 <b .red>提醒</b> 会被以红色粗体显示<br/>
    // <br/>
    // LittleT v3.1 2013/10/15<br/>
    // ----------------------<br/>
    // 修复：自动任务最后一次执行，界面无法正确显示下次将要执行的任务<br/>
    // 修复：界面上的一些问题修复<br/>
    // 修复：自动任务编辑后如果被执行，出现一处内存泄露<br/>
    // 修改：日历控件界面新设计<br/>
    // 修改：去掉侧边栏隐藏/显示的动画效果（因为xp下显示有卡顿）<br/>
    // 感谢：大羊、Ooo。。。嗯 反馈bug<br/>
    // <br/>
    // LittleT v3.0 2013/6/18<br/>
    // ----------------------<br/>
    // 修改：“计划”界面重新设计<br/>
    // 修复：关机任务不能被取消bug<br/>
    // <br/>
    // LittleT v2.9 2013/6/18<br/>
    // ----------------------<br/>
    // <b .red>说明：不好意思，V2.8更新的update界面出错。如果您看到这个提示，请直接按下回车键，软件即可自动升级。</b><br/>
    // 增强：记住上次退出时窗口位置，下次启动后恢复位置<br/>
    // 修改：静默更新，不再显示更新提示<br/>
    // 修复：通过其他软件调用产生“配置文件读取失败”问题<br/>
    // <br/>
    // LittleT v2.8 2013/6/8<br/>
    // ----------------------<br/>
    // 添加：程序设置对话框<br/>
    // 修改：自动任务的时间可手动填写。<br/>
    // 增强：最小化到系统托盘时显示气泡提示<br/>
    // <br/>
    // LittleT v2.7 2013/6/3<br/>
    // ----------------------<br/>
    // 修复：<b .red>实在抱歉，因时间太少，代码维护没做到位。
    // 导致本程序的v2.6版本不能创建自动任务。此版本仅为修复这一个低级错误。
    // 如您在使用过程中发现bug，请及时提交给我。感激不尽。</b><br/>
    // <br/>
    // LittleT v2.6 2013/6/1<br/>
    // ----------------------<br/>
    // 增强：ESC键退出全屏动画<br/>
    // 修改：改小了header的字体<br/>
    // 修复：“绝对时间执行”->“每周的某天”参数错误。感谢李雷提交bug<br/>
    // 修复：全局快捷键新建todo任务，不能输入文字问题，感谢袁飞提交bug<br/>
    // <br/>
    // LittleT v2.5 2013/5/14<br/>
    // ----------------------<br/>
    // 增强："自动任务-执行程序" 可以手动输入网址了<br/>
    // 增强：自动更新提示对话框显示详细更新内容<br/>
    // 修改：若干界面细节调整<br/>
    // 修改：去掉任务栏图标<br/>
    // 修改：托盘区图标只有在程序完全隐藏的时候才会显示<br/>
    // 修复：自动更新程序不能删除备份文件问题<br/>
    // 修复："自动任务->绝对时间->单个日期"执行日期解析错误<br/>
    // <br/>
    // LittleT v2.4	2013/5/10<br/>
    // ----------------------<br/>
    // 增加：系统托盘区图标<br/>
    // 增加：命令行方式创建自动任务<br/>
    // 增加：靠边自动隐藏<br/>
    // 修复：自动更新程序缺少DLL问题<br/>
    // 修改：TODO移除“立即完成”功能<br/>
    // 修改：界面细微调整<br/>
    // <br/>
    // LittleT v2.2	2013/4/25<br/>
    // ----------------------<br/>
    // 修改：“计划”中不能正确选中stage项目<br/>
    // 增强：增加了release下面调试代码<br/>
    // <br/>
    // LittleT v2.1	2013/4/22<br/>
    // ----------------------<br/>
    // 修改：自动更新功能调整为每30分钟检查一次<br/>
    // 修改：界面细节调整，准备发布<br/>
    // 修改：自动更新功能调整为每30分钟检查一次<br/>
    // 修改：界面细节调整，准备发布<br/>
    // <br/>
    // LittleT v2.0	2013/4/18<br/>
    // ----------------------<br/>
    // 增加：“计划”功能<br/>
    // 增加：TODO立即完成倒计时<br/>
    // 增加：程序自动更新功能<br/>
    // 修改：界面重新设计<br/>
    // 修复：若干个小bug<br/>
    // <br/>
    // LittleT v1.2	2013/3/18<br/>
    // ----------------------<br/>
    // 增强：界面加载代码重构，运行速度倍增！<br/>
    // 修复：点击任务栏图标程序不响应消息<br/>
    // 修复：不按下ctrl依然能使用快捷键的问题<br/>
    // 修复：关机任务实际为重启的bug<br/>
    // 修改：跳过自动任务时不再提示<br/>
    // <br/>
    // LittleT v1.1	2013/3/1<br/>
    // ----------------------<br/>
    // 新增：绝对时间任务多时间点执行<br/>
    // 修改：重新设计了“新建自动任务”的界面，现在应该更清晰漂亮些了<br/>
    // <br/>
    // LittleT v1.0	2013/2/16<br/>
    // ----------------------<br/>
    // 第一个版本

    return FileVersion().GetMajorMinorVersion();
}

//////////////////////////////////////////////////////////////////////////
LittleTConfig _Config;

std::vector<CStdString> LittleTConfig::GetHistorySoundFile()
{
    CStdString s_files = GetValue(L"Sound", L"file");
    std::vector<CStdString> ret;
    QStrEx(L'*', s_files).GetAll(ret);
    return ret;
}

std::vector<CStdString> LittleTConfig::AddSoundFilePath(const CStdString& file)
{
    auto files = GetHistorySoundFile();
    QStrEx se(L'*', L"");
    for (auto &s : files)
    {
        se.AddString(s);
    }
    se.AddString(file);
    se.Unique();
    SetValue(L"Sound", L"file", se.Commbine());

    return GetHistorySoundFile();
}
