// プレイヤーの基底クラス

#pragma once

#include "Interface.h"
#include "SmfController.h"

namespace GuruGuruSmf { namespace Core { namespace Player {

	/// <summary>
	/// GGSCodeイベント実行用の基底クラス(Playerが使用)
	/// </summary>
	class CodeEventExecutor
	{
	public:
		// 実行！
		virtual void Execute(CodeEvent* codeEvent, IPlayerCore* core) = 0;
	};
	
	/// <summary>
	/// 何も実行しない用
	/// </summary>
	class NullEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core){}
	};

	/// <summary>
	/// 終点用
	/// </summary>
	class MusicEndEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// デルタタイム用
	/// </summary>
	class DeltaTimeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// テンポ用
	/// </summary>
	class TempoEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: ノートオン用
	/// </summary>
	class PlayingNoteOnEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: ノートオフ用
	/// </summary>
	class PlayingNoteOffEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: プログラムチェンジ用
	/// </summary>
	class PlayingProgramChangeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: コントロールチェンジ用
	/// </summary>
	class PlayingControlChangeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: ボリューム用
	/// </summary>
	class PlayingVolumeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: ピッチベンド用
	/// </summary>
	class PlayingPitchBendEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: チャンネルプレッシャー用
	/// </summary>
	class PlayingCAfEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: ポリフォニックキープレッシャー用
	/// </summary>
	class PlayingPAfEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Play: エクスクルーシブ用
	/// </summary>
	class PlayingExclusiveEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	
	
	/// <summary>
	/// Seek: プログラムチェンジ用
	/// </summary>
	class SeekingProgramChangeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Seek: コントロールチェンジ用
	/// </summary>
	class SeekingControlChangeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Seek: ボリューム用
	/// </summary>
	class SeekingVolumeEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Seek: ピッチベンド用
	/// </summary>
	class SeekingPitchBendEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Seek: チャンネルプレッシャー用
	/// </summary>
	class SeekingCAfEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};
	
	/// <summary>
	/// Seek: ポリフォニックキープレッシャー用
	/// </summary>
	class SeekingPAfEventExecutor : public CodeEventExecutor
	{
	public:
		void Execute(CodeEvent* codeEvent, IPlayerCore* core);
	};


	/// <summary>
	/// プレイヤーの基底クラス
	/// </summary>
	class PlayerBase
	{
	private:
		CodeEventExecutor** playingEventExecutors;
		CodeEventExecutor** seekingEventExecutors;
		
		Smf* smf;
		GgsCode* code;
		
		int keyShift;
		int* pitches;
		bool* rhythmChannels;
		
		int masterPitch;
		
		bool doLoop, doSkipBeginning, doSkipEnding;
		bool doNormalize, canDoMasterPitch, doSendExclusive;

	protected:
		// SMFオブジェクト
		Smf* GetSmf(){ return smf; }
		
		// Codeオブジェクト
		GgsCode* GetCode(){ return code; }
		
		// ループするかどうか
		bool GetDoLoop(){ return doLoop; }
		
		// 冒頭の空白をスキップをするかどうか
		bool GetDoSkipBeginning(){ return doSkipBeginning; }
		
		// 末尾の空白をスキップするかどうか
		bool GetDoSkipEnding(){ return doSkipEnding; }
		
		// マスターピッチを有効にするかどうか
		bool GetCanDoMasterPitch(){ return canDoMasterPitch; }
		
		// ノーマライズするかどうか
		bool GetDoNormalize(){ return doNormalize; }
		
		// マスターピッチ
		void SetMasterPitchBase(int pitch){ masterPitch = pitch; }
		
		// キーシフト
		void SetKeyShift(int key){ keyShift = key; }
		
		// シフトされたノート番号を取得
		int GetShiftedNoteBase(int note, int channel);
		
		// マスターピッチを送信
		void SendMasterPitchBase(IDevice* device);
		
		// ピッチベンドメッセージを送信
		void SendPitchBendBase(IDevice* device, int channel, int pitch);
		
		// エクスクルーシブを送信
		void SendExclusiveBase(IDevice* device, CodeEvent* codeEvent);

		// ピッチベンドセンシビリティを初期化
		void InitializePitchBendSensitivity(IDevice* device);
		
		// SMFで初期化
		virtual void Initialize(Smf* newSmf);
		
		// 冒頭へ巻き戻し
		virtual void Rewind();
		
		// PlayOptionを処理
		void ParsePlayOption(int option);
		
		// イベントを実行(Play)
		void PlayingEventExecute(CodeEvent* ev, IPlayerCore* core);
		
		// イベントを実行(Seek)
		void SeekingEventExecute(CodeEvent* ev, IPlayerCore* core);
		
		// イベント実行オブジェクトの配列を作成(Play)
		void MakePlayCodeEventExcutors();
		
		// イベント実行オブジェクトの配列を作成(Seek)
		void MakeSeekCodeEventExcutors();
		
	public:
		PlayerBase();	// コンストラクタ
		virtual ~PlayerBase();	// デストラクタ
	};

}}}

