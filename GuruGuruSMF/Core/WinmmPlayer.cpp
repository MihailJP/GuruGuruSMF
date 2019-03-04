
#include "stdafx.h"

#include "WinmmPlayer.h"

namespace GuruGuruSmf { namespace Core { namespace WindowsMultimedia {

	// タイマーコールバック
	static void CALLBACK TimeCallback(UINT timerId, UINT msg, DWORD user, DWORD param1, DWORD param2)
	{
		((Player*)user)->OnTimer();
	}

	// コンストラクタ
	Player::Player()
	{
		device = new Device();
		times = new PlayerTimes();
		volume = new Volume(device);

		timerId = 0;
		doMasterPitch = false;

		state = PlayerState::Stop;
		loopCount = 0;
		sentReset = false;
		resetedTime = 0;
	}
	
	// デストラクタ
	Player::~Player()
	{
		CloseDevice();

		delete volume;
		delete times;
		delete device;
	}

	// タイマー実行部
	void Player::OnTimer()
	{
		if(doRestart){
			WorkerRestart();
		}
		
		if(state != PlayerState::Play) return;
		
		if(isPause){
			WorkerPause();
			return;
		}
		
		if(volume->Worker()){
			StopTimer();
			// オールノートオフ＋リセット
			for (int i=0; i<16; i++){
				device->SendShortMessage(0xb0+i, 123, 0);
			}
			SendReset();
			state = PlayerState::Stop;
			smfController->Close();
			return;
		}
		
		if(GetCanDoMasterPitch() && doMasterPitch){
			SendMasterPitchBase(device);
			doMasterPitch = false;
		}
		
		while(times->PassedMarkerTick()){
			CodeEvent ev;
			GetCode()->ReadEvent(&ev);
			PlayingEventExecute(&ev, this);
			
			// 末尾の空白飛ばし
			if(GetDoSkipEnding()){
				if(times->PassedLastNote()){
					isMusicEnd = true;
				}
			}
			
			// ループか終了か
			if(isMusicEnd){
				++loopCount;
				if (GetDoLoop()){
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
						times->GotoLoopPoint(GetDoSkipEnding());
						isMusicEnd = false;
						break;
					case LoopType::End:
						state = PlayerState::Stop;
						StopTimer();
						smfController->Close();
						return;
					}
				}else{
					state = PlayerState::Stop;
					StopTimer();
					smfController->Close();
					return;	// 自然終了
				}
			}
		}
		
		times->UpdateNowTick();
	}

	// デバイスを開く
	GGSERROR Player::OpenDevice(int deviceId, HWND handle)
	{
		Stop(0);
		GGS_LOGGING_INITIALIZE();
		return device->Open(deviceId);
	}
	
	// デバイスを閉じる
	void Player::CloseDevice()
	{
		Stop(0);
		device->Close();
	}
	
	// 演奏！
	GGSERROR Player::Play(ISmfController* smfCtrl, int option, int seek, int fadeInTime, int keyShift)
	{
		if (!device->GetIsOpen()) return GgsError::NotReady;
		Stop(0);
		
		Smf* newSmf = smfCtrl->GetSmf();

		if(!newSmf->GetIsReady()) return GgsError::NotReady;
		if(newSmf->GetData()->FirstNoteTick == -1) return GgsError::NoError;	// 無音データはスルー

		GGSERROR err = smfCtrl->Open();
		if(err != GgsError::NoError) return err;
		smfController = smfCtrl;

		Initialize(newSmf);
		ParsePlayOption(option);
		SetKeyShift(keyShift);
		
		SendReset();
		WaitAfterReset();
		sentReset = false;
		
		InitializePitchBendSensitivity(device);
		volume->SetFadeIn(fadeInTime);
		volume->InitializeModule(GetDoNormalize(), newSmf->GetData()->MaxVolume);
		
		// シーク処理
		if (seek > 0){
			Seek(seek);
		}else{
			SkipBeginning();
		}
		
		err = StartTimer();
		if (err != GgsError::NoError) return err;
		state = PlayerState::Play;			
		return GgsError::NoError;
	}
	
