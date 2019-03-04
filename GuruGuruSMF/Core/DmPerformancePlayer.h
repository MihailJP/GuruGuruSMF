// DirectMusic Performanceモード(Legacy)

#pragma once

#include <Windows.h>
#include "Constants.h"
#include "Interface.h"
#include "Smf.h"
#include <dxmus.core/Controller.h>

namespace GuruGuruSmf { namespace Core { namespace DirectMusicPerformance {

	/// <summary>
	/// DirectMusic Performanceモードのプレイヤー
	/// </summary>
	class Player : public IPlayer
	{
	private:
		bool isOpen;

	public:
		Player();	// コンストラクタ
		~Player();	// デストラクタ

		//
		// ISmfPlayerを実装
		//
		
		// デバイスを開く
		GGSERROR OpenDevice(int deviceId, HWND handle);
		
		// デバイスを閉じる
		void CloseDevice();
		
		// 演奏！
		GGSERROR Play(ISmfController* smfCtrl, int option, int seek, int fadeInTime, int keyShift);
		
		// 停止！
		void Stop(int fadeOutTime);
		
		// 一時停止
		void Pause(){ GuruGuruSmf::Dxmus::Controller::GetInstance()->PerformancePause(); }
		
		// 一時停止解除
		void Restart(){ GuruGuruSmf::Dxmus::Controller::GetInstance()->PerformanceRestart(); }
		
		// 演奏状態を取得
		void GetStatus(PlayerStatus* status){ GuruGuruSmf::Dxmus::Controller::GetInstance()->PerformanceGetStatus(status); }
		
		// マスターテンポ
		void SetMasterTempo(float tempo){ GuruGuruSmf::Dxmus::Controller::GetInstance()->PerformanceSetMasterTempo(tempo); }
		
		// マスターボリューム
		void SetMasterVolume(int volume){ GuruGuruSmf::Dxmus::Controller::GetInstance()->PerformanceSetMasterVolume(volume); }
		
		// マスターピッチ
		void SetMasterPitch(int pitch){}
		
	};




}}}
