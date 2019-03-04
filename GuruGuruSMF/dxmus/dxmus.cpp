// dxmus.cpp : DLL アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Controller.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

using namespace GuruGuruSmf::Dxmus;

// パフォーマンスプレイヤーを使用可能にする
extern "C" int WINAPI PerformanceOpen(HWND handle, int channels)
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformanceOpen(handle, channels);
}

// パフォーマンスプレイヤーを閉じる
extern "C" void WINAPI PerformanceClose()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceClose();
}

// パフォーマンスプレイヤーで演奏
extern "C" int WINAPI PerformancePlay(PerformancePlayer::SmfData* smf, int option, int seek)
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformancePlay(smf, option, seek);
}

// パフォーマンスプレイヤーの演奏を停止
extern "C" void WINAPI PerformanceStop()
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformanceStop();
}

// パフォーマンスプレイヤー一時停止
extern "C" void WINAPI PerformancePause()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformancePause();
}

// パフォーマンスプレイヤー一時停止解除
extern "C" void WINAPI PerformanceRestart()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceRestart();
}

// パフォーマンスプレイヤー・マスターボリューム
extern "C" void WINAPI PerformanceSetMasterVolume(long volume)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceSetMasterVolume(volume);
}

// パフォーマンスプレイヤー・マスターテンポ
extern "C" void WINAPI PerformanceSetMasterTempo(float tempo)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceSetMasterTempo(tempo);
}

// パフォーマンスプレイヤーの状態取得
extern "C" void WINAPI PerformanceGetStatus(GuruGuruSmf::PlayerStatus* status)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceGetStatus(status);
}

// ポートモードを使用可能にする
extern "C" int WINAPI PortOpen(HWND handle)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortOpen(handle);
}

// ポートモードを閉じる
extern "C" void WINAPI PortClose()
{
	Controller* cc = Controller::GetInstance();
	cc->PortClose();
}

// リファレンスタイムを取得
extern "C" int WINAPI PortGetReferenceTime(long long* time)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortGetReferenceTime(time);
}

// 短いMIDIメッセージを送る
extern "C" int WINAPI PortSendShortMessage(long long time, int midiMessage)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortSendShortMessage(time, midiMessage);
}

// 長いMIDIメッセージを送る
extern "C" int WINAPI PortSendLongMessage(long long time, LPBYTE midiMessage, int length)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortSendLongMessage(time, midiMessage, length);
}

// 使用する音色をリストで指定
extern "C" void WINAPI PortSetVoiceList(DWORD* list, int count)
{
	Controller* cc = Controller::GetInstance();
	cc->PortSetVoiceList(list, count);
}

// DLSを追加
extern "C" int WINAPI PortAddDls(wchar_t* fileName)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortAddDls(fileName);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		Controller::FreeInstance();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

