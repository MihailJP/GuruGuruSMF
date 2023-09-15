
#include "stdafx.h"

#include <dxmus.core/Controller.h>
#include "DmPortPlayer.h"



namespace GuruGuruSmf { namespace Core { namespace DirectMusicPort {

	// スレッド呼び出し用
	static DWORD WINAPI PlayerWorker(LPVOID object)
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		//CoInitialize(NULL);
		((Player*)object)->OnWorker();
		CoUninitialize();
		return 0;
	}

	// コンストラクタ
	Player::Player()
	{
		times = new PlayerTimes();
		device = new Device(times);
		volume = new Volume(device);

		workerThread = new Thread();

		sentReset = false;
		isWorkerOn = false;
		doMasterPitch = false;
		loopCount = 0;
		state = PlayerState::Stop;
	}

	// デストラクタ
	Player::~Player()
	{
		TRACE(L"●DmPort.Player:デストラクタ:CloseDevice\n");
		CloseDevice();

		TRACE(L": volume\n");
		delete volume;
		TRACE(L": device\n");
		delete device;
		TRACE(L": times\n");
		delete times;

		TRACE(L": thread\n");
		delete workerThread;
	}

	// デバイスを開く
	GGSERROR Player::OpenDevice(int deviceId, HWND handle)
	{
		CloseDevice();

		GGS_LOGGING_INITIALIZE();

		return device->Open(deviceId, handle);
	}

	// デバイスを閉じる
	void Player::CloseDevice()
	{
		if (!device->GetIsOpen()) return;
		
		TRACE(L"DmPort.Player: CloseDevice : Stop\n");
		Stop(0);
		TRACE(L"DmPort.Player: CloseDevice : Sleep\n");
		Sleep(150);	// FinalizeMusicでリセットを送信しているため
		TRACE(L"DmPort.Player: CloseDevice : device->Close()\n");
		device->Close();
	}

	// 演奏！
	GGSERROR Player::Play(ISmfController* smfCtrl, int option, int seek, int fadeInTime, int newKeyShift)
	{
		if (!device->GetIsOpen()) return GgsError::NotReady;
		Stop(0);

		Smf* newSmf = smfCtrl->GetSmf();

		if (!newSmf->GetIsReady()) return GgsError::NotReady;
		if (newSmf->GetData()->FirstNoteTick == -1) return GgsError::NoError;	// 無音データはスルー

		GGSERROR err = smfCtrl->Open();
		if(err != GgsError::NoError) return err;
		smfController = smfCtrl;

		Initialize(newSmf);

		VoiceList* voiceList = newSmf->GetData()->VoiceList;
		GuruGuruSmf::Dxmus::Controller::GetInstance()->PortSetVoiceList((DWORD*)voiceList->GetItems(), voiceList->GetCount());
		ParsePlayOption(option);
		SetKeyShift(newKeyShift);
		
		// 直前に何か送信していたら1ミリ秒のウェイトをかける
		err = times->ReadMasterClock();
		if(err != GgsError::NoError) return err;
		times->SetMarkerDmTimeByQueueDmTime(10000);
	
		// Stopなどでリセットを送ってなかった場合、GSリセットを送っておく
		if (!sentReset){
			err = device->SendLongMessage(Exclusive::GSReset, Exclusive::GSResetLength);
			if(err != GgsError::NoError) return err;
			times->Wait(10000);		// 不要なようですが一応1ミリ秒ウェイト
		}
		sentReset = false;
		
		// RPN0番にはリセットが効かないようなので送信しておく
		InitializePitchBendSensitivity(device);
		
		volume->SetFadeIn(fadeInTime);
		volume->InitializeModule(GetDoNormalize(), newSmf->GetData()->MaxVolume);
		times->Wait(10000);		// 処理順が狂うことがあるようなのでウェイト
		
		if (seek > 0){
			Seek(seek);
		}else{
			SkipBeginning();
		}

		times->UpdateQueueDmTime();
	
		if (TimeEventStep()) return GgsError::NoError;
		state = PlayerState::Play;
		StartWorker();
		
		return GgsError::NoError;
	}

