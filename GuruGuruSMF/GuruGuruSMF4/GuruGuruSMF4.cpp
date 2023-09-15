// GuruGuruSMF4.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <Core/Ggs4Controller.h>

#ifdef _DEBUG
#pragma comment(lib, "Core_d.lib")
#pragma comment(lib, "dxmus.core_d.lib")
#else
#pragma comment(lib, "Core.lib")
#pragma comment(lib, "dxmus.core.lib")
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

#ifdef _MANAGED
#pragma managed(push, off)
#endif


using namespace GuruGuruSmf;
using namespace GuruGuruSmf::Core;



extern "C" {
	// デバイスを開く
	GGSERROR WINAPI GGS4OpenDevice(int deviceId, HWND handle)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->OpenDevice(deviceId, handle);
	}

	// デバイスを閉じる
	void WINAPI GGS4CloseDevice()
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->CloseDevice();
	}

	// DLSを追加
	GGSERROR WINAPI GGS4AddDlsW(LPCWSTR fileName)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->AddDlsW(fileName);
	}

	GGSERROR WINAPI GGS4AddDlsA(LPCSTR fileName)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->AddDlsA(fileName);
	}

	// 演奏！
	GGSERROR WINAPI GGS4Play(int option, int musicId, int seek, int fadeInTime, int keyShift)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->Play(option, musicId, seek, fadeInTime, keyShift);
	}
	
	// 演奏停止
	void WINAPI GGS4Stop(int fadeOutTime)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->Stop(fadeOutTime);
	}
	
	// 一時停止
	void WINAPI GGS4Pause()
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->Pause();
	}
		
	// 一時停止解除
	void WINAPI GGS4Restart()
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->Restart();
	}
	
	// マスターテンポ
	void WINAPI GGS4SetMasterTempo(float tempo)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->SetMasterTempo(tempo);
	}
		
	// マスターボリューム
	void WINAPI GGS4SetMasterVolume(int volume)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->SetMasterVolume(volume);
	}
	
	// マスターピッチ
	void WINAPI GGS4SetMasterPitch(int pitch)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->SetMasterPitch(pitch);
	}
	
	// 演奏情報を取得
	void WINAPI GGS4GetPlayerStatus(PlayerStatus* status, int size)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->GetPlayerStatus(status, size);
	}

	// リストにファイルからSMFを追加
	GGSERROR WINAPI GGS4AddListFromFileW(LPCWSTR fileName, int option, int musicId)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->AddListFromFileW(fileName, option, musicId);
	}

	GGSERROR WINAPI GGS4AddListFromFileA(LPCSTR fileName, int option, int musicId)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->AddListFromFileA(fileName, option, musicId);
	}
	
	// リストにメモリーからSMFを追加
	GGSERROR WINAPI GGS4AddListFromMemory(const byte* smfBinary, int length, int option, int musicId)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->AddListFromMemory(smfBinary, length, option, musicId);
	}
	
	// リスト内のアイテムを1つクリア
	void WINAPI GGS4DeleteListItem(int musicId)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->DeleteListItem(musicId);
	}

	// リストをクリア
	void WINAPI GGS4ClearList()
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		ggs->ClearList();
	}

	// リスト内の登録IDを列挙
	const SmfListKeys* WINAPI GGS4EnumList()
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->EnumList();
	}

	// SMFの情報を取得
	GGSERROR WINAPI GGS4GetSmfInformation(SmfInformation* info, int size, int musicId)
	{
		Ggs4Controller* ggs = Ggs4Controller::GetInstance();
		return ggs->GetSmfInformation(info, size, musicId);
	}

}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Ggs4Controller::Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Ggs4Controller::FreeInstance();
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

