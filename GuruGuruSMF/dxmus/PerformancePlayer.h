
#pragma once

#include <windows.h>
#include <dmusici.h>

namespace GuruGuruSmf {

	// プレイヤーの状態
	struct PlayerStatus
	{
		int State;
		int Tempo;
		int Tick;
		int Time;
		int LoopCount;
	};

	// 演奏オプション
	class PlayOption
	{
	public:
		static const int Loop = 1;			// ループ
		static const int SkipBegin = 2;		// 冒頭の空白をスキップ
		static const int SkipEnd = 4;		// 末尾の空白をスキップ
		static const int PitchDepth = 8;	// ピッチデプスを有効にする
	};

	/// 演奏状態
	class PlayerState
	{
	public:
		static const int Stop = 0;		// 停止中
		static const int Play = 1;		// 演奏中
		static const int Pause = 2;		// 一時停止中
	};


namespace Dxmus {

	// データの保存タイプ
	class StoredType{
	public:
		static const int File = 0;
		static const int Memory = 1;
	};

	// DirectMusicPerformanceによるプレイヤー
	class PerformancePlayer
	{
	public:
		// Playに渡すデータ
		struct SmfData
		{
			int StoredType;
			wchar_t* FileName;
			void* SmfBinary;
			int SmfBinaryLength;
			int TimeBase;
			int TotalTick;
			int FirstNoteTick;
			int LastNoteTick;
			int LoopTick;
		};

	private:
		IDirectMusicPerformance8* performance;
		IDirectMusicLoader8* loader;
		IDirectMusicSegment8* segment;
		IDirectMusicSegmentState8* segStat;
		bool isOpen, isPlaying, isPause;

		MUSIC_TIME pausedTime;

		void ReleaseSegment();
		MUSIC_TIME TickToMusicTime(int tick, int timeBase);
		MUSIC_TIME GetNowMusicTime();
	
	public:
		PerformancePlayer();
		~PerformancePlayer();
		int Open(HWND handle, int channels);
		void Close();
		void Stop();
		int Play(SmfData* smf, int option, int seek);
		void Pause();
		void Restart();
		void SetMasterVolume(long volume);
		void SetMasterTempo(float tempo);
		void GetStatus(PlayerStatus* status);
		
	};
	

}}


