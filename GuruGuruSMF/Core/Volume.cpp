
#include "stdafx.h"

#include "Volume.h"

namespace GuruGuruSmf { namespace Core {

	// コンストラクタ
	Volume::Volume(IDevice* dev)
	{
		device = dev;

		InitializeCriticalSection(&lockObject);
		volumes = new int[16];
		masterVolume = 0;
	}

	// デストラクタ
	Volume::~Volume()
	{
		delete[] volumes;

		DeleteCriticalSection(&lockObject);
	}

	// 初期化
	void Volume::InitializeModule(bool normalize, int maxVolume)
	{
		if (normalize){
			normalizedGain = 127 - maxVolume;
		}else{
			normalizedGain = 0;
		}
		
		for (int i=0; i<16; i++){
			// ボリュームメッセージを送ってしまいます
			SetVolume(i, 100);
		}
		updated = false;
	}

	// trueだと音量を強制的に0にする、falseだと解除(同期)
	void Volume::Mute(bool isMute)
	{
		if (isMute){
			for (int i=0; i<16; i++){
				device->SendShortMessage(0xb0+i, 7, 0);
			}
		}else{
			for (int i=0; i<16; i++){
				SetVolume(i, volumes[i]);
			}
		}
	}

	// チャンネルごとのボリューム設定(同期)
	void Volume::SetVolume(int channel, int vol)
	{
		volumes[channel] = vol;
		vol += normalizedGain + masterVolume + fadingGain;
		
		if (vol > 127){
			vol = 127;
		}else if (vol < 0){
			vol = 0;
		}
		
		device->SendShortMessage(0xb0+channel, 7, vol);
	}

	// マスターボリュームを設定(非同期)
	void Volume::SetMasterVolume(int vol)
	{
		masterVolume = vol;
		updated = true;
	}

	// フェードインを設定(非同期) InitializeModuleの前に呼ぶ
	void Volume::SetFadeIn(int time)
	{
		if (time > 0){
			fadingTime = time;
			fadingCount = 0;
			fadingGain = -127;
			state = VolumeState::FadeIn;
		}else{
			fadingGain = 0;
			state = VolumeState::Normal;
		}
	}

	// フェードアウトを設定(非同期)
	void Volume::SetFadeOut(int time)
	{
		EnterCriticalSection(&lockObject);

		if (state == VolumeState::Normal){
			fadingCount = time;
		}else{
			fadingCount = fadingCount * time / fadingTime;
		}
		fadingTime = time;
		state = VolumeState::FadeOut;
		
		LeaveCriticalSection(&lockObject);
	}

	// フェード音量を計算
	void Volume::CalcFadingGain()
	{
		fadingGain = fadingCount * 127 / fadingTime - 127;
		if (fadingGain > 0){
			fadingGain = 0;
		}
		if (fadingGain != oldFadingGain){
			updated = true;
			oldFadingGain = fadingGain;
		}
	}

	// 非同期処理用のエントリーポイント tureの場合フェードアウト終了
	bool Volume::Worker()
	{
		bool terminated = false;
		
		EnterCriticalSection(&lockObject);

		if (state == VolumeState::FadeIn){
			fadingCount += Constant::TimeInterval;
			CalcFadingGain();
			if (fadingCount >= fadingTime){
				state = VolumeState::Normal;
			}
		}else if (state == VolumeState::FadeOut){
			fadingCount -= Constant::TimeInterval;
			CalcFadingGain();
			if (fadingCount < 0){
				state = VolumeState::Normal;
				terminated = true;
			}
		}
		
		LeaveCriticalSection(&lockObject);
		
		
		if (updated){
			for (int i=0; i<16; i++){
				SetVolume(i, volumes[i]);
			}
			updated = false;
		}
		return terminated;
	}



}}

