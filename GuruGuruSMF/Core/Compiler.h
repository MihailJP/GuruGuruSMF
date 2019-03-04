// SMF→内部コード変換

#pragma once

#include "Constants.h"
#include "Interface.h"
#include "SmfInterpreter.h"


namespace GuruGuruSmf { namespace Core { namespace Compiler {

	/// <summary>
	/// SMFイベント実行用の基底クラス(Compiler)
	/// </summary>
	class SmfEventExecutor
	{
	public:
		// 実行！
		virtual void Execute(SmfEvent* smfEvent, ICompilerCore* core) = 0;
	};

	/// <summary>
	/// 何も実行しない用
	/// </summary>
	class NullEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core){}
	};
	
	/// <summary>
	/// テンポ用
	/// </summary>
	class TempoEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// ノートオン用
	/// </summary>
	class NoteOnEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// ノートオフ用
	/// </summary>
	class NoteOffEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// プログラムチェンジ用
	/// </summary>
	class ProgramChangeEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// コントロールチェンジ用
	/// </summary>
	class ControlChangeEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};

	/// <summary>
	/// ピッチベンド用
	/// </summary>
	class PitchBendEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};

	/// <summary>
	/// チャンネルプレッシャー用
	/// </summary>
	class CAfEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// ポリフォニックキープレッシャー用
	/// </summary>
	class PAfEventExecutor : public SmfEventExecutor
	{
	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
	};
	
	/// <summary>
	/// エクスクルーシブ用
	/// </summary>
	class ExclusiveEventExecutor : public SmfEventExecutor
	{
	private:
		// データを比較する
		bool CompareExclusive(const byte* data, const byte* compare, int position, int length);

	public:
		void Execute(SmfEvent* smfEvent, ICompilerCore* core);
		
	};

	/// <summary>
	/// SMFを変換
	/// </summary>
	class Compiler : public SmfInterpreter, public ICompilerCore
	{
	private:
		GgsCode* code;
		SmfEventExecutor** eventExecutors;
		int markerTick;

	protected:
		// Tickを進める
		void UpdateTick(int newTick);
		
		// イベント分岐
		void EventExecute(SmfEvent* smfEvent);
		
		// 初期化処理
		void Initialize(Smf* newSmf);
	
	public:
		Compiler();		// コンストラクタ
		~Compiler();	// デストラクタ

		//
		// ICompilerCoreを実装
		//
		
		// GGSCodeオブジェクト
		GgsCode* GetCode(){ return code; }
		
		// 2バイトのイベントを書き込み
		void Write2ByteEvent(CODE_EVENTTYPE type, SmfEvent* smfEvent);

		// エクスクルーシブイベントを書き込み
		void WriteExclusiveEvent(SmfEvent* smfEvent);
		
		// リズムパートイベントを書き込む
		void WriteUseRhythmPartEvent(SmfEvent* smfEvent, int channel, bool rhythm);

		//

		// 変換！
		GGSERROR Compile(Smf* newSmf);
		
		
	};





}}}



