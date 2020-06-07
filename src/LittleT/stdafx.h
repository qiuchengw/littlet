#ifndef stdafx_h__
#define stdafx_h__

#define SS_NO_CONVERSION

#ifdef _DEBUG
#ifdef _ENABLE_VLD
#include <vld.h>
#endif
#endif

#include <WinSock2.h>
#include <Windows.h>

#include "stdstring.h"
#include "deps/app.h"
#include "deps/inet/WinHttpClient.h"
#include "include/misc.h"

#include <atlbase.h>
#include <atlapp.h>

#endif // stdafx_h__
