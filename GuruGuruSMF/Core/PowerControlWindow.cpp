#include "stdafx.h"
#include <process.h>
#include "PowerControlWindow.h"

using namespace std;

namespace GuruGuruSmf { namespace Core {
	
	// コールバック関数置き場
	void (*callBackSuspend)() = 0;

    void thread_entry(void* p_userdata)
    {
        PowerControlWindow* p_win = (PowerControlWindow*)p_userdata;
        p_win->createWindow();
        p_win->MessageLoop();
    }
	LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// コンストラクタ
	PowerControlWindow::PowerControlWindow()
	{
		_beginthread(thread_entry, 0, this);
	}
	
	// デストラクタ
	PowerControlWindow::~PowerControlWindow()
	{
		SendMessage(_hWnd, WM_CLOSE, NULL, NULL);
	}

	const wchar_t *g_AppName  = L"GuruGuruSMFHidden";
    void PowerControlWindow::createWindow()
    {
        WNDCLASSEX wcex;

        wcex.cbSize         = sizeof(WNDCLASSEX);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = &WindowProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = GetModuleHandle(NULL);
        wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName   = NULL;
        wcex.lpszClassName  = g_AppName;
        wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

        RegisterClassEx(&wcex);

        _hWnd = CreateWindow(
			g_AppName, g_AppName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

        //ShowWindow(_hWnd, SW_SHOWDEFAULT);
        UpdateWindow(_hWnd);
    }

	void PowerControlWindow::SetCallBackSuspend(void (*func)())
	{
		callBackSuspend = func;
	}

    void PowerControlWindow::MessageLoop()
    {
        MSG msg = {0};

        while (GetMessage(&msg, NULL, 0, 0))
        {
            if(msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	
    LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_POWERBROADCAST:
			switch(wParam)
			{
			case PBT_APMSUSPEND:
				if (callBackSuspend)	callBackSuspend();
				break;
			case PBT_APMRESUMESUSPEND:
				break;
			}
			break;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}}
