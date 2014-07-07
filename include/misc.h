#ifndef misc_h__
#define misc_h__

#include "stdstring.h"

namespace littlet
{
    /**
     *	异步的网络请求	
     *
     *  name    服务名： login / logout / feed
     *  bAsync  TRUE    异步网络
    **/
    bool SendWebRequest(const CStdString& name, const CStdString& content, BOOL bAsync = TRUE);

    // mac 地址
    bool GetMACAddress(CStdString& mac_addr);
}


#endif // misc_h__
