
#include "stdafx.h"

#include "Thread.h"

namespace GuruGuruSmf { namespace Core {

	// コンストラクタ
	Thread::Thread()
	{
		handle = 0;
	}

	// デストラクタ
	Thread::~Thread()
	{
		if (handle == 0) return;

		DWORD param;
		GetExitCodeThread(handle , &param);
		if (param == STILL_ACTIVE) {
			TerminateThread(handle , FALSE);
			WaitForSingleObject(handle, INFINITE);
		}
		CloseHandle(handle);
	}


	// スレッド開始
	GGSERROR Thread::Start(void* object, LPTHREAD_START_ROUTINE procedure, int priority)
	{
		if(handle) return GgsError::Failed;	// 前のスレッドオブジェクトがまだ残っている

		DWORD id;
		handle = CreateThread(NULL, 0, procedure, object, CREATE_SUSPENDED, &id);
		if(handle == 0) return GgsError::Failed;	// 失敗

		SetThreadPriority(handle, priority);
		ResumeThread(handle);
		return GgsError::NoError;
	}

	// 終了待ち
	void Thread::Join()
	{
		if(handle == 0) return;

		DWORD param;
		GetExitCodeThread(handle , &param);
		if(param == STILL_ACTIVE){
			WaitForSingleObject(handle, INFINITE);
		}

		CloseHandle(handle);
		handle = 0;
	}




}}


