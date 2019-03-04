
#include "stdafx.h"
#include "PerformancePlayer.h"


namespace GuruGuruSmf {	namespace Dxmus	{

	// コンストラクタ
	PerformancePlayer::PerformancePlayer()
	{
		performance = 0;
		loader = 0;
		segment = 0;
		segStat = 0;
		isOpen = false;
		isPlaying = false;
		isPause = false;
	}

	// デストラクタ
	PerformancePlayer::~PerformancePlayer()
	{
		Close();
	}

	// 初期化
	GGSERROR PerformancePlayer::Open(HWND handle, int channels)
	{
		Close();

		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		isOpen = true;

		HRESULT hr;
		hr = CoCreateInstance(CLSID_DirectMusicPerformance,
						NULL, CLSCTX_INPROC,
						IID_IDirectMusicPerformance8,
						(void**)&performance);
		if(FAILED(hr)){
			Close();
			return GgsError::CannotOpenDevice;
		}

		hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC,
						IID_IDirectMusicLoader8, (void**)&loader);
		if(FAILED(hr)){
			Close();
			return GgsError::CannotOpenDevice;
		}


		hr = performance->InitAudio(
						NULL, NULL, (HWND)handle,
						DMUS_APATH_SHARED_STEREOPLUSREVERB,
						channels, DMUS_AUDIOF_ALL, NULL);
		if(FAILED(hr)){
			Close();
			return GgsError::CannotOpenDevice;
		}

		wchar_t path[MAX_PATH];
		if(GetCurrentDirectoryW(MAX_PATH, path) == 0){
			DWORD errCode = GetLastError();
			if(errCode == ERROR_CALL_NOT_IMPLEMENTED){	// Windows9x ハードコーディングです
				char pathA[MAX_PATH];
				GetCurrentDirectoryA(MAX_PATH, pathA);
				MultiByteToWideChar(CP_ACP, 0, pathA, -1, path, MAX_PATH);
			}
		}
		loader->SetSearchDirectory(GUID_DirectMusicAllTypes, path, FALSE);

		return GgsError::NoError;
	}

	// 後始末
	void PerformancePlayer::Close()
	{
		if(!isOpen) return;

		Stop();
		ReleaseSegment();

		if(loader){
			loader->Release();
			loader = NULL;
		}

		if(performance){
			performance->CloseDown();
			performance->Release();
			performance = NULL;
		}
	
		CoUninitialize();
		isOpen = false;
	}

	// SegmentとSegStatを開放
	void PerformancePlayer::ReleaseSegment()
	{
		if(segStat){
			segStat->Release();
			segStat = NULL;
		}

		if(segment){
			segment->Unload(performance);
			segment->Release();
			segment = NULL;
		}
	}

	// TickをMUSIC_TIMEに変換
	MUSIC_TIME PerformancePlayer::TickToMusicTime(int tick, int timeBase)
	{
		long long a = tick;
		a = a * DMUS_PPQ / timeBase;
		return (MUSIC_TIME)a;
	}

