#include "stdafx.h"
#include <dmksctrl.h>
#include "Port.h"


namespace GuruGuruSmf {	namespace Dxmus
{

	// コンストラクタ
	Dls::Dls(IDirectMusicLoader8* ldr)
	{
		loader = ldr;
		collection = 0;
	}

	// デストラクタ
	Dls::~Dls()
	{
		if (collection){
			collection->Release();
			collection = 0;
		}
	}

	// ファイルから読み込み
	GGSERROR Dls::LoadFromFile(wchar_t *fileName)
	{
		HRESULT hr;
		hr = loader->LoadObjectFromFile(CLSID_DirectMusicCollection, IID_IDirectMusicCollection8,
								fileName, (LPVOID*)&collection );
		if(FAILED(hr))	return GgsError::FileLoadFailed;	// 読み込み失敗

		return GgsError::NoError;	// 成功
	}

	// 音色取り出し
	int Dls::GetInstrument(int voice, IDirectMusicInstrument8** instrument)
	{
		if(collection == 0)	return -1;

		return collection->GetInstrument(voice, instrument);
	}




	// コンストラクタ
	Port::Port()
	{
		music = 0;
		port = 0;
		buffer = 0;
		loader = 0;
		defaultDls = 0;
		clock = 0;

		downloadedList = 0;
		downloadedCount = 0;

		isOpen = false;
	}

	// デストラクタ
	Port::~Port()
	{
		Close();
	}

	// 初期化
	GGSERROR Port::Open(HWND handle)
	{
		Close();

		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		isOpen = true;

		HRESULT hr;
		hr = CoCreateInstance(CLSID_DirectMusic ,NULL ,
					CLSCTX_INPROC_SERVER ,IID_IDirectMusic8,
					(LPVOID*)&music);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;
		hr = music->SetDirectSound(NULL ,handle);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;

		//ポート作成
		DMUS_PORTPARAMS param;
		ZeroMemory(&param, sizeof(DMUS_PORTPARAMS));
		param.dwSize = sizeof(DMUS_PORTPARAMS);
		param.dwValidParams = DMUS_PORTPARAMS_VOICES
			| DMUS_PORTPARAMS_CHANNELGROUPS
			| DMUS_PORTPARAMS_SAMPLERATE
			| DMUS_PORTPARAMS_EFFECTS;
		param.dwVoices = 128;
		param.dwChannelGroups = 1;
		param.dwSampleRate = 44100;
		param.dwEffectFlags = DMUS_EFFECT_REVERB | DMUS_EFFECT_CHORUS;

		GUID guidPort;
		hr = music->GetDefaultPort(&guidPort);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;
		hr = music->CreatePort(guidPort, &param, &port, NULL);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;
		port->Activate(TRUE);

		// バッファ作成
		DMUS_BUFFERDESC bufferDesc;

		bufferDesc.dwSize = sizeof(DMUS_BUFFERDESC);
		bufferDesc.dwFlags = 0;
		bufferDesc.guidBufferFormat = GUID_NULL;
		bufferDesc.cbBuffer = 128;
		hr = music->CreateMusicBuffer(&bufferDesc, &buffer, NULL);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;

		// タイマ取得
		hr = port->GetLatencyClock(&clock);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;

		// ローダー作成
		hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL,
					CLSCTX_INPROC_SERVER, IID_IDirectMusicLoader8,
					(LPVOID*)&loader);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;

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


		// デフォルトのDLSをダウンロード
		DMUS_OBJECTDESC objectDesc;
		ZeroMemory(&objectDesc, sizeof(DMUS_OBJECTDESC));
		objectDesc.dwSize = sizeof(DMUS_OBJECTDESC);
		objectDesc.guidClass = CLSID_DirectMusicCollection;
		objectDesc.guidObject = GUID_DefaultGMCollection;
		objectDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT;
		hr = loader->GetObject(&objectDesc, IID_IDirectMusicCollection8,
					(LPVOID*)&defaultDls);
		if(FAILED(hr))	return GgsError::CannotOpenDevice;


		return GgsError::NoError;
	}

