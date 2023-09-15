// スリープ関連のメッセージを処理するウインドウ

#pragma once

#include <Windows.h>

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// スリープ関連のメッセージを処理するウインドウ
	/// </summary>
	class PowerControlWindow
	{
	private:
		HWND _hWnd;
	public:
		PowerControlWindow();	// コンストラクタ
		~PowerControlWindow();	// デストラクタ
		void createWindow();
		void MessageLoop();
		static void SetCallBackSuspend(void (*func)());
	};


}}

