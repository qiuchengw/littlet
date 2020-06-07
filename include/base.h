#ifndef base_h__
#define base_h__

#include <functional>

/**
 *  资源管理： 申请和释放保护  
 *
 *  实现摘录自：C++罗浮宫
 *
 *      http://mindhacks.cn/2012/08/27/modern-cpp-practices/
 *  
**/
class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> onExitScope)
        : onExitScope_(onExitScope), dismissed_(false)
    { }

    ~ScopeGuard()
    {
        if (!dismissed_)
        {
            onExitScope_();
        }
    }

    void Dismiss()
    {
        dismissed_ = true;
    }

private:
    std::function<void()> onExitScope_;
    bool dismissed_;

private: // noncopyable
    ScopeGuard(ScopeGuard const&);
    ScopeGuard& operator=(ScopeGuard const&);
};

// HANDLE h = CreateFile(...);
// ScopeGuard onExit([&] { CloseHandle(h); });

#define SCOPEGUARD_LINENAME_CAT(name,line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

#endif // base_h__
