
#include "stdafx.h"
#include "Times.h"
#include <dxmus.core/Controller.h>


namespace GuruGuruSmf { namespace Core {

namespace Analyzer {
	
	//
	// PlayerTimes
	//
	
	// 冒頭に巻き戻し
	void AnalyzerTimes::Rewind()
	{
		SetTempo(Constant::DefaultTempo);
		markerTick = 0;
		markerTime = 0;
	}
	
	// TickやTimeを更新
	void AnalyzerTimes::Update(int tick)
	{
		markerTime += (tick - markerTick) * tempoA;
		markerTick = tick;
	}

}

namespace WindowsMultimedia
{
	//
	// PlayerTimes
	//
	
	// コンストラクタ
	PlayerTimes::PlayerTimes()
	{
		masterTempo = 1;

		timeBase = 96;
		SetTempo(Constant::DefaultTempo);
		nowTime = 0;
		nowTick = 0;
	}
	
	// テンポ(SMFでの表現値)
	void PlayerTimes::SetTempo(double tp)
	{
		tempo = tp;
		tempoA = tp / (timeBase * 1000);
		deltaTick = 1000 * timeBase * Constant::TimeInterval / tp;
	}
	
	// MarkerTickを更新
	void PlayerTimes::UpdateMarkerTick(int delta)
	{
		queueTick = markerTick;
		queueTime = markerTime;
		markerTick += delta;
		markerTime += delta * tempoA;
	}
	
	// 現在のTickを進める
	void PlayerTimes::UpdateNowTick()
	{
		nowTime += Constant::TimeInterval * masterTempo;
		nowTick += deltaTick * masterTempo;
	}
	
	// 現在の演奏時間を修正 シーク時に使用
	void PlayerTimes::FixNowTime(int tick)
	{
		nowTime = queueTime + (tick - queueTick) * tempoA;
		nowTick = tick;
	}
	
	// 最初の発音の位置へ移動
	void PlayerTimes::GotoFirstNote()
	{
		nowTime = firstNoteTime;
		markerTime = nowTime;
		nowTick = firstNoteTick;
	}
	
	// ループ位置へ移動
	void PlayerTimes::GotoLoopPoint(bool isSkipEnding)
	{
		markerTick = loopTick;
		markerTime = loopTime;
		
		double sTime = totalTime;
		double sTick = totalTick;
		
		if (isSkipEnding){
			sTime = lastNoteTime;
			sTick = lastNoteTick;
		}
		nowTime = loopTime + (nowTime - sTime);
		nowTick = loopTick + (nowTick - sTick);
	}
	
	// SMFで初期化
	void PlayerTimes::Initialize(Smf* smf)
	{
		timeBase = smf->GetTimeBase();
		
		loopTick = smf->GetData()->LoopTick;
		loopTime = smf->GetData()->LoopTime;
		firstNoteTick = smf->GetData()->FirstNoteTick;
		firstNoteTime = smf->GetData()->FirstNoteTime;
		lastNoteTick = smf->GetData()->LastNoteTick;
		lastNoteTime = smf->GetData()->LastNoteTime;
		totalTick = smf->GetData()->TotalTick;
		totalTime = smf->GetData()->TotalTime;
		
		queueTick = 0;
		queueTime = 0;
	}
	
	// 冒頭に巻き戻し
	void PlayerTimes::Rewind()
	{
		SetTempo(Constant::DefaultTempo);
		markerTick = 0;
		markerTime = 0;
		
		nowTick = 0;
		nowTime = 0;
	}
	
	
}

namespace DirectMusicPort {

	//
	// PlayerTimes
	//
	
	// コンストラクタ
	PlayerTimes::PlayerTimes()
	{
		queueDmTime = 0;
		fixedQueueDmTime = 0;
		masterTempo = 1;
		realtimeMode = false;

		timeBase = 96;
		SetTempo(Constant::DefaultTempo);
		nowTime = 0;
		nowTick = 0;
	}
	
	// 次のメッセージ送信予定時間
	__int64 PlayerTimes::GetMarkerDmTime(){
		if (realtimeMode){
			if(fixedQueueDmTime > markerDmTime-10000){
				// 通常の処理時間を追い抜かないように細工
				markerDmTime = fixedQueueDmTime + 10000;
			}
			return fixedQueueDmTime;
		}else{
			return markerDmTime;
		}
	}
	
	// DirectMusicから基準時間を読んでくる
	GGSERROR PlayerTimes::ReadMasterClock()
	{
		return GuruGuruSmf::Dxmus::Controller::GetInstance()->PortGetReferenceTime(&nowDmTime);
	}
	
	// 直前に何か送信していたらウェイトをつけて、してなければ今の時間をセット
	void PlayerTimes::SetMarkerDmTimeByQueueDmTime(__int64 wait)
	{
		__int64 qt = fixedQueueDmTime + wait;
		markerDmTime = qt > nowDmTime ? qt : nowDmTime;
	}
	
	// ループ位置へ移動
	void PlayerTimes::GotoLoopPoint()
	{
		markerTick = loopTick;
		markerTime = loopTime;
	}
		
	// 更新処理
	void PlayerTimes::UpdateMarkerTick(int delta)
	{
		queueDmTime = markerDmTime;
		fixedQueueDmTime = queueDmTime;
		queueTime = markerTime;
		queueTick = markerTick;
		
		markerTick += delta;
		double a = delta * tempo / timeBase;
		markerTime += a / 1000;
		
		// スキップ中はマーカータイムを加算しない
		if (!skipping){
			markerDmTime += (__int64)(a*10 / masterTempo);
		}
	}

	// 送信時間を更新
	void PlayerTimes::UpdateQueueDmTime()
	{
		queueDmTime = markerDmTime;
		fixedQueueDmTime = queueDmTime;
	}
	
	// 現在の時間を算出
	void PlayerTimes::CalcNowTime()
	{
		double a = (double)((nowDmTime + Constant::PrepareTime) - queueDmTime);
		a *= masterTempo;
		nowTime = queueTime + a / 10000;
		nowTick = queueTick + (int)(a * timeBase / (tempo * 10));
	}
	
	// 一時停止解除
	void PlayerTimes::Restart()
	{
		__int64 t = nowDmTime - pausedDmTime;
		markerDmTime += t;
		queueDmTime += t;
		fixedQueueDmTime = markerDmTime;
	}
	
	// SMFで初期化
	void PlayerTimes::Initialize(Smf* smf)
	{
		timeBase = smf->GetTimeBase();
		
		loopTick = smf->GetData()->LoopTick;
		loopTime = smf->GetData()->LoopTime;
		firstNoteTick = smf->GetData()->FirstNoteTick;
		firstNoteTime = smf->GetData()->FirstNoteTime;
		lastNoteTick = smf->GetData()->LastNoteTick;
		lastNoteTime = smf->GetData()->LastNoteTime;
		
		skipping = false;
	}
	
	// 冒頭に巻き戻し
	void PlayerTimes::Rewind()
	{
		SetTempo(Constant::DefaultTempo);
		markerTick = 0;
		markerTime = 0;
		
		queueTime = 0;
		queueTick = 0;
	}
	
	
}

}}



