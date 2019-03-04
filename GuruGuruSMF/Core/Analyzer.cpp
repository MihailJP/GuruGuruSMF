
#include "stdafx.h"
#include "Analyzer.h"

namespace GuruGuruSmf { namespace Core { namespace Analyzer {
	
	//
	// NoteOnEventExecutor
	//
	
	// ノートオン用
	void NoteOnEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		if (smfEvent->Data2 != 0){
			core->NoteEvent(true);	// ノートオン
		}else{
			core->NoteEvent(false);	// ノートオフ
		}
	}
	
	
	//
	// NoteOffEventExecutor
	//
	
	// ノートオフ用
	void NoteOffEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		core->NoteEvent(false);	// ノートオフ
	}
	
	
	//
	// TempoEventExecutor
	//
	
	// テンポ用
	void TempoEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		core->SetTempo(smfEvent->Data1);
	}
	
	
	//
	// ProgramChangeEventExecutor
	//
	
	// プログラムチェンジ用
	void ProgramChangeEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		core->ProgramChangeEvent(smfEvent->MidiStatus & 0x0f ,smfEvent->Data1);
	}
	
	
	//
	// ControlChangeEventExecutor
	//
	
	// コントロールチェンジ用
	void ControlChangeEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		switch (smfEvent->Data1){
		case 7:	// ボリューム
			core->VolumeEvent(smfEvent->Data2);
			break;
		case 0:	// バンクセレクトMSB
			core->BankSelectEvent(smfEvent->MidiStatus & 0x0f, true, smfEvent->Data2);
			break;
		case 32:	// バンクセレクトLSB
			core->BankSelectEvent(smfEvent->MidiStatus & 0x0f, false, smfEvent->Data2);
			break;
		case 111:	// ループ
			core->MarkLoopPoint();
			break;
		default:
			break;
		}
	}
	
	
	//
	// ExclusiveEventExecutor
	//
	//
	
	// エクスクルーシブ用
	void ExclusiveEventExecutor::Execute(SmfEvent* smfEvent, IAnalyzerCore* core)
	{
		int len = smfEvent->ExclusiveLength;
		if (len < 6) return;	// 長すぎたり短いのはまとめてスルー
		
		byte* exData = smfEvent->ExclusiveData;
		if (exData[len-1] != 0xf7){
			return;		// あやしいバイナリは無視
		}
		
		if (len == 10){
			if (exData[0] == 0x41 && exData[2] == 0x42 && exData[3] == 0x12 &&
					exData[4] == 0x40 && (exData[5]&0xf0) == 0x10 && exData[6] == 0x15){
				// GSのリズムモードメッセージを拾っておく
				int ch = Constant::GSChannel[exData[5] & 0x0f];	// チャンネルの並びが特殊
				core->UseRhythmPartEvent(ch, exData[7] != 0);
				return;
			}
		}
	}
	
	
	//
	// Analyzer
	//
	
	// コンストラクタ
	Analyzer::Analyzer()
	{
		times = new AnalyzerTimes();
		
		voiceBank = new uint[16];
		eventExecutors = new SmfEventExecutor*[SmfEventType::TrackEnd];
		
		eventExecutors[SmfEventType::NoteOff] = new NoteOffEventExecutor();
		eventExecutors[SmfEventType::NoteOn] = new NoteOnEventExecutor();
		eventExecutors[SmfEventType::ProgramChange] = new ProgramChangeEventExecutor();
		eventExecutors[SmfEventType::ControlChange] = new ControlChangeEventExecutor();
		eventExecutors[SmfEventType::PitchBend] = new NullEventExecutor();
		
		eventExecutors[SmfEventType::CAf] = new NullEventExecutor();
		eventExecutors[SmfEventType::PAf] = new NullEventExecutor();
		
		eventExecutors[SmfEventType::MetaEvent] = new NullEventExecutor();
		eventExecutors[SmfEventType::Tempo] = new TempoEventExecutor();
		eventExecutors[SmfEventType::Exclusive] = new ExclusiveEventExecutor();
	}
	
	// デストラクタ
	Analyzer::~Analyzer()
	{
		for(int i=0; i<SmfEventType::TrackEnd; i++){
			delete eventExecutors[i];
		}
		delete[] eventExecutors;
		
		delete times;
		delete[] voiceBank;
	}
	
	// Tickを進めて演奏時間を計算
	void Analyzer::UpdateTick(int tick)
	{
		times->Update(tick);
		++eventCount;
	}

	// イベント分岐
	void Analyzer::EventExecute(SmfEvent* smfEvent)
	{
		eventExecutors[smfEvent->Type]->Execute(smfEvent, this);
		++eventCount;
	}

	// 初期化処理
	void Analyzer::Initialize(Smf* newSmf)
	{
		smfData = newSmf->GetData();
		times->SetTimeBase(newSmf->GetTimeBase());
		
		SmfInterpreter::Initialize(newSmf);
		
	}

	// 冒頭へ巻き戻し
	void Analyzer::Rewind()
	{
		eventCount = 0;
		
		SmfInterpreter::Rewind();
		times->Rewind();
		
		smfData->LastNoteTick = -1;
		smfData->LastNoteTime = 0;
		smfData->FirstNoteTick = -1;
		smfData->FirstNoteTime = 0;
		smfData->LoopTick = -1;
		smfData->LoopTime = 0;
		
		
		for (int i=0; i<16; i++){
			voiceBank[i] = 0;
		}
		voiceBank[9] = 0x80000000;
		
		voiceList.clear();
		voiceList.insert(0);	// @0だけは強制的に追加
		voiceList.insert(0x80000000);	// リズム@0
	
		smfData->MaxVolume = 100;	// もっと厳密にやる手もありますがここはアバウトに
	}

	// ノートオンかノートオフのイベント
	void Analyzer::NoteEvent(bool isNoteOn)
	{
		if (isNoteOn && smfData->FirstNoteTick == -1){
			smfData->FirstNoteTick = times->GetMarkerTick();
			smfData->FirstNoteTime = times->GetMarkerTime();
		}
		smfData->LastNoteTick = times->GetMarkerTick();
		smfData->LastNoteTime = times->GetMarkerTime();
	}
	
	// プログラムチェンジイベント
	void Analyzer::ProgramChangeEvent(int channel, int voice)
	{
		uint newVoice = (voiceBank[channel] & 0xffffff00) | (uint)voice;
		voiceBank[channel] = newVoice;
		voiceList.insert(newVoice);
	}

	// バンクセレクトイベント
	void Analyzer::BankSelectEvent(int channel, bool isMsb, int bank)
	{
		uint newBank = voiceBank[channel];
		if (isMsb){
			newBank = (newBank & 0xff00ffff) | (uint)bank << 16;
		}else{
			newBank = (newBank & 0xffff00ff) | (uint)bank << 8;
		}
		voiceBank[channel] = newBank;
	}

	// リズムパートイベント
	void Analyzer::UseRhythmPartEvent(int channel, bool isRhythmPart)
	{
		uint newVoice = voiceBank[channel];
		if (isRhythmPart){
			newVoice |= (uint)0x80000000;
		}else{
			newVoice &= (uint)0x7fffffff;
		}
		voiceBank[channel] = newVoice;
		voiceList.insert(newVoice);
	}

	// ボリュームイベント
	void Analyzer::VolumeEvent(int volume)
	{
		if (smfData->MaxVolume < volume)
			smfData->MaxVolume = volume;
	}

	// ループポイントを発見
	void Analyzer::MarkLoopPoint()
	{
		if (smfData->LoopTick == -1){
			smfData->LoopTick = times->GetMarkerTick();
			smfData->LoopTime = times->GetMarkerTime();
		}
	}

	// 解析！
	GGSERROR Analyzer::Analyze(Smf* newSmf)
	{
		if (!newSmf->GetIsReady()) return GgsError::NotReady;
		
		Initialize(newSmf);
		
		GGSERROR err = InterpretSmf();
		if (err != GgsError::NoError) return err;
		
		smfData->EventCount = eventCount+1;
		smfData->TotalTick = times->GetMarkerTick();
		smfData->TotalTime = (int)times->GetMarkerTime();
		
		smfData->VoiceList->CopyFrom(&voiceList);
		

		// ループの種類を調べる
		if (smfData->LoopTick >= smfData->LastNoteTick){
			smfData->LoopType = LoopType::End;
		}else if (smfData->LoopTick == -1){
			smfData->LoopType = LoopType::Beginning;
		}else if (smfData->LoopTick == 0){
			smfData->LoopType = LoopType::LoopPointAtBeginning;
		}else{
			smfData->LoopType = LoopType::LoopPoint;
		}
		
		return GgsError::NoError;
	}




}}}


