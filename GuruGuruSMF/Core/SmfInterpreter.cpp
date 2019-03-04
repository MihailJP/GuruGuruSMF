
#include "stdafx.h"
#include "SmfInterpreter.h"

namespace GuruGuruSmf { namespace Core {
	
	// コンストラクタ
	SmfInterpreter::SmfInterpreter()
	{
		trackData = 0;
	}
	
	// デストラクタ
	SmfInterpreter::~SmfInterpreter()
	{
		delete[] trackData;
	}
	
	// 全トラックで最小のTickを取得。終了していたら-1を返す
	int SmfInterpreter::GetMinTick()
	{
		int tick = Constant::MaxTick;
		
		for (int i=0; i<trackCount; i++){
			TrackData& trk = trackData[i];
			if (trk.State == TrackState::End) continue;
			
			if (tick > trk.Tick) tick = trk.Tick;
		}
		
		if (tick < Constant::MaxTick){
			return tick;
		}else{
			return -1;	// MaxTick以上の場合でも終了
		}
	}
	
	// 指定のSMFで初期化
	void SmfInterpreter::Initialize(Smf* newSmf)
	{
		smf = newSmf;
		
		trackCount = smf->GetTrackCount();
		trackData = new TrackData[trackCount];
		for (int i=0; i<trackCount; i++){
			TrackData& trk = trackData[i];
			trk.SmfTrack = smf->GetTrack(i);
		}
		
		Rewind();
	}
	
	// 最初まで巻き戻し
	void SmfInterpreter::Rewind()
	{
		smf->Rewind();
		
		for (int i=0; i<trackCount; i++){
			TrackData& trk = trackData[i];
			
			// 最初のデルタタイムを取得
			GGSERROR err;
			trk.Tick = trk.SmfTrack->ReadDeltaTime(err);
			
			if (err == GgsError::NoError){
				trk.State = TrackState::Play;
			}else{
				trk.State = TrackState::End;
			}
		}
		
	}
	
	// イベントを解釈して処理
	GGSERROR SmfInterpreter::InterpretSmf()
	{
		while(1){
			int tick = GetMinTick();
			if (tick < 0) break;
			UpdateTick(tick);
			
			for (int i=0; i<trackCount; i++){
				TrackData& trk = trackData[i];
				if (trk.State == TrackState::End) continue;
				
				while (tick >= trk.Tick){
					GGSERROR err;
					SmfEvent* smfEvent = trk.SmfTrack->ReadSmfEvent(err);
					if (err == GgsError::TrackLimit || smfEvent->Type == SmfEventType::TrackEnd){
						trk.State = TrackState::End;
						break;
					}
					if (err != GgsError::NoError) return err;

					EventExecute(smfEvent);
					trk.Tick += smfEvent->DeltaTime;
				}
			}
		}
		return GgsError::NoError;
	}
	
	
	
}}


