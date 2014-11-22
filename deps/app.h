#ifndef _app_h__
#define _app_h__

#pragma once

#include "AppHelper.h"

namespace quibase
{
    
    /**
     *	确保文件目录路径存在，不存在则创建	
     *
    **/
	BOOL MakeSureDirExist(LPCWSTR sDir);

    /**
     *	解析字符串到GUID	
     *
    **/
	BOOL GuidFromString(LPCTSTR szGuid, __out GUID& guid);

    /**
     *	判断是否是GUID
     *
    **/
	BOOL IsGuid(LPCTSTR szGuid);

    
    /**
     *	GUID转字符串
     *
    **/
	BOOL GuidToString(const GUID& guid, __out CStdString& sGuid);

    /**
     *	GUID是否空
     *
    **/
	BOOL GuidIsNull(const GUID& guid);

    /**
     *	返回空GUID
     *
    **/
	void NullGuid(__out GUID& guid);
	
    /**
     *	GUID相同
     *
    **/
    BOOL SameGuids(const GUID& guid1, const GUID& guid2);

    /**
     *	是否桌面被锁定了
     *
    **/
	BOOL IsWorkStationLocked();

    /**
     *	是否正在屏保模式下
     *
    **/
	BOOL IsScreenSaverActive();

    /**  获取特殊路径
     *	return
     *      TRUE    获取成功
     *	param
     *		-[in]
     *          csidl       例如：CSIDL_STARTUP
     *		-[out]
     *			sPath       获取到的路径
    **/
    BOOL GetSpeialPath(__in int csidl, __out CStdString &sPath);

    /**
     *	是否开机自动运行	
     *
    **/
    bool IsAutoRun();               
    
    /**
     *	创建开机自动运行	
     *
    **/
    void CreateAutoRun();           
    
    /**
     *	移除开机自动运行	
     *
    **/
    void RemoveAutoRun();           
    
    /**
     *	设置开机自动运行	
     *
    **/
    void SetAutoRun(bool bAutoRun); 

    /**
     *	提升权限	
     *
    **/
    BOOL UpgradeProcessPrivilege(); 
    
    /**
     *	较对系统时间	
     *
    **/ 
    int  CorrectTime();             
    
    /**
     *	较对时间，通过提升权限以加参数的方式运行程序实现	
     *
    **/ 
    int  SyncTime();        

    /**
     *	把资源文件释放出来	
     *
    **/ 
    BOOL ReleaseRes(LPCTSTR filename,WORD wResID, LPCTSTR filetype);  

    /**
     *	用默认浏览器打开，无默认时用IE打开	
     *
    **/ 
    void VisitWebsiteWithDefaultBrowser(LPCWSTR lpszUrl);             

    /**
     *	url是否为ie首页	
     *
    **/ 
    bool IsIEHomePage(LPCTSTR url);   
    
    /**
     *	设置ie首页, 如果已经是首页，则不重复创建
     *
    **/ 
    bool SetIEHomePage(LPCTSTR url);  

    /**
     *	快速启动
     *
    **/ 
    void PinToTaskbar(LPCTSTR lpszDestPath);        

    /**
     *	删除快速启动
     *
    **/ 
    void UnPinFromTaskbar(LPCTSTR lpszDestPath);    

    /**
     *	创建桌面快捷
     *      lpszProgram     为NULL代表创建调用者的路径。
     *                      ！！！调用者可能为DLL
    **/
    BOOL CreateShortcut(LPCSTR lpszDestPath, LPCTSTR lpszProgram, LPCTSTR lpszIco,
        LPCTSTR lpszArguments, LPCTSTR lpszWorkingDir, LPCTSTR lpszDescription);

    /**
     * 是否是PE文件		
     *
    **/
    BOOL IsPEFile(LPCTSTR lpszPath);
};


#endif // apphelper_h__
