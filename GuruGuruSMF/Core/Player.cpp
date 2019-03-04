
#include "stdafx.h"

#include "Player.h"


namespace GuruGuruSmf { namespace Core { namespace Player {

	//
	// MusicEndEventExecutor
	//

	// 終点用
	void MusicEndEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->MusicEnd();
	}

	
	//
	// DeltaTimeEventExecutor
	//

	// デルタタイム用
	void DeltaTimeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->AddMarkerTick(codeEvent->IntData);
	}
	
	
	//
	// TempoEventExecutor
	//

	// テンポ用
	void TempoEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->SetTempo(codeEvent->IntData);
	}
	
	
	//
	// PlayingNoteOnEventExecutor
	//

	// Play: ノートオン用
	void PlayingNoteOnEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		int note = core->GetShiftedNote(codeEvent->ByteData1, codeEvent->Channel);
		core->GetDevice()->SendShortMessage(0x90+codeEvent->Channel,
		                             note, codeEvent->ByteData2);
	}
	
	
	//
	// PlayingNoteOffEventExecutor
	//

	// Play: ノートオフ用
	void PlayingNoteOffEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		int note = core->GetShiftedNote(codeEvent->ByteData1, codeEvent->Channel);
		core->GetDevice()->SendShortMessage(0x80+codeEvent->Channel,
		                             note, codeEvent->ByteData2);
	}
	

	//
	// PlayingProgramChangeEventExecutor
	//

	// Play: プログラムチェンジ用
	void PlayingProgramChangeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->GetDevice()->SendShortMessage(0xc0+codeEvent->Channel,
		                             codeEvent->ByteData1, 0);
	}
	
	
	//
	// PlayingControlChangeEventExecutor
	//

	// Play: コントロールチェンジ用
	void PlayingControlChangeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->GetDevice()->SendShortMessage(0xb0+codeEvent->Channel,
		                             codeEvent->ByteData1, codeEvent->ByteData2);
	}

	
	//
	// PlayingVolumeEventExecutor
	//

	/// Play: ボリューム用
	void PlayingVolumeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->SendVolume(codeEvent->Channel, codeEvent->ByteData2);
	}
	
	
	//
	// PlayingPitchBendEventExecutor
	//

	// Play: ピッチベンド用
	void PlayingPitchBendEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->SendPitchBend(codeEvent->Channel, codeEvent->IntData);
	}
	
	
	//
	// PlayingCAfEventExecutor
	//

	// Play: チャンネルプレッシャー用
	void PlayingCAfEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->GetDevice()->SendShortMessage(0xd0+codeEvent->Channel,
		                             codeEvent->ByteData1, 0);
	}
	
	
	//
	// PlayingPAfEventExecutor
	//

	// Play: ポリフォニックキープレッシャー用
	void PlayingPAfEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		int note = core->GetShiftedNote(codeEvent->ByteData1, codeEvent->Channel);
		core->GetDevice()->SendShortMessage(0xa0+codeEvent->Channel,
		                             note, codeEvent->ByteData2);
	}
	
	
	//
	// PlayingExclusiveEventExecutor
	//

	// Play: エクスクルーシブ用
	void PlayingExclusiveEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->SendExclusive(codeEvent);
	}
	
	
	
	//
	// SeekingProgramChangeEventExecutor
	//

	// Seek: プログラムチェンジ用
	void SeekingProgramChangeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel, 0, codeEvent);
	}
	
	
	//
	// SeekingControlChangeEventExecutor
	//

	// Seek: コントロールチェンジ用
	void SeekingControlChangeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel,
		                   codeEvent->ByteData1, codeEvent);
	}
	
	
	//
	// SeekingVolumeEventExecutor
	//

	// Seek: ボリューム用
	void SeekingVolumeEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel, 0, codeEvent);
	}
	
	
	//
	// SeekingPitchBendEventExecutor
	//

	// Seek: ピッチベンド用
	void SeekingPitchBendEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel, 0, codeEvent);
	}
	
	
	//
	// SeekingCAfEventExecutor
	//

	// Seek: チャンネルプレッシャー用
	void SeekingCAfEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel, 0, codeEvent);
	}
	
	
	//
	// SeekingPAfEventExecutor
	//

	// Seek: ポリフォニックキープレッシャー用
	void SeekingPAfEventExecutor::Execute(CodeEvent* codeEvent, IPlayerCore* core)
	{
		core->RegisterEvent((CODE_EVENTTYPE)codeEvent->Type, codeEvent->Channel,
		                   codeEvent->ByteData1, codeEvent);
	}


	//
	// PlayerBase
	//

	// コンストラクタ
	PlayerBase::PlayerBase()
	{
		rhythmChannels = new bool[16];
		pitches = new int[16];
		masterPitch = 0;

		MakePlayCodeEventExcutors();
		MakeSeekCodeEventExcutors();
	}

	// デストラクタ
	PlayerBase::~PlayerBase()
	{
		TRACE(L"●PlayerBase:デストラクタ:pitchs\n");
		delete[] pitches;
		TRACE(L":rhythmChannels\n");
		delete[] rhythmChannels;

		TRACE(L":EventExecutors\n");
		for (int i=0; i<CodeEventType::MusicEnd+1; i++){
			delete playingEventExecutors[i];
			delete seekingEventExecutors[i];
		}
		TRACE(L":playingEventExecutors\n");
		delete[] playingEventExecutors;
		TRACE(L":seekingEventExecutors\n");
		delete[] seekingEventExecutors;
	}

	// シフトされたノート番号を取得
	int PlayerBase::GetShiftedNoteBase(int note, int channel)
	{
		if (rhythmChannels[channel]) return note;
		
		note += keyShift;
		if (note < 0){
			note = 0;
		}else if (note > 127){
			note = 127;
		}
		return note;
	}

	// マスターピッチを送信
	void PlayerBase::SendMasterPitchBase(IDevice* device)
	{
		for (int i=0; i<16; i++){
			SendPitchBendBase(device, i, pitches[i]);
		}
	}

	// ピッチベンドメッセージを送信
	void PlayerBase::SendPitchBendBase(IDevice* device, int channel, int pitch)
	{
		int p = pitch + 8192 + masterPitch;
		pitches[channel] = pitch;
		if (p < 0){
			p = 0;
		}else if (p > 16383){
			p = 16383;
		}
		
		device->SendShortMessage(0xe0+channel, p & 0x7f, (p >> 7) & 0x7f);
	}

	// エクスクルーシブを送信
	void PlayerBase::SendExclusiveBase(IDevice* device, CodeEvent* codeEvent)
	{
		TRACE1(L"PlayerBase::SendExclusiveBase(%d)\n", doSendExclusive);
		if (doSendExclusive){
			if (codeEvent->Type == CodeEventType::UseRhythmPart){
				rhythmChannels[codeEvent->Channel] = (codeEvent->Option1 != 0);
			}
			
			ExclusiveData* ex = code->GetExclusive(codeEvent->IntData);
			device->SendLongMessage(ex->GetData(), ex->GetLength());
		}
	}

	// ピッチベンドセンシビリティを初期化
	void PlayerBase::InitializePitchBendSensitivity(IDevice* device)
	{
		int n;
		if (canDoMasterPitch){
			n = 12;
		}else{
			n = 2;
		}
		
		for (int i=0; i<16; i++){
			device->SendShortMessage(0xb0+i, 101, 0);
			device->SendShortMessage(0xb0+i, 100, 0);
			device->SendShortMessage(0xb0+i, 6, n);
		}
	}

	// SMFで初期化
	void PlayerBase::Initialize(Smf* newSmf)
	{
		smf = newSmf;
		code = newSmf->GetCode();
		
		for (int i=0; i<16; i++){
			pitches[i] = 0;
		}
	}

	// 冒頭へ巻き戻し
	void PlayerBase::Rewind()
	{
		for (int i=0; i<16; i++){
			rhythmChannels[i] = (i == 9);
		}
	}
	
	// PlayOptionを処理
	void PlayerBase::ParsePlayOption(int option)
	{
		doLoop = ((option & PlayOption::Loop) != 0);
		doSkipBeginning = ((option & PlayOption::SkipBeginning) != 0);
		doSkipEnding = ((option & PlayOption::SkipEnding) != 0);
		canDoMasterPitch = ((option & PlayOption::MasterPitch) != 0);
		doNormalize = ((option & PlayOption::Normalize) != 0);
		doSendExclusive = ((option & PlayOption::Exclusive) != 0);
	}

	// イベントを実行(Play)
	void PlayerBase::PlayingEventExecute(CodeEvent* ev, IPlayerCore* core)
	{
		playingEventExecutors[ev->Type]->Execute(ev, core);
	}

	// イベントを実行(Seek)
	void PlayerBase::SeekingEventExecute(CodeEvent* ev, IPlayerCore* core)
	{
		seekingEventExecutors[ev->Type]->Execute(ev, core);
	}

	// イベント実行オブジェクトの配列を作成(Play)
	void PlayerBase::MakePlayCodeEventExcutors()
	{
		playingEventExecutors = new CodeEventExecutor*[CodeEventType::MusicEnd+1];
		playingEventExecutors[CodeEventType::DeltaTime] = new DeltaTimeEventExecutor();
		playingEventExecutors[CodeEventType::NoteOn] = new PlayingNoteOnEventExecutor();
		playingEventExecutors[CodeEventType::NoteOff] = new PlayingNoteOffEventExecutor();
		playingEventExecutors[CodeEventType::ProgramChange] = new PlayingProgramChangeEventExecutor();
		
		playingEventExecutors[CodeEventType::ControlChangeA] = new PlayingControlChangeEventExecutor();
		playingEventExecutors[CodeEventType::ControlChangeB] = new PlayingControlChangeEventExecutor();
		playingEventExecutors[CodeEventType::ControlChangeC] = new NullEventExecutor();
		playingEventExecutors[CodeEventType::Volume] = new PlayingVolumeEventExecutor();
		
		playingEventExecutors[CodeEventType::PAf] = new PlayingPAfEventExecutor();
		playingEventExecutors[CodeEventType::CAf] = new PlayingCAfEventExecutor();
		playingEventExecutors[CodeEventType::PitchBend] = new PlayingPitchBendEventExecutor();
		
		playingEventExecutors[CodeEventType::Tempo] = new TempoEventExecutor();
		playingEventExecutors[CodeEventType::Exclusive] = new PlayingExclusiveEventExecutor();
		playingEventExecutors[CodeEventType::UseRhythmPart] = new PlayingExclusiveEventExecutor();
		
		playingEventExecutors[CodeEventType::MusicEnd] = new MusicEndEventExecutor();
	}
	
	// イベント実行オブジェクトの配列を作成(Seek)
	void PlayerBase::MakeSeekCodeEventExcutors()
	{
		seekingEventExecutors = new CodeEventExecutor*[CodeEventType::MusicEnd+1];
		seekingEventExecutors[CodeEventType::DeltaTime] = new DeltaTimeEventExecutor();
		seekingEventExecutors[CodeEventType::NoteOn] = new NullEventExecutor();
		seekingEventExecutors[CodeEventType::NoteOff] = new NullEventExecutor();
		seekingEventExecutors[CodeEventType::ProgramChange] = new SeekingProgramChangeEventExecutor();
		
		seekingEventExecutors[CodeEventType::ControlChangeA] = new SeekingControlChangeEventExecutor();
		seekingEventExecutors[CodeEventType::ControlChangeB] = new PlayingControlChangeEventExecutor();
		seekingEventExecutors[CodeEventType::ControlChangeC] = new NullEventExecutor();
		seekingEventExecutors[CodeEventType::Volume] = new SeekingVolumeEventExecutor();
		
		seekingEventExecutors[CodeEventType::PAf] = new SeekingPAfEventExecutor();
		seekingEventExecutors[CodeEventType::CAf] = new SeekingCAfEventExecutor();
		seekingEventExecutors[CodeEventType::PitchBend] = new SeekingPitchBendEventExecutor();
		
		seekingEventExecutors[CodeEventType::Tempo] = new TempoEventExecutor();
		seekingEventExecutors[CodeEventType::Exclusive] = new PlayingExclusiveEventExecutor();
		seekingEventExecutors[CodeEventType::UseRhythmPart] = new PlayingExclusiveEventExecutor();

		seekingEventExecutors[CodeEventType::MusicEnd] = new MusicEndEventExecutor();
	}

}}}