	// 停止
	void Player::Stop(int fadeOutTime)
	{
		if (!device->GetIsOpen() || state == PlayerState::Stop) return;
		
		// 一時停止状態の時はただちに停止
		if (state != PlayerState::Pause && fadeOutTime > 0){	// フェードアウト処理
			volume->SetFadeOut(fadeOutTime);
			return;
		}
		
		StopWorker();
		FinalizeMusic(false, true);
	}

	// 一時停止
	void Player::Pause()
	{
		if (state == PlayerState::Play){
			isPause = true;
			doRestart = false;
			Sleep(Constant::TimeInterval);	// 保険
		}
	}

	// 一時停止解除
	void Player::Restart()
	{
		if (state == PlayerState::Pause){
			doRestart = true;
			Sleep(Constant::TimeInterval);	// 保険
		}
	}

	// 演奏情報を取得
	void Player::GetStatus(PlayerStatus* status)
	{
		status->State = state;
		status->LoopCount = loopCount;
		status->Tick = times->GetNowTick();
		status->Time = (int)times->GetNowTime();
		status->Tempo = times->GetMusicTempo();
	}

	// マスターテンポ
	void Player::SetMasterTempo(float masterTempo)
	{
		times->SetMasterTempo(masterTempo);
	}

	// マスターボリューム
	void Player::SetMasterVolume(int masterVolume)
	{
		volume->SetMasterVolume(masterVolume);
	}

	// マスターピッチ
	void Player::SetMasterPitch(int pitch)
	{
		SetMasterPitchBase(pitch);
		doMasterPitch = true;
	}
		
	// マーカーTickに加算処理
	void Player::AddMarkerTick(int deltaTime)
	{
		times->UpdateMarkerTick(deltaTime);
		
		// ループ位置を設定
		if (loopPosition == -1 && times->PassedLoopTick()){
			loopPosition = GetCode()->GetPosition();
		}
	}
		
	// 終点処理
	void Player::MusicEnd()
	{
		isMusicEnd = true;
	}

	// テンポをセット
	void Player::SetTempo(int tempo)
	{
		times->SetTempo(tempo);
	}

	// ボリュームメッセージを送信
	void Player::SendVolume(int channel, int vol)
	{
		volume->SetVolume(channel, vol);
	}
		
	// ピッチベンドメッセージを送信
	void Player::SendPitchBend(int channel, int pitch)
	{
		SendPitchBendBase(device, channel, pitch);
	}

	// エクスクルーシブを送信
	void Player::SendExclusive(CodeEvent* codeEvent)
	{
		SendExclusiveBase(device, codeEvent);
	}
		
	// Seek用にイベントを登録
	void Player::RegisterEvent(CODE_EVENTTYPE type, int channel, int data1, CodeEvent* codeEvent)
	{
		int key = (byte)type << 16 | channel << 8 | data1;
		seekingCodeEventTable[key] = *codeEvent;
	}

	// シフトされたノート番号を取得
	int Player::GetShiftedNote(int note, int channel)
	{
		return GetShiftedNoteBase(note, channel);
	}
		
	// 現在時間+PrepareTimeまでのイベントを処理
	bool Player::TimeEventStep()
	{
		while(!times->PassedPrepareTime()){
			CodeEvent ev;
			GetCode()->ReadEvent(&ev);
			PlayingEventExecute(&ev, this);
			
			// 末尾の空白飛ばし
			if(GetDoSkipEnding()){
				if(times->PassedLastNote()){
					times->RewindQueueTime();	// 処理済み時間まで戻しておく
					isMusicEnd = true;
				}
			}
			
			// ループか終了か
			if(isMusicEnd){
				++loopCount;
				if(GetDoLoop()){
					switch(GetSmf()->GetData()->LoopType){
					case LoopType::Beginning:
						Rewind();
						SkipBeginning();
						break;
					case LoopType::LoopPointAtBeginning:
						Rewind();
						break;
					case LoopType::LoopPoint:
						GetCode()->SetPosition(loopPosition);
						times->GotoLoopPoint();
						isMusicEnd = false;
						break;
					case LoopType::End:
						return true;
					}
				}else{
					return true;	// 自然終了
				}
			}
		}
		
		// 現在のTickと演奏時間を計算
		times->CalcNowTime();
		return false;
	}
		
