#ifndef __SYNCINVOKE_H__
#define __SYNCINVOKE_H__

#include "atlbase.h"

namespace SyncInvoker
{
	class Thread
	{
		public :
			Thread(UINT uThrdDeadMsg = 0) 
                : m_uThreadDeadMsg(uThrdDeadMsg) 
            {

            }
			
            virtual ~Thread() 
            {
                CloseHandle(m_hThread);
            }

			virtual Thread &Create(LPSECURITY_ATTRIBUTES lpThreadAttributes = 0, 
                DWORD dwStackSize = 0, DWORD dwCreationFlags = 0, UINT uThrdDeadMsg = 0)
			{	
				if (uThrdDeadMsg) 
                    m_uThreadDeadMsg = uThrdDeadMsg;
				
                m_dwCreatingThreadID = GetCurrentThreadId();
//                 m_hThread = CreateThread(lpThreadAttributes, dwStackSize, ThreadProc, 
//                     reinterpret_cast<LPVOID>(this), dwCreationFlags, &m_dwThreadId); 
                // qiuchengw modify
                m_hThread = (HANDLE)_beginthreadex(lpThreadAttributes, dwStackSize, ThreadProc, 
                    reinterpret_cast<LPVOID>(this), dwCreationFlags, &m_dwThreadId); 

				return *this; 
			}

			bool Valid() const 
            {
                return m_hThread != NULL;
            }

			DWORD ThreadId() const 
            {
                return m_dwThreadId;
            }

			HANDLE ThreadHandle() const 
            {
                return m_hThread;
            }

		protected :
			virtual DWORD ThreadProc() {return 0;}

			UINT m_dwThreadId;
			HANDLE m_hThread;
			DWORD m_dwCreatingThreadID;
			UINT m_uThreadDeadMsg;

			static UINT_PTR WINAPI ThreadProc(LPVOID pv)
			{
				if (!pv) 
                    return 0;

				DWORD dwRet = reinterpret_cast<Thread*>(pv)->ThreadProc(); 
				if (reinterpret_cast<Thread*>(pv)->m_uThreadDeadMsg) 
                {
                    PostThreadMessage(reinterpret_cast<Thread*>(pv)->m_dwCreatingThreadID, 
                        reinterpret_cast<Thread*>(pv)->m_uThreadDeadMsg, 
                        0, dwRet); 
                }
				return dwRet; 
			}
	};

	template <class T_OWNER>
	class CSyncCall : public Thread
	{
	public:
		CSyncCall():m_owner(NULL), m_method(NULL),m_param(0){}

		typedef LRESULT (T_OWNER::*METHOD_PTR)(LPVOID);

		bool Call(T_OWNER *owner,METHOD_PTR method,LPVOID param) 
		{
			m_owner = owner;
			m_method = method;
			m_param = param;
			return Create().Valid();
		}

		virtual DWORD ThreadProc()
		{
			if(m_owner)
            {
                m_callResult = (m_owner->*m_method)(m_param);
                return m_callResult;
            }
			return 0;
		}

        LRESULT ResultCall()const
        {
            return m_callResult;
        }

	private:
		T_OWNER *m_owner;
		METHOD_PTR m_method;
		LPVOID m_param;
        LRESULT m_callResult;
	};
}

// retVal 调用的返回值
#define SyncInvoke(cls, method, param, retVal) \
{ \
	SyncInvoker::CSyncCall<cls> syncCall; \
	syncCall.Call(this, &cls::method, param); \
	AtlWaitWithMessageLoop(syncCall.ThreadHandle()); \
    retVal = syncCall.ResultCall();\
}
#endif
