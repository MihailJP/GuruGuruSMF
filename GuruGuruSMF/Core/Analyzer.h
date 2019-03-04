// SMF解析

#pragma once

#include <set>

#include "Constants.h"
#include "Interface.h"
#include "Times.h"
#include "SmfInterpreter.h"


namespace GuruGuruSmf { namespace Core { namespace Analyzer {
	
	/// <summary>
	/// SMFイベント実行用の基底クラス(Analyzer)
	/// </summary>
	class SmfEventExecutor
	{
	public:
		// 実行！
		virtual void Execute(SmfEvent* smfEvent, IAnalyzerCore* core) = 0;
	};
	
	/// <summary>
	/// 何も実行しない用
	/// </summary>
	class NullEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core){}
	};
	
	/// <summary>
	/// ノートオン用
	/// </summary>
	class NoteOnEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	/// <summary>
	/// ノートオフ用
	/// </summary>
	class NoteOffEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	/// <summary>
	/// テンポ用
	/// </summary>
	class TempoEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	/// <summary>
	/// プログラムチェンジ用
	/// </summary>
	class ProgramChangeEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	/// <summary>
	/// コントロールチェンジ用
	/// </summary>
	class ControlChangeEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	
	/// <summary>
	/// エクスクルーシブ用
	/// </summary>
	class ExclusiveEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, IAnalyzerCore* core);
	};
	
	
	/// <summary>
	/// SMF解析
	/// </summary>
	class Analyzer : public SmfInterpreter, public IAnalyzerCore
	{
	private:
		SmfEventExecutor** eventExecutors;
		AnalyzerTimes* times;
		int eventCount;
		SmfAnalyzedData* smfData;
		
		uint* voiceBank;
		std::set<uint> voiceList;

	protected:
		// Tickを進めて演奏時間を計算
		void UpdateTick(int tick);

		// イベント分岐
		void EventExecute(SmfEvent* smfEvent);

		// 初期化処理
		void Initialize(Smf* newSmf);

		// 冒頭へ巻き戻し
		void Rewind();

	public:
		Analyzer();		// コンストラクタ
		~Analyzer();	// デストラクタ

		//
		// IAnalyzerCoreを実装
		//
		
		// テンポを設定
		void SetTempo(int tempo){ times->SetTempo(tempo); }

		// ノートオンかノートオフのイベント
		void NoteEvent(bool isNoteOn);

		// プログラムチェンジイベント
		void ProgramChangeEvent(int channel, int voice);

		// バンクセレクトイベント
		void BankSelectEvent(int channel, bool isMsb, int bank);

		// リズムパートイベント
		void UseRhythmPartEvent(int channel, bool isRhythmPart);

		// ボリュームイベント
		void VolumeEvent(int volume);

		// ループポイントを発見
		void MarkLoopPoint();

		//

		// 解析！
		GGSERROR Analyze(Smf* newSmf);


	};
	
	
	
	
	
	
}}}




