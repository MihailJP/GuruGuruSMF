#include "stdafx.h"
#include <dmksctrl.h>
#include "Port.h"


namespace GuruGuruSmf {	namespace Dxmus
{

	// �R���X�g���N�^
	Dls::Dls(IDirectMusicLoader8* ldr)
	{
		loader = ldr;
		collection = 0;
	}

	// �f�X�g���N�^
	Dls::~Dls()
	{
		if (collection){
			collection->Release();
			collection = 0;
		}
	}

	// �t�@�C������ǂݍ���
	int Dls::LoadFromFile(wchar_t *fileName)
	{
		HRESULT hr;
		hr = loader->LoadObjectFromFile(CLSID_DirectMusicCollection, IID_IDirectMusicCollection8,
								fileName, (LPVOID*)&collection );
		if( FAILED(hr) )	return -1;	// �ǂݍ��ݎ��s

		return 0;	// ����
	}

	// ���F���o��
	int Dls::GetInstrument(int voice, IDirectMusicInstrument8** instrument)
	{
		if(collection == 0)	return -1;

		return collection->GetInstrument(voice, instrument);
	}




	// �R���X�g���N�^
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

	// �f�X�g���N�^
	Port::~Port()
	{
		Close();
	}

	// ������
	int Port::Open(HWND handle)
	{
		Close();

		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		isOpen = true;

		HRESULT hr;
		hr = CoCreateInstance(CLSID_DirectMusic ,NULL ,
					CLSCTX_INPROC_SERVER ,IID_IDirectMusic8,
					(LPVOID*)&music);
		hr = music->SetDirectSound(NULL ,handle);

		//�|�[�g�쐬
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
		hr = music->CreatePort(guidPort, &param, &port, NULL);
		port->Activate(TRUE);

		// �o�b�t�@�쐬
		DMUS_BUFFERDESC bufferDesc;

		bufferDesc.dwSize = sizeof(DMUS_BUFFERDESC);
		bufferDesc.dwFlags = 0;
		bufferDesc.guidBufferFormat = GUID_NULL;
		bufferDesc.cbBuffer = 128;
		hr = music->CreateMusicBuffer(&bufferDesc, &buffer, NULL);

		// �^�C�}�擾
		hr = port->GetLatencyClock(&clock);

		// ���[�_�[�쐬
		hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL,
					CLSCTX_INPROC_SERVER, IID_IDirectMusicLoader8,
					(LPVOID*)&loader);

		wchar_t* path = new wchar_t[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, path);
		loader->SetSearchDirectory(GUID_DirectMusicAllTypes, path, FALSE);


		// �f�t�H���g��DLS���_�E�����[�h
		DMUS_OBJECTDESC objectDesc;
		ZeroMemory(&objectDesc, sizeof(DMUS_OBJECTDESC));
		objectDesc.dwSize = sizeof(DMUS_OBJECTDESC);
		objectDesc.guidClass = CLSID_DirectMusicCollection;
		objectDesc.guidObject = GUID_DefaultGMCollection;
		objectDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT;
		hr = loader->GetObject(&objectDesc, IID_IDirectMusicCollection8,
					(LPVOID*)&defaultDls);


		return 0;
	}

	// ���F���X�g�����Z�b�g
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

	// ���F�����o��
	int Port::GetInstrument(int voice, IDirectMusicInstrument8** instrument)
	{
		// �܂��̓f�t�H���g����T��
		defaultDls->GetInstrument(voice, instrument);
		if(*instrument != 0) return 0;

		// ������Ȃ���΃��X�g����T��
		for(int i=0; i<(int)dlsList.size(); i++){
			if(dlsList[i]->GetInstrument(voice, instrument) == 0) return 0;
		}
		return -1;
	}

	// ���F���X�g���Z�b�g
	void Port::SetVoiceList(DWORD* list, int count)
	{
		ResetVoiceList();

		IDirectMusicInstrument8* instrument = 0 ;
		IDirectMusicDownloadedInstrument8* downloaded = 0;

		downloadedList = new IDirectMusicDownloadedInstrument8*[count];
		downloadedCount = count;

		// �L���m�[�g�͑S�͈�
		DMUS_NOTERANGE noteRange;
		noteRange.dwLowNote = 0;
		noteRange.dwHighNote = 127;

		// ���F��ǂݏo����port�ɐݒ�
		for(int i=0; i<count; i++){
			downloadedList[i] = 0;

			int voice = list[i];

			if(GetInstrument(voice, &instrument)){	// ���F���Ȃ�������o���N0
				defaultDls->GetInstrument(voice&0xff0000ff, &instrument);
				if(instrument == 0) continue;	// ����͂���グ
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



	// ��n��
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

	// �|�[�g�̃v���p�e�B�擾
	DWORD Port::GetPortProperty(GUID guid)
	{
		HRESULT   hr;
		IKsControl  *pControl;
		KSPROPERTY  ksp;
		DWORD   dwFlags;
		ULONG   cb = 0;

		hr = port->QueryInterface(IID_IKsControl, (void**)&pControl);
		if (FAILED(hr))
		{
			return 0;
		}
		ksp.Set = guid;
		ksp.Id  = 0;
		ksp.Flags = KSPROPERTY_TYPE_GET;
		pControl->KsProperty(&ksp, sizeof(ksp),
				&dwFlags, sizeof(dwFlags), &cb);
		pControl->Release();
		return dwFlags;
	}

	// �x���t�����Ԃ��擾
	int Port::GetReferenceTime(long long* time)
	{
		return clock->GetTime(time);
	}

	// �Z�����b�Z�[�W�𑗂�
	int Port::SendShortMessage(long long time, int midiMessage)
	{
		if(!isOpen)	return -1;

		HRESULT hr;
		hr = buffer->PackStructured(time, 0, midiMessage);
		if(FAILED(hr)) return hr;
		hr = port->PlayBuffer(buffer);
		buffer->Flush();
			
		return hr;
	}

	// �������b�Z�[�W�𑗂�
	int Port::SendLongMessage(long long time, LPBYTE midiMessage, int length)
	{
		if(!isOpen) return -1;

		HRESULT hr;
		hr = buffer->PackUnstructured(time, 0, length, midiMessage);
		if(FAILED(hr)) return hr;
		hr = port->PlayBuffer(buffer);
		buffer->Flush();
			
		return hr;
	}

	// DLS��ǉ�
	int Port::AddDls(wchar_t* fileName)
	{
		Dls* dls = new Dls(loader);
		if(dls->LoadFromFile(fileName) == 0){
			dlsList.push_back(dls);
			return 0;
		}else{
			delete dls;
			return -1;
		}
	}

}}

