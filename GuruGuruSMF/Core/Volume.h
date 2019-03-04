// 音量関係

#pragma once

#include <Windows.h>
#include "Constants.h"
#include "Interface.h"

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// 音量コントローラ
	/// </summary>
	class Volume
	{
	private:
		IDevice* device;
		VOLUMESTATE state;
		int* volumes;
		int masterVolume, normalizedGain;
		bool updated;
		int fadingCount, fadingTime, fadingGain, oldFadingGain;
		CRITICAL_SECTION lockObject;

	public:
		Volume(IDevice* dev);	// コンストラクタ
		~Volume();	// デストラクタ

		// 状態
		int GetState(){ return state; }
		
		// trueだと音量を強制的に0にする、falseだと解除(同期)
		void Mute(bool isMute);
		
		// チャンネルごとのボリューム設定(同期)
		void SetVolume(int channel, int vol);
		
		// マスターボリュームを設定(非同期)
		void SetMasterVolume(int vol);
		
		// フェードインを設定(非同期) InitializeModuleの前に呼ぶ
		void SetFadeIn(int time);
		
		// フェードアウトを設定(非同期)
		void SetFadeOut(int time);

		// フェード音量を計算
		void CalcFadingGain();
		
		// 非同期処理用のエントリーポイント tureの場合フェードアウト終了
		bool Worker();
		
		// 初期化
		void InitializeModule(bool normalize, int maxVolume);

	};


}}


