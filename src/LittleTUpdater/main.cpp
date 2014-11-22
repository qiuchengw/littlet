#include <iostream>
#include <vector>

// 隐藏console窗口
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"wmainCRTStartup\"") 

#include "../common/LittleTcmn.h"

#include "QBuffer.h"
#include "crypt/CXUnzip.h"
#include "deps/sys/UACSElfElevation.h"

#include <atlbase.h>
#include <atlstr.h>

using namespace std;

// #ifndef _DEBUG
// #define  _DEBUG   1
// #endif
#ifdef _DEBUG
#pragma comment(lib, "quil_mtd")
#else
#pragma comment(lib, "quil_mt")
#endif

BOOL IsFileExist(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    if( pszFile == NULL )
        return bRet;
    if( pszFile[0] == 0 )
        return bRet;

    WIN32_FIND_DATA fd = {0};
    HANDLE hFile = FindFirstFile(pszFile, &fd);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        if( !(fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) )
            bRet = TRUE;
    }
    return bRet;
}

struct _Files 
{
    wstring     sdest;
    wstring     sbackup; // 备份文件
    wstring     ssrc;   // 源文件
    QBuffer     buf;
};

typedef std::vector<_Files*> LstFiles;

/** 解压压缩包内的所有数据并保存到链表
 *	return:
 *      true    成功
 *	params:
 *		-[in]
 *          szZip   zip文件
 *          lst     文件信息
**/
bool ExtractAll(__in LPCWSTR szZip, __out LstFiles& lst)
{
    lst.clear();
    // 解压压缩包
    CXUnzip theZip;
    if ( !theZip.Open(szZip) )
    {
        return false;
    }

    bool bRet = true;
    // 遍历zip包里面的文件
    ZIPENTRYW ze;
    for (int i = 0; i < theZip.GetItemCount(); ++i)
    {
        if (!theZip.GetItemInfo(i,&ze))
            continue;

//         if (StrCmpIW(LITTLET_UPDATE_CONTENT_FILENEWNAME, ze.name) == 0)
//             continue;   // 更新内容，不必写

        _Files* p = new _Files;
        p->sdest = ze.name;
        if ( !theZip.UnzipToBuffer(i, p->buf) )
        {
            delete p;
            bRet = false;
            break;
        }
        lst.push_back(p);
    }
    if (!bRet)
    {
        for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
        {
            delete *i;
        }
        lst.clear();
    }
    theZip.Close();
    return bRet;
}

bool UpdateFiles(__in LstFiles& lst, __in wstring sDest)
{
    bool bRet = true;
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        _Files* p = *i;
        p->ssrc = sDest;
        p->ssrc += p->sdest;
        p->sbackup = p->ssrc;
        p->sbackup += L".__bak__";
        // 先备份这个文件
        if (!IsFileExist(p->ssrc.c_str()))
        {
            // 原来不存在的文件，就直接拷贝新文件就完成了
            p->sbackup.clear();
        }
        else
        {
            // 已有的文件需要先备份下
            // 如果是更新自己“LittleTUpdater.exe”, 那就直接配合主进程LittleT来更新自己
            // 只需要把自己写到当前文件夹下面的就行了
            if (StrCmpIW(p->sdest.c_str(), L"LittleTUpdater.exe") == 0)
            {
                // 主进程“LittleT.exe”启动的时候会检查有没有这个文件
                // 有的话，就会用它替换掉当前的LittleTUpdater.exe
                p->buf.FileWrite(LITTLET_UPDATER_NEWNAME);
                continue;
            }
            // 如果不是更新“更新程序”，那么先备份
            if (!MoveFileEx(p->ssrc.c_str(), p->sbackup.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
#ifdef _DEBUG
                wcout<<"backup File：["
                    <<p->ssrc.c_str()<<"] to ["
                    <<p->sbackup.c_str()<<"] failed!"<<endl;
#endif
                p->ssrc.clear();
                p->sbackup.clear();
                bRet = false;
                break;
            }
        }
        // 把文件内容写到本地
        if ( !p->buf.FileWrite(p->ssrc.c_str()) )
        {
#ifdef _DEBUG
            wcout<<"Write File：["<<p->ssrc.c_str()<<"] failed!"<<endl;
#endif
            bRet = false;
            break;
        }
    }
    return bRet;
}

