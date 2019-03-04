
#include "stdafx.h"

#include "Compiler.h"


namespace GuruGuruSmf { namespace Core { namespace Compiler {

	//
	// TempoEventExecutor
	//
	
	// テンポ用
	void TempoEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		CodeEvent ev;
		ev.Type = CodeEventType::Tempo;
		ev.IntData = smfEvent->Data1;
		core->GetCode()->WriteEvent(&ev);
	}
	

	//
	// NoteOnEventExecutor
	//
	
	// ノートオン用
	void NoteOnEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		if (smfEvent->Data2 != 0){
			core->Write2ByteEvent(CodeEventType::NoteOn, smfEvent);
		}else{
			core->Write2ByteEvent(CodeEventType::NoteOff, smfEvent);
		}
	}
	

	//
	// NoteOffEventExecutor
	//
	
	// ノートオフ用
	void NoteOffEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		core->Write2ByteEvent(CodeEventType::NoteOff, smfEvent);
	}
	

	//
	// ProgramChangeEventExecutor
	//

	// プログラムチェンジ用
	void ProgramChangeEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		CodeEvent ev;
		ev.Type = CodeEventType::ProgramChange;
		ev.Channel = (byte)(smfEvent->MidiStatus & 0x0f);
		ev.ByteData1 = (byte)smfEvent->Data1;
		core->GetCode()->WriteEvent(&ev);
	}
	

	//
	// ControlChange
	//

	// コントロールチェンジ用
	void ControlChangeEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		CodeEvent ev;
		ev.Channel = (byte)(smfEvent->MidiStatus & 0x0f);
		ev.ByteData1 = (byte)smfEvent->Data1;
		ev.ByteData2 = (byte)smfEvent->Data2;
		
		switch(smfEvent->Data1){
		case 7:	// ボリューム
			ev.Type = CodeEventType::Volume;
			break;
		case 0: case 6: case 32: case 38: case 98: case 99:
		case 100: case 101: case 126: case 127:	// Seek時も送信するタイプ
			ev.Type = CodeEventType::ControlChangeB;
			break;
		case 120: case 121: case 123: case 124: case 125:	// 特殊なタイプ
			ev.Type = CodeEventType::ControlChangeC;
			break;
		case 111:	// ループ
			return;
		default:	// 通常のコントロールチェンジ
			ev.Type = CodeEventType::ControlChangeA;
			break;
		}
		core->GetCode()->WriteEvent(&ev);

	}
	

	//
	// PitchBendEventExecutor
	//

	// ピッチベンド用
	void PitchBendEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		CodeEvent ev;
		ev.Type = CodeEventType::PitchBend;
		ev.Channel = (byte)(smfEvent->MidiStatus & 0x0f);
		ev.IntData = smfEvent->Data1;
		core->GetCode()->WriteEvent(&ev);
	}
	

	//
	// CAfEventExecutor
	//

	// チャンネルプレッシャー用
	void CAfEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		CodeEvent ev;
		ev.Type = CodeEventType::CAf;
		ev.Channel = (byte)(smfEvent->MidiStatus & 0x0f);
		ev.ByteData1 = (byte)smfEvent->Data1;
		core->GetCode()->WriteEvent(&ev);
	}
	
	
	//
	// PAfEventExecutor
	//

	// ポリフォニックキープレッシャー用
	void PAfEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		core->Write2ByteEvent(CodeEventType::PAf, smfEvent);
	}
	
	
	//
	// ExclusiveEventExecutor
	//

	// エクスクルーシブ解析用
	static const byte codeGMSystemOn[] = {0x7e,0x7f,0x09};
	static const byte codeMasterVolume[] = {0x7f,0x7f,0x04,0x01};
	static const byte codeXGSystemOn[] = {0x43,0xff,0x4c,0x00,0x00,0x7e,0x00};
	static const byte codeGS[] = {0x41,0xff,0x42,0x12};
	static const byte codeGSReset[] = {0x40,0x00,0x7f};
	static const byte codeGSModeSet[] = {0x00,0x00,0x7f};

	// エクスクルーシブ用
	void ExclusiveEventExecutor::Execute(SmfEvent* smfEvent, ICompilerCore* core)
	{
		int len = smfEvent->ExclusiveLength;
		if (len < 5) return;	// 長すぎたり短いのはまとめてスルー
		
		byte* data = smfEvent->ExclusiveData;
		if (data[len-1] != 0xf7){
			return;		// あやしいバイナリは無視
		}
		
		// 解析
		if (len == 5){
			if (CompareExclusive(data, codeGMSystemOn, 0, 3)){
				TRACE(L"GM System On\n");
				return;	// GMシステムオン
			}
		}else if (len == 7){
			if (CompareExclusive(data, codeMasterVolume, 0, 4)){
				TRACE(L"MasterVolume\n");
				return;	// マスターボリューム Portモードで動作が不安定らしい
			}
		}else if (len == 8){
			if (CompareExclusive(data, codeXGSystemOn, 0, 7)){
				TRACE(L"XG System On\n");
				return;	// XGシステムオン
			}
		}else if (len == 10){
			if (CompareExclusive(data, codeGS, 0, 4)){	// GS系
				if (CompareExclusive(data, codeGSReset, 4, 3)){
					TRACE(L"GS Reset\n");
					return;	// GSリセット
				}else if (CompareExclusive(data, codeGSModeSet, 4, 3)){
					TRACE(L"GS Mode Set\n");
					return;	// GSモードセットのどちらか
				}else if (data[4] == 0x40 && (data[5]&0xf0) == 0x10 && data[6] == 0x15){
					// リズムパート指定
					int ch = Constant::GSChannel[data[5] & 0x0f];
					core->WriteUseRhythmPartEvent(smfEvent, ch, data[7] != 0);
					TRACE2(L"Use Rhythm Part[%d] = %d\n", ch, data[7]!=0);
					return;
				}
			}
		}
		
		core->WriteExclusiveEvent(smfEvent);
	}
	
		
	// データを比較する
	bool ExclusiveEventExecutor::CompareExclusive(const byte* data, const byte* compare, int position, int length)
	{
		for (int i=0; i<length; i++){
			if (compare[i] == 0xff) continue;
			if (data[position+i] != compare[i]) return false;
		}
		return true;
	}
	

	//
	// Compiler
	//

	// コンストラクタ
	Compiler::Compiler()
	{
		eventExecutors = new SmfEventExecutor*[SmfEventType::TrackEnd];
		
		eventExecutors[SmfEventType::NoteOff] = new NoteOffEventExecutor();
		eventExecutors[SmfEventType::NoteOn] = new NoteOnEventExecutor();
		eventExecutors[SmfEventType::ProgramChange] = new ProgramChangeEventExecutor();
		eventExecutors[SmfEventType::ControlChange] = new ControlChangeEventExecutor();
		eventExecutors[SmfEventType::PitchBend] = new PitchBendEventExecutor();
		
		eventExecutors[SmfEventType::CAf] = new CAfEventExecutor();
		eventExecutors[SmfEventType::PAf] = new PAfEventExecutor();
		
		eventExecutors[SmfEventType::MetaEvent] = new NullEventExecutor();
		eventExecutors[SmfEventType::Tempo] = new TempoEventExecutor();
		eventExecutors[SmfEventType::Exclusive] = new ExclusiveEventExecutor();
	}
	
	// デストラクタ
	Compiler::~Compiler()
	{
		for(int i=0; i<SmfEventType::TrackEnd; i++){
			delete eventExecutors[i];
		}
		delete[] eventExecutors;
	}
	
	// Tickを進める
	void Compiler::UpdateTick(int newTick)
	{
		int delta = newTick - markerTick;
		if (delta == 0) return;
		
		CodeEvent ev;
		ev.Type = CodeEventType::DeltaTime;
		ev.IntData = delta;
		code->WriteEvent(&ev);
		
		markerTick = newTick;
	}

	// イベント分岐
	void Compiler::EventExecute(SmfEvent* smfEvent)
	{
		eventExecutors[smfEvent->Type]->Execute(smfEvent, this);
	}

	// 初期化処理
	void Compiler::Initialize(Smf* newSmf)
	{
		code = newSmf->GetCode();
		code->SetLength(newSmf->GetData()->EventCount);
		
		SmfInterpreter::Initialize(newSmf);
		
		markerTick = 0;
	}

	// 2バイトのイベントを書き込み
	void Compiler::Write2ByteEvent(CODE_EVENTTYPE type, SmfEvent* smfEvent)
	{
		CodeEvent ev;
		ev.Type = type;
		ev.Channel = (byte)(smfEvent->MidiStatus & 0x0f);
		ev.ByteData1 = (byte)smfEvent->Data1;
		ev.ByteData2 = (byte)smfEvent->Data2;
		code->WriteEvent(&ev);
	}

	// エクスクルーシブイベントを書き込み
	void Compiler::WriteExclusiveEvent(SmfEvent* smfEvent)
	{
		ExclusiveData* ex = new ExclusiveData(smfEvent->ExclusiveData, smfEvent->ExclusiveLength);
		int id = code->ResisterExclusive(ex);
		
		CodeEvent ev;
		ev.Type = CodeEventType::Exclusive;
		ev.IntData = id;
		code->WriteEvent(&ev);
	}

	// リズムパートイベントを書き込む
	void Compiler::WriteUseRhythmPartEvent(SmfEvent* smfEvent, int channel, bool rhythm)
	{
		ExclusiveData* ex = new ExclusiveData(smfEvent->ExclusiveData, smfEvent->ExclusiveLength);
		int id = code->ResisterExclusive(ex);
		
		CodeEvent ev;
		ev.Type = CodeEventType::UseRhythmPart;
		ev.Channel = (byte)channel;
		if (rhythm){
			ev.Option1 = 1;
		}else{
			ev.Option1 = 0;
		}
		ev.IntData = id;
		code->WriteEvent(&ev);
	}

	// 変換！
	GGSERROR Compiler::Compile(Smf* newSmf)
	{
		if (!newSmf->GetIsReady()) return GgsError::NotReady;
		
		Initialize(newSmf);
		
		GGSERROR err = InterpretSmf();
		if (err != GgsError::NoError) return err;
		
		CodeEvent ev;
		ev.Type = CodeEventType::MusicEnd;
		code->WriteEvent(&ev);
		return GgsError::NoError;
	}

	
}}}
