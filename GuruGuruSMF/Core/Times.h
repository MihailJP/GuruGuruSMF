// 時間関係のデータ

#pragma once

#include "Constants.h"
#include "Interface.h"
#include "Smf.h"

namespace GuruGuruSmf { namespace Core {
namespace Analyzer {
	
	// 時間関係を扱うクラス(Analyzer)
	class AnalyzerTimes
	{
	private:
		double timeBaseA, tempoA;
		int markerTick;
		double markerTime;
		
	public:
		// タイムベース
		void SetTimeBase(int tb){ timeBaseA = tb * 1000; }
		
		// テンポ(要TimeBase)
		void SetTempo(double tp){ tempoA = tp / timeBaseA; }
		
		// 処理中のTick
		int GetMarkerTick(){ return markerTick; }
		
		// 処理中の演奏時間
		double GetMarkerTime(){ return markerTime; }
		
		// 冒頭に巻き戻し
		void Rewind();
		
		// TickやTimeを更新
		void Update(int tick);
	};
}

namespace WindowsMultimedia
{
	
	// 時間関係を扱うクラス(WindowsMultimedia.Player)
	class PlayerTimes
	{
	private:
		int timeBase;
		double tempo, tempoA, masterTempo, deltaTick;
		int markerTick, queueTick;
		double nowTime, nowTick, markerTime, queueTime;
		int loopTick, firstNoteTick, lastNoteTick, totalTick;
		double loopTime, firstNoteTime, lastNoteTime, totalTime;
		
	public:
		// コンストラクタ
		PlayerTimes();
		
		// テンポ(SMFでの表現値)
		void SetTempo(double tp);
		
		// マスターテンポ
		void SetMasterTempo(double mt){ masterTempo = mt; }
		
		// 現在のTick
		int GetNowTick(){ return (int)(nowTick + 0.001); }
		
		// 現在の演奏時間
		int GetNowTime(){ return (int)(nowTime + 0.001); }
		
		// BPM表記でのテンポ値を取得
		int GetMusicTempo(){ return (int)(60000000 / tempo + 0.001); }
		
		// MarkerTickより以後ならtrue
		bool PassedMarkerTick(){ return nowTick >= markerTick; }
		
		// ループ位置以降ならtrue
		bool PassedLoopTick(){ return markerTick >= loopTick; }
		
		// 最初の音以降ならtrue
		bool PassedFirstNote(){ return markerTick >= firstNoteTick; }
		
		// 最後の音より後ならtrue
		bool PassedLastNote(){ return markerTick > lastNoteTick; }
		
		// 指定のTick以降ならtrue
		bool PassedTick(int tick){ return markerTick >= tick; }
		
		// MarkerTickを更新
		void UpdateMarkerTick(int delta);
		
		// 現在のTickを進める
		void UpdateNowTick();
		
		// 現在の演奏時間を修正 シーク時に使用
		void FixNowTime(int tick);
		
		// 最初の発音の位置へ移動
		void GotoFirstNote();
		
		// ループ位置へ移動
		void GotoLoopPoint(bool isSkipEnding);
		
		// SMFで初期化
		void Initialize(Smf* smf);
		
		// 冒頭に巻き戻し
		void Rewind();
		
	};

}

namespace DirectMusicPort {

	// 時間関係を扱うクラス(DirectMusicPort.Player)
	class PlayerTimes : public IDirectMusicTimer
	{
	private:
		bool skipping, realtimeMode;
		int timeBase;
		double tempo, masterTempo;
		__int64 markerDmTime, queueDmTime , nowDmTime, fixedQueueDmTime, pausedDmTime;
		double markerTime, queueTime, nowTime;
		int markerTick, queueTick, nowTick;
		int loopTick, firstNoteTick, lastNoteTick;
		double loopTime, firstNoteTime, lastNoteTime;
		
	public:
		PlayerTimes();	// コンストラクタ
		
		//
		// IDirectMusicTimerを実装
		//
		
		// 次のメッセージ送信予定時間
		__int64 GetMarkerDmTime();

		// 送信時間に1加算(データ送信時用)
		void IncrementMarkerDmTime(){ markerDmTime++; fixedQueueDmTime++; }
		
		//
		
		// テンポ(SMFでの表現値)
		void SetTempo(double tp){ tempo = tp; }
		
		// マスターテンポ
		void SetMasterTempo(double mt){ masterTempo = mt; }
		
		// 処理中のTick
		void SetMarkerTick(int mt){ markerTick = mt; }
		
		// 処理中の演奏時間
		void SetMarkerTime(double mt){ markerTime = mt; }
		
		// 現在のTick(計算によって算出)
		int GetNowTick(){ return nowTick; }
		
		// 現在の演奏時間(計算によって算出)
		double GetNowTime(){ return nowTime; }
		
		// BPM表記でのテンポ値を取得
		int GetMusicTempo(){ return (int)(60000000 / tempo + 0.001); }
		
		// DirectMusicから基準時間を読んでくる
		void ReadMasterClock();
		
		// 飛ばし処理をする時はtrue
		void SetSkipping(bool s){ skipping = s; }
		
		// 直前に何か送信していたらウェイトをつけて、してなければ今の時間をセット
		void SetMarkerDmTimeByQueueDmTime(__int64 wait);
		
		// 送信時間にウェイトをかける
		void Wait(long wait){ markerDmTime += wait; }
		
		// 処理済み時間まで巻き戻す
		void RewindQueueTime(){ markerDmTime = queueDmTime; }
		
		// ループ位置へ移動
		void GotoLoopPoint();
		
		// 更新処理
		void UpdateMarkerTick(int delta);

		// 送信時間を更新
		void UpdateQueueDmTime();
		
		// 現在の時間を算出
		void CalcNowTime();
		
		// メッセージ処理時間を過ぎていたらtrue
		bool PassedPrepareTime(){ return markerDmTime > nowDmTime + Constant::PrepareTime; }
		
		// ループ位置以降ならtrue
		bool PassedLoopTick(){ return markerTick >= loopTick; }
		
		// 最初の音以降ならtrue
		bool PassedFirstNote(){ return markerTick >= firstNoteTick; }
		
		// 最後の音より後ならtrue
		bool PassedLastNote(){ return markerTick > lastNoteTick; }
		
		// 指定のTick以降ならtrue
		bool PassedTick(int tick){ return markerTick >= tick; }
		
		// 一時停止
		void Pause(){ pausedDmTime = nowDmTime; }
		
		// 一時停止解除
		void Restart();
		
		// QueueDmTimeの補正
		void FixQueueDmTime(){ fixedQueueDmTime += Constant::TimeInterval * 10000; }
		
		// リアルタイムモードに入る
		void EnterRealtimeMode(){ realtimeMode = true; }
		
		// リアルタイムモード解除
		void LeaveRealtimeMode(){ realtimeMode = false; }
		
		// SMFで初期化
		void Initialize(Smf* smf);
		
		// 冒頭に巻き戻し
		void Rewind();
		
		
		
	};


}

}}