// 操作没完成，从备份中恢复原来的文件
bool RestoreFile(__in LstFiles& lst)
{
    bool bRet = true;
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        _Files* p = *i;
        if ( !p->sbackup.empty() )
        {
            // 试3次恢复
            int j = 0;
            while (j < 3)
            {
                if (MoveFileEx(p->sbackup.c_str(), p->ssrc.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                {
                    break;
                }
                j++;
            }
            if (j == 3)
            {
                // fuck!
                // 程序应该被破坏了
                bRet = false;
            }
            else
            {
                p->sbackup.clear();
                p->ssrc.clear();
            }
        }
    }
    return bRet;
}

// 文件更新成功，备份文件可以删除了
void DeleteBackup(__in LstFiles& lst)
{
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        if (!(*i)->sbackup.empty())
        {
            ::DeleteFile((*i)->sbackup.c_str());
        }
    }
}

// argv[1]  zip path
// argv[2]  dest path
int wmain(int nArgc, wchar_t**argv)
{
#ifdef _DEBUG
    for (int i = 0; i < nArgc; i++)
    {
        wcout<<L"arg "<<i<<":  "<<argv[i]<<endl;
    }
#endif

    if (nArgc < 3)
    {
        wcout<<"参数错误！更新失败"<<endl;
        return -1;
    }

    wcout<<"正在更新，请稍等片刻，不要关闭这个窗口，应该不会超过10秒的....."<<endl;
    // 确保主进程已经退出了
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, LITTLET_UPDATER_EVENTNAME);
    if (NULL != hEvent)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // 等待这个事件变为有信号再执行
            WaitForSingleObject(hEvent,3000);
        }
        CloseHandle(hEvent);
    }
    // 释放到目的路径
    wstring sZip = argv[1];
    wstring sDestDir = argv[2];
    bool bFailed = false;
    do 
    {
        // 解压所有文件
        LstFiles lst;
        //    wstring szip = L"f:\\UI\\ddd\\UI.zip";
        wcout<<"解压文件....."<<endl;
        if ( !ExtractAll(sZip.c_str(), lst) )
        {
            wcout<<"解压文件错误，更新失败"<<endl;
            bFailed = true;
            break;
        }

        // 提升自己的运行权限，以应对vista或以上系统的UAC限制
        CUACSElfElevations::SelfElevation();

        // 开始替换文件
        wcout<<"更新程序....."<<endl;
        if ( !UpdateFiles(lst, sDestDir))
        {
            wcout<<"晕，不好意思，更新文件失败！"<<endl;
            if (!RestoreFile(lst))
            {
                wcout<<"-_-! 更不好意思了，不能恢复到原来状态！"<<endl;
                wcout<<"请手动去掉__bak__扩展名，如果不会-->问问懂电脑的怎么做"<<endl;
                // fuuuuuuuuuuuuuuuuuuck!!!!!!
            }
            bFailed = true;
        }
        else
        {
            // ok，更新成功
            // 删除备份文件
            DeleteBackup(lst);
        }

        // 清理资源
        for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
        {
            delete *i;
        }
        lst.clear();
    } while (false);

    // 更新的中间文件也删除吧
    DeleteFile(sZip.c_str());

    if (bFailed)
    {
        system("pause");
    }
    else
    {
        wcout<<"搞定了..."<<endl;
    }

    // 无论如何，试试重新启动主程序
    wstring sfile = sDestDir;
    sfile += L"LittleT.exe";
    ShellExecute(NULL,L"open", sfile.c_str(), NULL, sDestDir.c_str(), SW_SHOWNORMAL);

    return 0;
}