	// 冒頭の空白をスキップ
	void Player::SkipBeginning()
	{
		if(GetDoSkipBeginning()){
			times->SetSkipping(true);
			
			while(!times->PassedFirstNote()){
				CodeEvent ev;
				GetCode()->ReadEvent(&ev);
				PlayingEventExecute(&ev, this);
			}
			times->SetSkipping(false);
		}
	}
		
	// シーク処理
	void Player::Seek(int tick)
	{
		seekingCodeEventTable.clear();
		times->SetSkipping(true);
		while(!times->PassedTick(tick)){
			CodeEvent ev;
			GetCode()->ReadEvent(&ev);
			SeekingEventExecute(&ev, this);
			if (isMusicEnd) break;
		}
		
		// 保存しておいたデータを送信
		std::map<int, CodeEvent>::iterator it = seekingCodeEventTable.begin();
		while(it != seekingCodeEventTable.end()){
			PlayingEventExecute(&(*it).second, this);
			++it;
		}
		seekingCodeEventTable.clear();

		times->SetSkipping(false);
	}
	
	// スレッド開始
	void Player::StartWorker()
	{
		workerThread->Join();	// スレッドが完全に終了してないケースがあるので待つ

		isWorkerOn = true;
		workerThread->Start(this, PlayerWorker, THREAD_PRIORITY_TIME_CRITICAL);
	}

	// スレッド停止
	void Player::StopWorker()
	{
		if (isWorkerOn){
			isWorkerOn = false;
			workerThread->Join();
		}
	}
		
	// 一時停止
	void Player::WorkerPause()
	{
		if (state == PlayerState::Play){
			// 音量を0にする
			times->EnterRealtimeMode();
			volume->Mute(true);
			times->LeaveRealtimeMode();
			times->Pause();
			
			state = PlayerState::Pause;
		}
	}
		
	// 一時停止解除
	void Player::WorkerRestart()
	{
		times->Restart();
		
		times->EnterRealtimeMode();
		volume->Mute(false);
		times->LeaveRealtimeMode();
		
		state = PlayerState::Play;
		isPause = false;
		doRestart = false;
	}
		
	// スレッド実行部
	void Player::OnWorker()
	{
		while(isWorkerOn){
			times->ReadMasterClock();
			
			if (doRestart) WorkerRestart();
			
			if (isPause){
				WorkerPause();
			}else{
				times->EnterRealtimeMode();
				bool terminated = volume->Worker();
				times->LeaveRealtimeMode();
				if (terminated){
					// オールノートオフ
					for (int i=0; i<16; i++){
						device->SendShortMessage(0xb0+i, 123, 0);
					}
					times->UpdateQueueDmTime();
					FinalizeMusic(true, true);
					return;
				}
				
				if (GetCanDoMasterPitch() && doMasterPitch){
					SendMasterPitch();
					doMasterPitch = false;
				}
				
				if (TimeEventStep()){
					FinalizeMusic(true, false);
					return;
				}
			}
			
			Sleep(Constant::TimeInterval);
			times->FixQueueDmTime();
		}
	}
	
	// 音楽停止時の最終処理
	void Player::FinalizeMusic(bool isWorker, bool doReset)
	{
		if (isWorker){
			isWorkerOn = false;
		}
		state = PlayerState::Stop;
		smfController->Close();
	
		if (doReset){
			// GSリセットを送る
			times->ReadMasterClock();
			times->SetMarkerDmTimeByQueueDmTime(10000);
			device->SendLongMessage(Exclusive::GSReset, Exclusive::GSResetLength);
			sentReset = true;
			times->UpdateQueueDmTime();
		}
	}
	
	// マスターピッチを送信
	void Player::SendMasterPitch()
	{
		times->EnterRealtimeMode();
		SendMasterPitchBase(device);
		times->LeaveRealtimeMode();
	}
		
	// SMFで初期化
	void Player::Initialize(Smf* newSmf)
	{
		PlayerBase::Initialize(newSmf);
		
		times->Initialize(newSmf);
		loopPosition = -1;
	
		loopCount = 0;
		isPause = false;
		doRestart = false;
	
		Rewind();
	}

	// 冒頭に巻き戻し
	void Player::Rewind()
	{
		PlayerBase::Rewind();
		isMusicEnd = false;

		times->Rewind();
		GetCode()->SetPosition(0);
	}



}}}

