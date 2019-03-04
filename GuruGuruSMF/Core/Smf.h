// SMFデータ関連

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <set>

#include "Constants.h"

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// 音色リスト
	/// </summary>
	class VoiceList
	{
	private:
		int count;
		uint* items;

	public:
		VoiceList();	// コンストラクタ
		~VoiceList();	// デストラクタ
		
		// set配列からコピー
		void CopyFrom(std::set<uint>* src);

		// 要素数を取得
		int GetCount(){ return count; }

		// 要素を取得
		uint* GetItems(){ return items; }
	};



	/// <summary>
	/// SMFの解析データ
	/// </summary>
	struct SmfAnalyzedData
	{
		int TotalTick;
		int TotalTime;
		int FirstNoteTick;
		double FirstNoteTime;
		int LastNoteTick;
		double LastNoteTime;
		int LoopTick;
		double LoopTime;
		int MaxVolume;
		int EventCount;
		VoiceList* VoiceList;
		LOOPTYPE LoopType;
	};


	/// <summary>
	/// エクスクルーシブメッセージ1つ分(0xF0で始まるもの専用)
	/// </summary>
	class ExclusiveData
	{
	private:
		byte* binary;
		int length;

	public:
		ExclusiveData(byte* data, int len);	// コンストラクタ
		~ExclusiveData();	// デストラクタ

		byte* GetData(){ return binary; }
		int GetLength(){ return length; }
	};


	// GgsCodeのイベント1つ分(8バイト固定)
	struct CodeEvent
	{
		union
		{
			int Header;
			struct
			{
				unsigned char Type;
				unsigned char Channel;
				unsigned char Option1;
				unsigned char Option2;
			};
		};

		union
		{
			int IntData;
			struct
			{
				unsigned char ByteData1;
				unsigned char ByteData2;
				unsigned char ByteData3;
				unsigned char ByteData4;
			};
		};
	};

	/// <summary>
	/// GuruGuruSMF内部コードを扱う
	/// </summary>
	class GgsCode
	{
	private:
		std::vector<ExclusiveData*> exclusiveList;
		int length, position;
		int* binary;

	public:
		GgsCode();	// コンストラクタ
		~GgsCode();	// デストラクタ

		// 長さ
		int GetLength(){ return length; }
		
		// 現在の位置(ノーチェック)
		int GetPosition(){ return position; }
		void SetPosition(int pos){ position = pos; }
		
		// 長さを設定
		void SetLength(int newLength);
		
		// 現在の位置にイベント書き込み
		void WriteEvent(CodeEvent* ev);
		
		// 現在位置からイベント読み込み
		void ReadEvent(CodeEvent* ev);

		// 長さを0にしてエクスクルーシブデータを開放
		void Clear();
		
		// エクスクルーシブデータを登録しそのIDを取得
		int ResisterExclusive(ExclusiveData* exclusive);
		
		// エクスクルーシブデータを取得
		ExclusiveData* GetExclusive(int index);
	};

	/// <summary>
	/// SMFのイベント1つ分を扱う
	/// </summary>
	struct SmfEvent
	{
		SMF_EVENTTYPE Type;		// イベント
		int MidiStatus;			// MIDIステータス
		int DeltaTime;			// デルタタイム
		int Data1;				// データ1
		int Data2;				// データ2
		int EndPosition;		// トラックの終端(イベント取得時に使用)
		int ExclusiveLength;	// エクスクルーシブの長さ
		byte* ExclusiveData;	// エクスクルーシブデータ
	};


	/// <summary>
	/// SMFのバイナリデータを扱う
	/// </summary>
	class SmfStream
	{
	private:
		union Int32BE
		{
			int Value;
			byte B[4];
		};

		int position, length;
		byte* binary;
		const byte* baseAddress;
		bool isAllocate;

		// 可変長の数値を読み込む
		GGSERROR ReadVInt(int& intData, int endPosition);

		// 2バイトデータのイベントを取得
		GGSERROR ReadEvent2Data(SmfEvent* smfEvent, SMF_EVENTTYPE type);

		// 1バイトデータのイベントを取得
		GGSERROR ReadEvent1Data(SmfEvent* smfEvent, SMF_EVENTTYPE type);

		// ピッチベンドイベントを取得
		GGSERROR ReadPitchBend(SmfEvent* smfEvent);

		// エクスクルーシブイベントを取得(0xf0) 0xf7から始まるものは無視
		GGSERROR ReadExclusive(SmfEvent* smfEvent, int status);

		// メタイベントを取得
		GGSERROR ReadMetaEvent(SmfEvent* smfEvent);

	public:
		SmfStream();	// コンストラクタ
		~SmfStream();	// デストラクタ

		// 現在の位置
		int GetPosition(){ return position; }

		// データのポインタを取得
		const byte* GetBaseAddress(){ return baseAddress; }

		// 長さ
		int GetLength(){ return length; }

		// ファイルから読み込み
		GGSERROR LoadFromFile(LPCWSTR fileName);

		// メモリから読み込み
		GGSERROR LoadFromMemory(const byte* smfBinary, int newLength);

		// 破棄
		void Unload();

		// シーク
		GGSERROR Seek(int pos);

		// 現在位置から2バイトの数値を取得
		GGSERROR ReadUShort(int& intData);

		// 現在位置から4バイトの数値を取得
		GGSERROR ReadInt(int& intData);

		// 現在位置からデルタタイムを取得
		GGSERROR ReadDeltaTime(int& deltaTime, int endPosition);

		// 現在位置からSMFイベントを1つ＋デルタタイムを取得
		GGSERROR ReadSmfEvent(SmfEvent* smfEvent);

	};

	/// <summary>
	/// SMFのトラック1つ分を管理する
	/// </summary>
	class SmfTrack
	{
	private:
		SmfEvent smfEvent;
		SmfStream* smfStream;
		int beginPosition, position, length;

	public:
		// 初期化用構造体
		struct InitParam
		{
			SmfStream* SmfStream;
			int BeginPosition;
			int Length;
		};

		// 現在の位置
		int GetPosition(){ return position; }
		void SetPosition(int pos){ position = pos; }

		// 初期化
		void Initialize(InitParam* initParam);

		// 最初に巻き戻し
		void Rewind();

		// 現在位置からデルタタイムを取得
		int ReadDeltaTime(GGSERROR& error);

		// 現在位置からSMFイベントを1つ＋デルタタイムを取得
		SmfEvent* ReadSmfEvent(GGSERROR& error);

	};



	/// <summary>
	/// SMF1つ分を管理するクラス
	/// </summary>
	class Smf
	{
	private:
		bool isReady, buffered;
		SmfStream* smfStream;
		std::wstring smfFileName;
		int trackCount, timeBase;
		SmfTrack** tracks;
		GgsCode* code;
		VoiceList* voiceList;
		SmfAnalyzedData data;
		STOREDTYPE storedType;

		// ヘッダー読み込み
		GGSERROR ReadHeader();

		// 内部データをクリア
		void ClearData();

	public:
		Smf();	// コンストラクタ
		~Smf();	// デストラクタ

		// 解析データオブジェクト
		SmfAnalyzedData* GetData(){ return &data; }

		// GGSCodeコードオブジェクト
		GgsCode* GetCode(){ return code; }

		// メモリーを確保して常に保存しておくかどうか
		bool GetBuffered(){ return buffered; }
		void SetBuffered(bool b){ buffered = b; }

		// ファイル保存かメモリ保存か
		STOREDTYPE GetStoredType(){ return storedType; }

		// ファイル名
		const wchar_t* GetFileName(){ return smfFileName.c_str(); }

		// データのポインタを取得
		const byte* GetBaseAddres(){ return smfStream->GetBaseAddress(); }

		// 長さ
		int GetLength(){ return smfStream->GetLength(); }

		// SMFとして有効かどうか
		bool GetIsReady(){ return isReady; }

		// トラック数
		int GetTrackCount(){ return trackCount; }

		// タイムベース
		int GetTimeBase(){ return timeBase; }

		// ファイルから読み込み
		GGSERROR LoadFromFile(LPCWSTR fileName);

		// メモリから読み込み
		GGSERROR LoadFromMemory(const byte* smfBinary, int length);

		// SMFバイナリを破棄
		void ReleaseSmfBinary(){ smfStream->Unload(); }

		// コードバイナリを破棄
		void ReleaseCodeBinary(){ code->Clear(); }

		// 再読み込み
		GGSERROR Reload();

		// トラックオブジェクトを取得
		SmfTrack* GetTrack(int index){ return tracks[index]; }

		// 最初へ巻き戻し
		void Rewind();

	};



}}