	// 演奏！
	GGSERROR PerformancePlayer::Play(PerformancePlaySmfData* smf, int option, int seek)
	{
		if(!isOpen) return GgsError::NotReady;
		Stop();

		HRESULT hr;

		if(smf->StoredType == Core::StoredType::File){	// ファイルから読み込み
			hr = loader->LoadObjectFromFile(CLSID_DirectMusicSegment,
						IID_IDirectMusicSegment8,
						(WCHAR*)smf->FileName, (LPVOID*)&segment);
		}else{	// メモリから読み込み
			DMUS_OBJECTDESC desc;
			ZeroMemory(&desc, sizeof(DMUS_OBJECTDESC));

			desc.dwSize = sizeof(DMUS_OBJECTDESC);
			desc.guidClass = CLSID_DirectMusicSegment;
			desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
			desc.llMemLength = smf->SmfBinaryLength;
			desc.pbMemData = (byte*)smf->SmfBinary;

			hr = loader->GetObject(&desc, IID_IDirectMusicSegment8, (LPVOID*)&segment);
		}

		if(FAILED(hr)){
			ReleaseSegment();
			return GgsError::NoSmf;
		}

		hr = segment->SetParam(GUID_StandardMIDIFile, 0xFFFFFFFF, 0, 0, NULL);
		if(FAILED(hr)){
			ReleaseSegment();
			return GgsError::Failed;
		}

		hr = segment->Download(performance);
		if(FAILED(hr)){
			ReleaseSegment();
			return GgsError::Failed;
		}

		bool isLoop = ((option & PlayOption::Loop) != 0);
		bool isSkipBeginning = ((option & PlayOption::SkipBeginning) != 0);
		bool isSkipEnding = ((option & PlayOption::SkipEnding) != 0);

		// 冒頭の空白スキップorシーク
		int startTick = 0;
		if(seek > 0){
			startTick = seek;
		}else if(isSkipBeginning){
			startTick = smf->FirstNoteTick;
		}
		segment->SetStartPoint(
				TickToMusicTime(startTick, smf->TimeBase));

		// ループ演奏
		if(isLoop){
			if(smf->LoopTick < smf->LastNoteTick){
				int loopStart, loopEnd;
				if(smf->LoopTick == -1){
					if(isSkipBeginning){
						loopStart = smf->FirstNoteTick;
					}else{
						loopStart = 0;
					}
				}else{
					loopStart = smf->LoopTick;
				}

				if(isSkipEnding){
					loopEnd = smf->LastNoteTick;
				}else{
					loopEnd = smf->TotalTick;
				}
				segment->SetRepeats(DMUS_SEG_REPEAT_INFINITE);
				segment->SetLoopPoints(
					TickToMusicTime(loopStart, smf->TimeBase),
					TickToMusicTime(loopEnd, smf->TimeBase));
			}
		}

		hr = performance->PlaySegmentEx(segment, NULL, NULL, 0, 0,
						(IDirectMusicSegmentState**)&segStat, NULL, NULL );
		if(FAILED(hr)){
			ReleaseSegment();
			return GgsError::Failed;
		}
		isPlaying = true;

		return GgsError::NoError;
	}

	// 一時停止
	void PerformancePlayer::Pause()
	{
		if(!isPlaying || isPause) return;

		pausedTime = GetNowMusicTime();
		performance->Stop(NULL,NULL,0,0);

		isPause = true;
	}

	// 一時停止解除
	void PerformancePlayer::Restart()
	{
		if(!isPause) return;

		segment->SetStartPoint(pausedTime);
		if (segStat){
			segStat->Release();
			segStat = NULL;
		}
		performance->PlaySegmentEx(segment, NULL, NULL, 0, 0, (IDirectMusicSegmentState**)&segStat, NULL, NULL);
		isPause = false;
	}

	// 停止！
	void PerformancePlayer::Stop()
	{
		isPause = false;
		if(!isPlaying) return;

		performance->Stop(NULL, NULL, 0 , 0);
		ReleaseSegment();
		isPlaying = false;
	}

	// マスターボリュームを設定
	void PerformancePlayer::SetMasterVolume(long volume)
	{
		performance->SetGlobalParam(GUID_PerfMasterVolume, &volume, sizeof(long));
	}

	// マスターテンポを設定
	void PerformancePlayer::SetMasterTempo(float tempo)
	{
		performance->SetGlobalParam(GUID_PerfMasterTempo, &tempo, sizeof(float));
	}

	// 演奏状態を取得
	void PerformancePlayer::GetStatus(PlayerStatus* status)
	{
		if(isPause){
			status->State = PlayerState::Pause;
		}else if(isPlaying){
			status->State = PlayerState::Play;
		}else{
			status->State = PlayerState::Stop;
		}

		// ver.0.3.3譲りの超手抜き
		status->Time = 0;
		status->Tick = 0;
		status->Tempo = 0;
		status->LoopCount = 0;
	}


	// 現在のMUSIC_TIME値を取得
	MUSIC_TIME PerformancePlayer::GetNowMusicTime()
	{
		if(!isPlaying) return 0;

		MUSIC_TIME startTime, startPoint, now;
		segStat->GetStartTime(&startTime);
		segStat->GetStartPoint(&startPoint);
		performance->GetTime(0, &now);

		MUSIC_TIME loopStart, loopEnd, length;
		DWORD repeats;
		segment->GetLoopPoints(&loopStart, &loopEnd);
		segment->GetLength(&length);
		segment->GetRepeats(&repeats);

		MUSIC_TIME offset = now - (startTime - startPoint);
		loopEnd = loopEnd ? loopEnd : length;

		if(repeats && loopStart < loopEnd && loopEnd > startPoint){
			if(repeats != DMUS_SEG_REPEAT_INFINITE && 
					offset > loopStart + (loopEnd -loopStart) * (int)repeats){
				offset -= (loopEnd - loopStart) * repeats;
			}else if(offset > loopStart){
				offset = loopStart + (offset - loopStart) % (loopEnd - loopStart);
			}
		}

		return offset;
	}

}}

