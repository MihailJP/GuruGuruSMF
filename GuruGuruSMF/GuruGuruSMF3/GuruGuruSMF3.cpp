// GuruGuruSMF3.cpp : DLL アプリケーションのエントリ ポイントを定義します。
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

	// デバイス設定
	GGSERROR WINAPI GGSSetDevice(int deviceId, HWND handle)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->OpenDevice(deviceId, handle);
	}

	// リスト0番にファイルをロード
	GGSERROR WINAPI GGSLoadFile(LPCSTR fileName,int option)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromFileA(fileName, option, 0);
	}

	// リスト0番にメモリから読み込み
	GGSERROR WINAPI GGSReadMemory(const byte* smfBinary, int length, int option)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromMemory(smfBinary, length, option, 0);
	}

	// ファイルからリストに追加
	GGSERROR WINAPI GGSAddList(LPCSTR fileName, int option, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromFile3(fileName, option, musicId);
	}

	// メモリからリストへ追加
	GGSERROR WINAPI GGSReadList(const byte* smfBinary, int length, int option, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromMemory3(smfBinary, length, option, musicId);
	}

	// リスト内のアイテムを1つクリア
	void WINAPI GGSDeleteList(int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->DeleteListItem(musicId);
	}

	// リストをクリア
	void WINAPI GGSFlushList(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->ClearList();
	}

	// リスト内のIDを列挙
	const char* WINAPI GGSViewList(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->EnumListToString();
	}

	// リセットを送信
	void WINAPI GGSSendReset(int reset)
	{
	}

	// 演奏！
	GGSERROR WINAPI GGSPlay(int option, int musicId, int seek)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->Play(option, musicId, seek, 0, 0);
	}

	// 停止
	void WINAPI GGSStop(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->Stop(0);
	}

	// 一時停止
	void WINAPI GGSPause(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->PauseRestart();
	}

	// マスターテンポ
	void WINAPI GGSTempoDepth(int tempo)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		float t;
		if(tempo < 0){
			t = (float)(1 + (double)tempo / 200);
		}else{
			t = (float)(1 + (double)tempo / 50);
		}
		ggs->SetMasterTempo(t);
	}

	// マスターピッチ
	void WINAPI GGSPitchDepth(int pitch)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->SetMasterPitch(pitch);
	}

	// マスターボリューム
	void WINAPI GGSMasterVolume(int volume)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->SetMasterVolume(volume);
	}

	// SMFのデータを取得
	GGSERROR WINAPI GGSGetSMFDatas(SmfInformation3* info, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->GetSmfInformation3(info, musicId);
	}

	// 演奏状況取得
	void WINAPI GGSGetStatus(PlayerStatus3* status)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->GetPlayerStatus3(status);
	}

	// DLS読み込み
	void WINAPI GGSAddDLS(LPCSTR fileName)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->AddDlsA(fileName);
	}

	// 設定ダイアログ
	void WINAPI GGSConfigDlg(HWND handle)
	{
	}
	
	// 後始末(デバイスを閉じる)
	void WINAPI GGSFree(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->CloseDevice();
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
		Ggs3Controller::Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Ggs3Controller::FreeInstance();
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