	// 停止！
	void Player::Stop(int fadeOutTime)
	{
		if (state == PlayerState::Stop) return;

		// 一時停止状態の時はただちに停止
		if (state != PlayerState::Pause && fadeOutTime > 0){	// フェードアウト処理
			volume->SetFadeOut(fadeOutTime);
			return;
		}
		
		StopTimer();
		Sleep(Constant::TimeInterval);	// 保険

		// オールノートオフ＋リセット
		for (int i=0; i<16; i++){
			device->SendShortMessage(0xb0+i, 123, 0);
		}
		SendReset();
		state = PlayerState::Stop;
		
		smfController->Close();
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
	
	// 演奏状態を取得
	void Player::GetStatus(PlayerStatus* status)
	{
		status->State = state;
		status->LoopCount = loopCount;
		status->Tick = times->GetNowTick();
		status->Time = times->GetNowTime();
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
	void Player::SetTempo(int newTempo)
	{
		times->SetTempo(newTempo);
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
	
	// タイマー開始
	GGSERROR Player::StartTimer()
	{
		StopTimer();
		
		timeBeginPeriod(1);
		timerId = timeSetEvent(Constant::TimeInterval, 1, TimeCallback, (DWORD_PTR)this, 1);
		if (timerId == 0){
			timeEndPeriod(1);
			return GgsError::CannotStartTimer;
		}
		
		return GgsError::NoError;
	}
		
	// タイマー停止
	void Player::StopTimer()
	{
		if (timerId != 0){
			timeKillEvent(timerId);
			timeEndPeriod(1);
			timerId = 0;
		}
	}
		
	// 一時停止
	void Player::WorkerPause()
	{
		if (state == PlayerState::Play){
			// 音量を0にする
			volume->Mute(true);
			
			state = PlayerState::Pause;
		}
		isPause = false;
	}
		
	// 一時停止解除
	void Player::WorkerRestart()
	{
		volume->Mute(false);
		
		state = PlayerState::Play;
		isPause = false;
		doRestart = false;
	}
		
	// 冒頭の空白をスキップ
	void Player::SkipBeginning()
	{
		if (GetDoSkipBeginning()){
			while (!times->PassedFirstNote()){
				CodeEvent ev;
				GetCode()->ReadEvent(&ev);
				PlayingEventExecute(&ev, this);
			}
			
			times->GotoFirstNote();
		}
	}
	
	// シーク処理
	void Player::Seek(int tick)
	{
		if (tick > GetSmf()->GetData()->LastNoteTick){
			tick = GetSmf()->GetData()->LastNoteTick;
		}
		
		seekingCodeEventTable.clear();
		while (!times->PassedTick(tick)){
			CodeEvent ev;
			GetCode()->ReadEvent(&ev);
			SeekingEventExecute(&ev, this);
			if (isMusicEnd) break;
		}
		
		times->FixNowTime(tick);
		
		// 保存しておいたデータを送信
		std::map<int, CodeEvent>::iterator it = seekingCodeEventTable.begin();
		while(it != seekingCodeEventTable.end()){
			PlayingEventExecute(&(*it).second, this);
			++it;
		}
		seekingCodeEventTable.clear();
	}
	
	// リセット命令を送る
	void Player::SendReset()
	{
		if (!sentReset){
			// オールサウンドオフ＋リセットオールコントローラ
			for (int i=0; i<16; i++){
				device->SendShortMessage(0xb0+i, 120, 0);
				device->SendShortMessage(0xb0+i, 121, 0);
			}
			device->SendLongMessage(Exclusive::GMSystemOn, Exclusive::GMSystemOnLength);
			
			LARGE_INTEGER t;
			if(QueryPerformanceCounter(&t)){
				resetedTime = t.QuadPart;
			}else{
				resetedTime = timeGetTime();
			}
		}
		
		sentReset = true;
	}

	// リセット後のウェイトを入れる
	void Player::WaitAfterReset()
	{
		double interval;

		LARGE_INTEGER t;
		if(QueryPerformanceCounter(&t)){
			interval = (double)(t.QuadPart - resetedTime);

			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);
			interval = interval * 1000 / freq.QuadPart;
		}else{
			interval = (double)(resetedTime - timeGetTime());
		}

		if (interval < 50){
			int wait = (int)(50 - interval) + 1;
			Sleep(wait);
		}
	}


}}}




