// SMF操作

#pragma once

#include "Constants.h"
#include "Smf.h"

namespace GuruGuruSmf { namespace Core {
	
	/// <summary>
	/// SMF操作基底クラス
	/// </summary>
	class SmfInterpreter
	{
	public:
		/// <summary>
		/// 1トラック分のデータ
		/// </summary>
		struct TrackData
		{
			SmfTrack* SmfTrack;		// トラックオブジェクトの参照
			TRACKSTATE State;				// トラックの状態
			int Tick;				// Tick
		};
		
	private:
		Smf* smf;
		int trackCount;
		TrackData* trackData;
		
		// 全トラックで最小のTickを取得。終了していたら-1を返す
		int GetMinTick();
		
	protected:
		SmfInterpreter();	// コンストラクタ
		virtual ~SmfInterpreter();	// デストラクタ
		
		// SMFオブジェクト置き場
		Smf* GetSmf(){ return smf; }
		
		// トラック数
		int GetTrackCount(){ return trackCount; }
		
		// 指定のSMFで初期化
		virtual void Initialize(Smf* newSmf);
		
		// 最初まで巻き戻し
		virtual void Rewind();
		
		// Tickを進める
		virtual void UpdateTick(int newTick) = 0;
		
		// イベント分岐
		virtual void EventExecute(SmfEvent* smfEvent) = 0;
		
		// イベントを解釈して処理
		GGSERROR InterpretSmf();
		
		
	};
	
	
}}




