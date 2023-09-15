// GuruGuruSMF4の定数

#pragma once

typedef unsigned char byte;
typedef unsigned int uint;

namespace GuruGuruSmf {

	namespace PlayOption
	{
		const int Loop = 1;			// ループ再生する
		const int SkipBeginning = 2;	// 冒頭の空白をスキップ
		const int SkipEnding = 4;		// 末尾の空白をスキップ
		const int MasterPitch = 8;		// マスターピッチを有効にする
		const int Normalize = 16;		// 最大音量を127に引き上げる
		const int Exclusive = 32;		// エクスクルーシブを送信する
	}
	
	namespace LoadOption
	{
		const int Buffered = 1;		// データを内部に保持するか
	}
	
	namespace Device
	{
		const int DirectMusicPerformance = -1000;	// DirectMusic Performanceモード(廃止)
		const int DirectMusic = -1100;				// DirectMusic Portモード
	}
	
	/// <summary>
	/// エラーコード
	/// </summary>
	namespace GgsError {
		enum Code {
			NoError, 			// エラーなし
			Failed,				// なんだかよくわからないけど失敗
			FileNotFound,		// ファイルが存在しない
			CannotOpenFile,		// ファイルが開けない
			FileLoadFailed,		// ファイル読み込みに失敗
			BadFileSize,		// ファイルサイズが不正
			NoSmf,				// SMFではありません
			BadSmf,				// 不正なSMF
			UnmatchedSmf,		// 未対応のSMF
			CannotOpenDevice,	// デバイスが開けない
			CannotStartTimer,	// タイマーが開始できない
			IllegalPointer,		// 範囲外を参照しようとした
			TrackLimit,			// メッセージの途中でトラックが終端に達した
			NotReady,			// 演奏可能ではない
			UndefinedId,		// 未定義のID
			ApiNull				// APIが有効でない
		};
	}
	typedef GgsError::Code GGSERROR;
	

	/// <summary>
	/// プレイヤーの状態
	/// </summary>
	namespace PlayerState {
		enum Code {
			Stop,		// 停止中
			Play,		// 演奏中
			Pause,		// 一時停止中
			Suspended	// サスペンドによりデバイスが閉じられてます
		};
	}
	typedef PlayerState::Code PLAYERSTATE;

	/// <summary>
	/// プレイヤーの状態
	/// </summary>
	struct PlayerStatus
	{
		PLAYERSTATE State;
		int Tempo;
		int Tick;
		int Time;
		int LoopCount;
	};

	/// <summary>
	/// PerformancePlayへ渡すSMFデータ
	/// </summary>
	struct PerformancePlaySmfData
	{
		int StoredType;
		const wchar_t* FileName;
		void* SmfBinary;
		int SmfBinaryLength;
		int TimeBase;
		int TotalTick;
		int FirstNoteTick;
		int LastNoteTick;
		int LoopTick;
	};

	/// <summary>
	/// SMFのデータ
	/// </summary>
	struct SmfInformation
	{
		int TimeBase;		// ベースカウント
		int TotalTick;		// 総Tick
		int TotalTime;		// 総演奏時間
		int FirstNoteTick;	// 最初の発音tick
		int FirstNoteTime;	// 最初の発音の時間
		int LastNoteTick;	// 最後の発音tick
		int LastNoteTime;	// 最後の発音の時間
		int LoopTick;		// CC#111の位置
		int LoopTime;		// CC#111の位置の時間
	};

	/// <summary>
	/// SMFリストのキー一覧
	/// </summary>
	struct SmfListKeys
	{
		int Count;		// キーの数
		int* Keys;		// キーのリスト
	};

}


namespace GuruGuruSmf { namespace Core
{
	
	/// <summary>
	/// トラックの状態
	/// </summary>
	namespace TrackState {
		enum Code {
			Play,		// 演奏(解析)中
			End			// 終了
		};
	}
	typedef TrackState::Code TRACKSTATE;

	/// <summary>
	/// データの保存タイプ
	/// </summary>
	namespace StoredType {
		enum Code {
			File, Memory
		};
	}
	typedef StoredType::Code STOREDTYPE;
	
	/// <summary>
	/// SMFイベント定数
	/// </summary>
	namespace SmfEventType {
		enum Code {
			NoteOff, NoteOn, ProgramChange, ControlChange,
			PitchBend, CAf, PAf,
			MetaEvent, Tempo, Exclusive,
			TrackEnd
		};
	}

	typedef SmfEventType::Code SMF_EVENTTYPE;
	
	// エクスクルーシブメッセージ
	namespace Exclusive
	{
		// GMシステムオン
		const int GMSystemOnLength = 6;
		const byte GMSystemOn[6] = {0xf0,0x7e,0x7f,0x09,0x01,0xf7};
		
		// GSリセット
		const int GSResetLength = 11;
		const byte GSReset[11] = {0xf0,0x41,0x10,0x42,0x12,0x40,0x00,0x7f,0x00,0x41,0xf7};
	}
	
	// ループタイプ
	namespace LoopType {
		enum Code {
			Beginning,				// 最初へ(ループポイントなし)
			LoopPointAtBeginning,	// ループポイントで最初へ
			LoopPoint,				// ループポイントへ
			End						// 終了する
		};
	}

	typedef LoopType::Code LOOPTYPE;
	
	// GGSCodeのイベントタイプ
	namespace CodeEventType {
		enum Code {
			DeltaTime, NoteOn, NoteOff,
			ProgramChange, ControlChangeA, ControlChangeB, ControlChangeC, Volume,
			PAf, CAf, PitchBend,
			Tempo, Exclusive, UseRhythmPart,
			MusicEnd
		};
	}

	typedef CodeEventType::Code CODE_EVENTTYPE;
	
	// ボリュームコントローラの状態
	namespace VolumeState {
		enum Code {
			Normal, FadeIn, FadeOut
		};
	}

	typedef VolumeState::Code VOLUMESTATE;
	
	
	// その他定数
	namespace Constant {
		const int MaxTick = 0x70000000;		// 扱えるTickの最大値(とりあえずこの数字)
		const __int64 PrepareTime = 600000;		// Portモードで先処理する時間(60ms)
		const int TimeInterval = 5;			// 非同期処理周期(5ms)
		const double DefaultTempo = 500000;			// テンポの初期値(500000 = bpm120)
		const int GSChannel[] = {9,0,1,2,3,4,5,6,7,8,10,11,12,13,14,15};	// GS音源のch配列
	}


}}





