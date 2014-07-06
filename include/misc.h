#ifndef misc_h__
#define misc_h__

#include "stdstring.h"

namespace littlet
{
    // ÍøÂçÇëÇó
    bool SendWebRequest(const CStdString& name, const CStdString& content);

    // mac µØÖ·
    bool GetMACAddress(CStdString& mac_addr);
}


#endif // misc_h__
