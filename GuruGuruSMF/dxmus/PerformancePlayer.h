
#pragma once

#include <windows.h>
#include <dmusici.h>

namespace GuruGuruSmf {

	// �v���C���[�̏��
	struct PlayerStatus
	{
		int State;
		int Tempo;
		int Tick;
		int Time;
		int LoopCount;
	};

	// ���t�I�v�V����
	class PlayOption
	{
	public:
		static const int Loop = 1;			// ���[�v
		static const int SkipBegin = 2;		// �`���̋󔒂��X�L�b�v
		static const int SkipEnd = 4;		// �����̋󔒂��X�L�b�v
		static const int PitchDepth = 8;	// �s�b�`�f�v�X��L���ɂ���
	};

	/// ���t���
	class PlayerState
	{
	public:
		static const int Stop = 0;		// ��~��
		static const int Play = 1;		// ���t��
		static const int Pause = 2;		// �ꎞ��~��
	};


namespace Dxmus {

	// �f�[�^�̕ۑ��^�C�v
	class StoredType{
	public:
		static const int File = 0;
		static const int Memory = 1;
	};

	// DirectMusicPerformance�ɂ��v���C���[
	class PerformancePlayer
	{
	public:
		// Play�ɓn���f�[�^
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


