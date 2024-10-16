/*
#include <Windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <atlbase.h>  // CComPtr 智能指针
#include <regex>      // 正则表达式库
#include <chrono>     // 时间过滤

#pragma comment(lib, "Ole32.lib")

class EventHandler : public IUIAutomationEventHandler {
private:
    std::chrono::steady_clock::time_point lastEventTime;

public:
    EventHandler() {
        lastEventTime = std::chrono::steady_clock::now();
    }

    // 实现 AddRef
    ULONG STDMETHODCALLTYPE AddRef() override {
        return 1; // 简化的实现，实际项目中可以使用引用计数
    }

    // 实现 Release
    ULONG STDMETHODCALLTYPE Release() override {
        return 1; // 简化的实现，实际项目中可以使用引用计数
    }

    // 实现 QueryInterface
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IUIAutomationEventHandler)) {
            *ppv = static_cast<IUIAutomationEventHandler*>(this);
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement* pSender, EVENTID eventID) override {
        // 时间过滤：避免短时间重复触发
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEventTime).count();
        if (duration < 500) {  // 500 毫秒内的重复事件忽略
            return S_OK;
        }
        lastEventTime = now;

        // 获取窗口名称
        BSTR name;
        pSender->get_CurrentName(&name);
        // 获取窗口句柄
        HWND hWnd;
        pSender->get_CurrentNativeWindowHandle((UIA_HWND*)&hWnd);

        // 如果句柄为 NULL 或名称为空，则忽略事件
        if (hWnd == NULL || SysStringLen(name) == 0) {
            SysFreeString(name);
            //SysFreeString(className);
            return S_OK;
        }
        //std::wcout.imbue(std::locale("zh_CN"));

        // 获取窗口类名
        TCHAR className[512];
        GetClassName(hWnd, className, sizeof(className) / sizeof(TCHAR));

        // 获取窗口标题
        TCHAR windowTitle[512];
        GetWindowText(hWnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

        //int pid = 0;
        //pSender->get_CurrentProcessId(&pid);

        // 正则表达式匹配窗口标题和类名
        std::wregex namePattern(L"ProWindow");  // 匹配包含“Listary”的窗口标题
        std::wregex classPattern(L"HwndWrapper\\[Listary\\.exe;;.*\\]");  // 匹配类名

        bool isNameMatch = std::regex_match(name, namePattern);
        bool isClassMatch = std::regex_match(className, classPattern);

        //std::wcout << L"每个字符的内存存储数据: ";
        //for (const wchar_t* p = windowTitle; *p != L'\0'; ++p) {
        //    // 输出每个字符的整数表示形式
        //    std::wcout << static_cast<int>(*p) << L" ";
        //}
        //std::wcout << std::endl;
        // 格式化输出窗口信息
        std::wcout << L"[事件捕获] 窗口句柄: " << std::hex << hWnd << std::dec
            //<< L" | 标题: " << name
            << L" | 窗口标题: " << windowTitle
            << L" | 窗口类名: " << className
            << L" | 匹配标题: " << (isNameMatch ? L"是" : L"否")
            << L" | 匹配类名: " << (isClassMatch ? L"是" : L"否")
            //<< L" | PID: " << pid
            << std::endl;
        std::wcout.clear();
        // 如果匹配到目标窗口，则关闭
        if (isNameMatch && isClassMatch) {
            std::wcout << L"尝试关闭窗口: " << name << std::endl;
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }

        // 释放 BSTR 字符串
        SysFreeString(name);
        return S_OK;
    }
};

// 初始化 UI Automation 并注册事件处理器
void MonitorWindows() {
    CComPtr<IUIAutomation> pAutomation;
    CoInitialize(NULL);
    CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);

    CComPtr<IUIAutomationElement> pRootElement;
    pAutomation->GetRootElement(&pRootElement);

    // 创建 EventHandler 对象
    CComPtr<EventHandler> pHandler = new EventHandler();

    // 注册窗口打开事件 (UIA_Window_WindowOpenedEventId)
    pAutomation->AddAutomationEventHandler(
        UIA_Window_WindowOpenedEventId,
        pRootElement,
        TreeScope_Subtree,
        NULL,
        pHandler
    );

    std::wcout << L"监听窗口事件中，按 Ctrl+C 退出..." << std::endl << std::flush;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
}

int main() {
    //system("chcp 65001");
    //std::locale::global(std::locale("zh_CN.UTF-8"));
    std::wcout.imbue(std::locale("zh_CN.UTF-8"));
    MonitorWindows();
    return 0;
}
*/