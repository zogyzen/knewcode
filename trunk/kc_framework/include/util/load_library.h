#pragma once

#ifdef WIN32    // Windows环境
    #include <windows.h>
	constexpr const char c_LoadOutLibExtName[] = ".dll";
#else           // linux环境
    #include <dlfcn.h>
	constexpr const char c_LoadOutLibExtName[] = ".so";
#endif

#include <string>
#include <memory>
using namespace std;

namespace KC
{
    template<bool bDefLoad = false>
    class KLoadLibrary
    {
    public:
        // 构造
        KLoadLibrary(const char* strLibFileName, string& sError) : m_strLibFileName(strLibFileName)
        {
            sError = "";
            m_strLibFileName += m_LibExtName;
            if (bDefLoad) sError = LoadLib();
        }
        KLoadLibrary(const char* strLibFileName) : m_strLibFileName(strLibFileName)
        {
            m_strLibFileName += m_LibExtName;
            if (bDefLoad) LoadLib();
        }
        // 卸载库文件
        ~KLoadLibrary()
        {
            #ifdef WIN32    // Windows环境
            if (0 != m_lib_handle) ::FreeLibrary(m_lib_handle);
            #else          // linux环境
            if (nullptr != m_lib_handle) dlclose(m_lib_handle);
            #endif
        }
        // 加载动态库
        string LoadLib(void)
        {
            string sError = "";
            #ifdef WIN32    // Windows环境
            if (0 == m_lib_handle)
            {
                m_lib_handle = ::LoadLibrary(m_strLibFileName.c_str());
                if (0 == m_lib_handle)
                    sError = m_strLibFileName + " (Can't load lib file)";
            }
            #else           // linux环境
            if (nullptr == m_lib_handle)
            {
                m_lib_handle = dlopen(m_strLibFileName.c_str(), RTLD_LAZY);
                if (nullptr == m_lib_handle)
                    sError = m_strLibFileName + " [Can't load lib file]" + dlerror();
            }
            #endif
            return sError;
        }
        // 加载库函数
        template<typename _Func>
        inline _Func GetLibFunc(const char* strFuncName, string& sError)
        {
            sError = LoadLib();
            _Func _f = (_Func)GetLibFunc(strFuncName);
            if (sError.empty() && _Func() == _f)
                sError = m_strLibFileName + "::" + strFuncName + " (Can't get lib function)";
            return _f;
        }

        template<typename _Func>
        inline _Func GetLibFunc(const char* strFuncName)
        {
            string sError;
            return this->GetLibFunc<_Func>(strFuncName, sError);
        }
        void* GetLibFunc(const char* strFuncName)
        {
            void* _f = nullptr;
            #ifdef WIN32    // Windows环境
            if (0 != m_lib_handle) _f = (void*)::GetProcAddress(m_lib_handle, strFuncName);
            #else           // linux环境
            if (nullptr != m_lib_handle) _f = (void*)dlsym(m_lib_handle, strFuncName);
            #endif
            return _f;
        }
        // 返回库文件名
        const char* GetLibFileName(void) const { return m_LibExtName; }
        // 最近访问时间
        void SetLastTime(void) { m_lastTime = std::time(0); }
        time_t GetLastTime(void) { return m_lastTime; }

    private:
        // 库实例句柄
        #ifdef WIN32    // Windows环境
        HINSTANCE m_lib_handle = 0;
        #else           // linux环境
        void* m_lib_handle = nullptr;
        #endif
		// 库扩展名
        const char* m_LibExtName = c_LoadOutLibExtName;
        // 库文件全名
        string m_strLibFileName = "";
        // 最近访问时间
        time_t m_lastTime = std::time(0);
    };
    typedef std::shared_ptr<KLoadLibrary<>> TKLoadLibraryPtr;
}
