// DirectMusic Portモードプレイヤー

#pragma once

#include <Windows.h>
#include <map>

#include "Interface.h"
#include "Device.h"
#include "Volume.h"
#include "Times.h"
#include "Thread.h"
#include "Player.h"


namespace GuruGuruSmf { namespace Core { namespace DirectMusicPort {


	/// <summary>
	/// DirectMusic Portモードのプレイヤー
	/// </summary>
	class Player : public Core::Player::PlayerBase, public IPlayer, public IPlayerCore
	{
	private:
		Device* device;
		bool sentReset;
		
		Thread* workerThread;
		volatile bool isWorkerOn;
		
		ISmfController* smfController;

		std::map<int, CodeEvent> seekingCodeEventTable;
		bool doMasterPitch;
		
		PlayerTimes* times;
		Volume* volume;
		
		PLAYERSTATE state;
		int loopPosition, loopCount;
		bool isMusicEnd;
		
		bool isPause, doRestart;

		// 現在時間+PrepareTimeまでのイベントを処理
		bool TimeEventStep();
		
		// 冒頭の空白をスキップ
		void SkipBeginning();
		
		// シーク処理
		void Seek(int tick);
		
		// スレッド開始
		void StartWorker();

		// スレッド停止
		void StopWorker();
		
		// 一時停止
		void WorkerPause();
		
		// 一時停止解除
		void WorkerRestart();
		
		// 音楽停止時の最終処理
		void FinalizeMusic(bool isWorker, bool doReset);
		
		// マスターピッチを送信
		void SendMasterPitch();

	protected:
		// SMFで初期化
		void Initialize(Smf* newSmf);

		// 冒頭に巻き戻し
		void Rewind();

	public:
		Player();	// コンストラクタ
		~Player();	// デストラクタ

		// スレッド実行部
		void OnWorker();
		
		//
		// IPlayerを実装
		//
		
		// デバイスを開く
		GGSERROR OpenDevice(int deviceId, HWND handle);
		
		// デバイスを閉じる
		void CloseDevice();
		
		// 演奏！
		GGSERROR Play(ISmfController* smfCtrl, int option, int seek, int fadeInTime, int newKeyShift);
		
		// 停止
		void Stop(int fadeOutTime);
		
		// 一時停止
		void Pause();
		
		// 一時停止解除
		void Restart();

		// 演奏情報を取得
		void GetStatus(PlayerStatus* status);
		
		// マスターテンポ
		void SetMasterTempo(float masterTempo);
		
		// マスターボリューム
		void SetMasterVolume(int masterVolume);
		
		// マスターピッチ
		void SetMasterPitch(int pitch);
		
		
		//
		// IPlayerCoreを実装
		//
		
		// MIDIメッセージ送信デバイスを取得
		IDevice* GetDevice(){ return device; }
		
		// マーカーTickに加算処理
		void AddMarkerTick(int deltaTime);
		
		// 終点処理
		void MusicEnd();
		
		// テンポをセット
		void SetTempo(int tempo);
		
		// ボリュームメッセージを送信
		void SendVolume(int channel, int vol);
		
		// ピッチベンドメッセージを送信
		void SendPitchBend(int channel, int pitch);
		
		// エクスクルーシブを送信
		void SendExclusive(CodeEvent* codeEvent);
		
		// Seek用にイベントを登録
		void RegisterEvent(CODE_EVENTTYPE type, int channel, int data1, CodeEvent* codeEvent);

		// シフトされたノート番号を取得
		int GetShiftedNote(int note, int channel);


	};

}}}

