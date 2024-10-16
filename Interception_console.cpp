/*
#include <Windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <atlbase.h>  // CComPtr ����ָ��
#include <regex>      // ������ʽ��
#include <chrono>     // ʱ�����

#pragma comment(lib, "Ole32.lib")

class EventHandler : public IUIAutomationEventHandler {
private:
    std::chrono::steady_clock::time_point lastEventTime;

public:
    EventHandler() {
        lastEventTime = std::chrono::steady_clock::now();
    }

    // ʵ�� AddRef
    ULONG STDMETHODCALLTYPE AddRef() override {
        return 1; // �򻯵�ʵ�֣�ʵ����Ŀ�п���ʹ�����ü���
    }

    // ʵ�� Release
    ULONG STDMETHODCALLTYPE Release() override {
        return 1; // �򻯵�ʵ�֣�ʵ����Ŀ�п���ʹ�����ü���
    }

    // ʵ�� QueryInterface
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IUIAutomationEventHandler)) {
            *ppv = static_cast<IUIAutomationEventHandler*>(this);
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement* pSender, EVENTID eventID) override {
        // ʱ����ˣ������ʱ���ظ�����
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEventTime).count();
        if (duration < 500) {  // 500 �����ڵ��ظ��¼�����
            return S_OK;
        }
        lastEventTime = now;

        // ��ȡ��������
        BSTR name;
        pSender->get_CurrentName(&name);
        // ��ȡ���ھ��
        HWND hWnd;
        pSender->get_CurrentNativeWindowHandle((UIA_HWND*)&hWnd);

        // ������Ϊ NULL ������Ϊ�գ�������¼�
        if (hWnd == NULL || SysStringLen(name) == 0) {
            SysFreeString(name);
            //SysFreeString(className);
            return S_OK;
        }
        //std::wcout.imbue(std::locale("zh_CN"));

        // ��ȡ��������
        TCHAR className[512];
        GetClassName(hWnd, className, sizeof(className) / sizeof(TCHAR));

        // ��ȡ���ڱ���
        TCHAR windowTitle[512];
        GetWindowText(hWnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

        //int pid = 0;
        //pSender->get_CurrentProcessId(&pid);

        // ������ʽƥ�䴰�ڱ��������
        std::wregex namePattern(L"ProWindow");  // ƥ�������Listary���Ĵ��ڱ���
        std::wregex classPattern(L"HwndWrapper\\[Listary\\.exe;;.*\\]");  // ƥ������

        bool isNameMatch = std::regex_match(name, namePattern);
        bool isClassMatch = std::regex_match(className, classPattern);

        //std::wcout << L"ÿ���ַ����ڴ�洢����: ";
        //for (const wchar_t* p = windowTitle; *p != L'\0'; ++p) {
        //    // ���ÿ���ַ���������ʾ��ʽ
        //    std::wcout << static_cast<int>(*p) << L" ";
        //}
        //std::wcout << std::endl;
        // ��ʽ�����������Ϣ
        std::wcout << L"[�¼�����] ���ھ��: " << std::hex << hWnd << std::dec
            //<< L" | ����: " << name
            << L" | ���ڱ���: " << windowTitle
            << L" | ��������: " << className
            << L" | ƥ�����: " << (isNameMatch ? L"��" : L"��")
            << L" | ƥ������: " << (isClassMatch ? L"��" : L"��")
            //<< L" | PID: " << pid
            << std::endl;
        std::wcout.clear();
        // ���ƥ�䵽Ŀ�괰�ڣ���ر�
        if (isNameMatch && isClassMatch) {
            std::wcout << L"���Թرմ���: " << name << std::endl;
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }

        // �ͷ� BSTR �ַ���
        SysFreeString(name);
        return S_OK;
    }
};

// ��ʼ�� UI Automation ��ע���¼�������
void MonitorWindows() {
    CComPtr<IUIAutomation> pAutomation;
    CoInitialize(NULL);
    CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);

    CComPtr<IUIAutomationElement> pRootElement;
    pAutomation->GetRootElement(&pRootElement);

    // ���� EventHandler ����
    CComPtr<EventHandler> pHandler = new EventHandler();

    // ע�ᴰ�ڴ��¼� (UIA_Window_WindowOpenedEventId)
    pAutomation->AddAutomationEventHandler(
        UIA_Window_WindowOpenedEventId,
        pRootElement,
        TreeScope_Subtree,
        NULL,
        pHandler
    );

    std::wcout << L"���������¼��У��� Ctrl+C �˳�..." << std::endl << std::flush;
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