	// 音色リストをリセット
	void Port::ResetVoiceList()
	{
		if(downloadedCount == 0) return;

		for(int i=0; i<downloadedCount; i++){
			if(downloadedList[i]){
				port->UnloadInstrument(downloadedList[i]);
				downloadedList[i]->Release();
			}
		}
		delete[] downloadedList;
		downloadedList = 0;
		downloadedCount = 0;
	}

	// 音色を取り出し
	int Port::GetInstrument(int voice, IDirectMusicInstrument8** instrument)
	{
		// まずはデフォルトから探す
		defaultDls->GetInstrument(voice, instrument);
		if(*instrument != 0) return 0;

		// 見つからなければリストから探す
		for(int i=0; i<(int)dlsList.size(); i++){
			if(dlsList[i]->GetInstrument(voice, instrument) == 0) return 0;
		}
		return -1;
	}

	// 音色リストをセット
	void Port::SetVoiceList(DWORD* list, int count)
	{
		ResetVoiceList();

		IDirectMusicInstrument8* instrument = 0 ;
		IDirectMusicDownloadedInstrument8* downloaded = 0;

		downloadedList = new IDirectMusicDownloadedInstrument8*[count];
		downloadedCount = count;

		// 有効ノートは全範囲
		DMUS_NOTERANGE noteRange;
		noteRange.dwLowNote = 0;
		noteRange.dwHighNote = 127;

		// 音色を読み出してportに設定
		for(int i=0; i<count; i++){
			downloadedList[i] = 0;

			int voice = list[i];

			if(GetInstrument(voice, &instrument)){	// 音色がなかったらバンク0
				defaultDls->GetInstrument(voice&0xff0000ff, &instrument);
				if(instrument == 0) continue;	// これはお手上げ
				instrument->SetPatch(voice);
			}

			port->DownloadInstrument(instrument, &downloaded, &noteRange, 1);
			downloadedList[i] = downloaded;
			downloaded = 0;
			instrument->Release();
			instrument = 0;
		}
		port->Compact();
	}



	// 後始末
	void Port::Close()
	{
		ResetVoiceList();

		for(int i=0; i<(int)dlsList.size(); i++){
			delete dlsList[i];
		}
		dlsList.clear();

		if(clock){
			clock->Release();
			clock = 0;
		}

		if(buffer){
			buffer->Release();
			buffer = 0;
		}

		if(defaultDls){
			defaultDls->Release();
			defaultDls = 0;
		}

		if(port){
			port->Activate(FALSE);
			port->Release();
			port = 0;
		}

		if(loader){
			loader->Release();
			loader = 0;
		}

		if(music){
			music->Release();
			music = 0;
		}
			
		if(isOpen){
			CoUninitialize();
			isOpen = false;
		}
	}

	// 遅延付き時間を取得
	GGSERROR Port::GetReferenceTime(long long* time)
	{
		HRESULT hr = clock->GetTime(time);
		if(FAILED(hr)) return GgsError::Failed;
		return GgsError::NoError;
	}

	// 短いメッセージを送る
	GGSERROR Port::SendShortMessage(long long time, int midiMessage)
	{
		if(!isOpen)	return GgsError::NotReady;

		HRESULT hr;
		hr = buffer->PackStructured(time, 0, midiMessage);
		if(FAILED(hr)) return GgsError::Failed;
		hr = port->PlayBuffer(buffer);
		buffer->Flush();
			
		return GgsError::NoError;
	}

	// 長いメッセージを送る
	GGSERROR Port::SendLongMessage(long long time, LPBYTE midiMessage, int length)
	{
		if(!isOpen) return GgsError::NotReady;

		HRESULT hr;
		hr = buffer->PackUnstructured(time, 0, length, midiMessage);
		if(FAILED(hr)) return GgsError::Failed;
		hr = port->PlayBuffer(buffer);
		buffer->Flush();
		if(FAILED(hr)) return GgsError::Failed;

		return GgsError::NoError;
	}

	// DLSを追加
	GGSERROR Port::AddDls(wchar_t* fileName)
	{
		Dls* dls = new Dls(loader);
		GGSERROR err = dls->LoadFromFile(fileName);
		if(err == GgsError::NoError){
			dlsList.push_back(dls);
			return err;
		}else{
			delete dls;
			return err;
		}
	}

}}

