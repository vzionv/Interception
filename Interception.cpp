#include <Windows.h>
#include <UIAutomation.h>
#include <atlbase.h>  // CComPtr 智能指针
#include <regex>      // 正则表达式库
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>      // 时间过滤
#include "resource.h"
#include <sstream>     // std::wstringstream
#if defined(_WIN64) || defined(WIN32) || defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Comctl32.lib")  // 用于托盘图标
#pragma execution_character_set("utf-8")  

#define WM_TRAYICON (WM_USER + 1)  // 托盘消息ID
#define ID_TRAY_EXIT 1001          // 托盘菜单中的退出选项ID

struct Rule {
    std::wstring processName;
    std::wregex windowTitlePattern;
    std::wregex classNamePattern;
};

std::vector<Rule> rules;
bool loggingEnabled = true; // 默认开启日志记录
std::wstring ruleFilePath = L"interception.ini"; // 默认的 ini 文件路径

#ifdef _UNICODE
std::wofstream logFile;
#else
std::ofstream logFile;
#endif

// 日志函数
void logDebug(const std::wstring& message) {
    if (loggingEnabled && logFile.is_open()) {
        logFile << std::nounitbuf << message << std::endl;
    }
}

// 托盘图标消息处理
void ShowTrayIcon(HWND hWnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wcscpy_s(nid.szTip, L"Interception");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

// 创建托盘右键菜单
void ShowTrayMenu(HWND hWnd) {
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"退出");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

// 解析命令行参数
void ParseCommandLine(LPCWSTR lpCmdLine) {
    std::wstringstream cmdStream(lpCmdLine);
    std::wstring token;
    while (cmdStream >> token) {
        if (token == L"-q") {
            loggingEnabled = false;
        }
        else if (token == L"-r") {
            if (cmdStream >> token) {
                ruleFilePath = token;
            }
        }
    }
}
// 读取规则文件
void LoadRules(const std::wstring& filePath) {
	std::wifstream file(filePath);
	std::wstring line;
	while (std::getline(file, line)) {
		std::wstringstream ss(line);
		Rule rule;
		std::wstring titleRegex, classRegex;

		ss >> rule.processName >> titleRegex >> classRegex;
		rule.windowTitlePattern = std::wregex(titleRegex);
		rule.classNamePattern = std::wregex(classRegex);
		rules.push_back(rule);
	}
}

// 自定义事件处理器
class EventHandler : public IUIAutomationEventHandler {
private:
	std::chrono::steady_clock::time_point lastEventTime;

public:
	EventHandler() {
		lastEventTime = std::chrono::steady_clock::now();
	}

	ULONG STDMETHODCALLTYPE AddRef() override { return 1; } // 简化的实现，实际项目中可以使用引用计数
	ULONG STDMETHODCALLTYPE Release() override { return 1; } // 简化的实现，实际项目中可以使用引用计数
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
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEventTime).count() < 500) {  // 500 毫秒内的重复事件忽略
			return S_OK;
		}
		lastEventTime = now;

		BSTR name;
		pSender->get_CurrentName(&name);  // 获取窗口名称
		HWND hWnd;
		pSender->get_CurrentNativeWindowHandle((UIA_HWND*)&hWnd);  // 获取窗口句柄

		// 如果句柄为 NULL 或名称为空，则忽略事件
		if (hWnd == NULL || SysStringLen(name) == 0) {
			SysFreeString(name);
			return S_OK;
		}

		// 获取窗口类名和窗口标题
		TCHAR className[512], windowTitle[512];
		GetClassName(hWnd, className, sizeof(className) / sizeof(TCHAR));
		GetWindowText(hWnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));
		std::wostringstream woss;
		bool matched = false;
		// 正则表达式匹配窗口标题和类名
		for (const auto& rule : rules) {
			if (std::regex_match(windowTitle, rule.windowTitlePattern) &&
				std::regex_match(className, rule.classNamePattern)) {
				// 将输出格式化到 wostringstream 对象中
				woss << L"[匹配捕获] 窗口句柄: " << std::hex << hWnd << std::dec
					<< L" | 窗口标题: " << windowTitle
					<< L" | 窗口类名: " << className
					<< std::endl;
				// 从 wostringstream 对象中获取 wstring
				std::wstring logMsg = woss.str();
				logDebug(logMsg);
				// 尝试关闭弹窗
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				matched = true;
			}
		}
		if (!matched) {
			woss.clear();
			woss.str(L"");
			woss << L"[事件捕获] 窗口句柄: " << std::hex << hWnd << std::dec
				<< L" | 窗口标题: " << windowTitle
				<< L" | 窗口类名: " << className
				<< std::endl;
			std::wstring logMsg = woss.str();
			logDebug(logMsg);
		}

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
		UIA_Window_WindowOpenedEventId, pRootElement, TreeScope_Subtree, NULL, pHandler);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
}


// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    switch (message) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu(hWnd);  // 右键单击时显示托盘菜单
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            //Shell_NotifyIcon(NIM_DELETE, &NOTIFYICONDATA{ sizeof(NOTIFYICONDATA), hWnd, 1 });
            PostQuitMessage(0);  // 退出消息循环
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        //Shell_NotifyIcon(NIM_DELETE, &NOTIFYICONDATA{ sizeof(NOTIFYICONDATA), hWnd, 1 });
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 程序入口
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	//std::locale::global(std::locale("zh_CN.UTF-8"));
	logFile.imbue(std::locale("zh_CN.UTF-8"));
    ParseCommandLine(lpCmdLine);  // 解析命令行参数
	if (loggingEnabled) {
		logFile.open(L"interception.log", std::ios::ate);
	}

	//char pwd[255];
	//int len = MultiByteToWideChar(CP_UTF8, 0, pwd, -1, NULL, 0);
	//wchar_t* wpwd = new wchar_t[len];
	//MultiByteToWideChar(CP_UTF8, 0, pwd, -1, wpwd, len);
	//std::wstring wstr(wpwd);
	//std::wstring logMsg = L"getcwd pwd is " + std::wstring(wstr);
	//logDebug(logMsg);
	//delete[] pwd;

	LoadRules(ruleFilePath);  // 加载规则
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"InterceptionClass";

    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(0, L"InterceptionClass", L"Interception", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd) return 0;

    ShowTrayIcon(hWnd);

	MonitorWindows();

	if (logFile.is_open()) logFile.close();
	return 0;
}
