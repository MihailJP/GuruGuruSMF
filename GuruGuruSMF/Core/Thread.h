// スレッド(今のところDirectMusic Portモードのみ使用)

#pragma once

#include <Windows.h>
#include "Constants.h"

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// スレッドクラス
	/// </summary>
	class Thread
	{
	private:
		HANDLE handle;


	public:
		Thread();	// コンストラクタ
		~Thread();	// デストラクタ

		// スレッド開始
		GGSERROR Start(void* object, LPTHREAD_START_ROUTINE procedure, int priority);

		// 終了待ち
		void Join();
	};

}}

