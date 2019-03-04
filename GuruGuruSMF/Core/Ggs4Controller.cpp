
#include "stdafx.h"

#include "Ggs4Controller.h"

namespace GuruGuruSmf { namespace Core {

	//
	// GgsController
	//

	// コンストラクタ
	GgsController::GgsController()
	{
		playingId = 0;
		nextId = 1;
		listKeys = 0;

		smfPlayer = 0;
		analyzer = new Analyzer::Analyzer();
		compiler = new Compiler::Compiler();
	}

	// デストラクタ
	GgsController::~GgsController()
	{
		CloseDevice();

		ClearList();
		delete[] listKeys;

		delete smfPlayer;

		delete analyzer;
		delete compiler;
		GuruGuruSmf::Dxmus::Controller::FreeInstance();
	}

	// デバイスを開く
	GGSERROR GgsController::OpenDevice(int deviceId, HWND handle)
	{
		CloseDevice();

		smfPlayer = MakePlayer(deviceId);
		
		GGSERROR err = smfPlayer->OpenDevice(deviceId, handle);
		if(err != GgsError::NoError){
			delete smfPlayer;
			smfPlayer = 0;
			return err;
		}

		return GgsError::NoError;
	}
		
	// デバイスを閉じる
	void GgsController::CloseDevice()
	{
		TRACE(L"●Ggs4Controller::CloseDevice\n");
		Stop(0);
		if(smfPlayer){
			TRACE(L": smfPlayer->CloseDevice()\n");
			smfPlayer->CloseDevice();

			delete smfPlayer;
			smfPlayer = 0;

			TRACE(L": CloseDevice: done\n");
			smfPlayer = 0;
		}
	}


	// リストをクリア
	void GgsController::ClearList()
	{
		Stop(0);

		std::map<int, SmfController*>::iterator it = smfList.begin();
		
		while(it != smfList.end()){
			delete (*it).second;
			++it;
		}
		smfList.clear();
	}

	// リスト内のアイテムを1つ削除
	void GgsController::DeleteListItem(int musicId)
	{
		// 演奏中のSMFを削除しようとしたら演奏停止
		if(playingId == musicId) Stop(0);

		std::map<int, SmfController*>::iterator it = smfList.find(musicId);
		if(it != smfList.end()){
			delete (*it).second;
			smfList.erase(musicId);
		}
	}

	// リスト内のIDを列挙
	const SmfListKeys* GgsController::EnumList()
	{
		int size = smfList.size();
		smfListKeys.Count = size;

		delete[] listKeys;
		listKeys = 0;
		if(size == 0){
			smfListKeys.Keys = 0;
			return &smfListKeys;
		}

		listKeys = new int[size];
		int i = 0;
		std::map<int, SmfController*>::iterator it = smfList.begin();

		while(it != smfList.end()){
			listKeys[i++] = (*it).first;
			++it;
		}
		smfListKeys.Keys = listKeys;

		return &smfListKeys;
	}

	// リスト内のIDを列挙して文字列化(Legacy)
	const char* GgsController::EnumListToString()
	{
		char buf[16];
		EnumList();
		listKeysString.clear();

		int count = smfListKeys.Count;
		if(count){
			sprintf_s(buf, 16, "%d", listKeys[0]);
			listKeysString += buf;
			for(int i=1; i<count; i++){
				sprintf_s(buf, 16, ",%d", listKeys[i]);
				listKeysString += buf;
			}
		}

		return listKeysString.c_str();
	}

	// ファイルからリストへ追加(ver.0.3.3)
	GGSERROR GgsController::AddListFromFile3(LPCSTR fileName, int option, int musicId)
	{
		if(musicId == 0){
			musicId = nextId++;
		}
		return AddListFromFileA(fileName, option, musicId);
	}

	// ファイルからリストへ追加(ANSI)
	GGSERROR GgsController::AddListFromFileA(LPCSTR fileName, int option, int musicId)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0);
		wchar_t* stringBuff = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, fileName, -1, stringBuff, len);

		GGSERROR err = AddListFromFileW(stringBuff, option, musicId);
		delete[] stringBuff;

		return err;
	}

	// ファイルからリストへ追加(UNICODE)
	GGSERROR GgsController::AddListFromFileW(LPCWSTR fileName, int option, int musicId)
	{
		DeleteListItem(musicId);
		
		SmfController* smfController = MakeSmfController((option & LoadOption::Buffered) != 0);
		smfController->Initialize(analyzer, compiler);

		GGSERROR err = smfController->LoadFromFile(fileName);
		if(err != GgsError::NoError) return err;
		smfList[musicId] = smfController;
		
		return GgsError::NoError;
	}

	// メモリからリストへ追加(ver.0.3.3)
	GGSERROR GgsController::AddListFromMemory3(const byte* smfBinary, int length, int option, int musicId)
	{
		if(musicId == 0){
			musicId = nextId++;
		}
		return AddListFromMemory(smfBinary, length, option, musicId);
	}

	// メモリからリストへ追加
	GGSERROR GgsController::AddListFromMemory(const byte* smfBinary, int length, int option, int musicId)
	{
		
		// 演奏中のSMFを上書きしようとしたら演奏停止
		if(playingId == musicId) Stop(0);
		
		DeleteListItem(musicId);
		
		SmfController* smfController = MakeSmfController((option & LoadOption::Buffered) != 0);
		smfController->Initialize(analyzer, compiler);

		GGSERROR err = smfController->LoadFromMemory(smfBinary, length);
		if(err != GgsError::NoError) return err;
		smfList[musicId] = smfController;

		return GgsError::NoError;
	}

	// 停止！
	void GgsController::Stop(int fadeOutTime)
	{
		if(fadeOutTime < 100){
			fadeOutTime = 0;
		}
		
		if(smfPlayer){
			smfPlayer->Stop(fadeOutTime);
		}
	}

	// 演奏！
	GGSERROR GgsController::Play(int option, int musicId, int seek, int fadeInTime, int keyShift)
	{
		if(smfPlayer == 0) return GgsError::NotReady;
		
		Stop(0);
		if(smfList.find(musicId) == smfList.end()){
			return GgsError::UndefinedId;
		}
		
		GGSERROR err;
		SmfController* smfController = smfList[musicId];
		
		if(fadeInTime < 100){
			fadeInTime = 0;
		}
		
		err = smfPlayer->Play(smfController, option, seek, fadeInTime, keyShift);
		if(err != GgsError::NoError)	return err;
		playingId = musicId;
		
		return GgsError::NoError;
	}

	// DLSを追加(ANSI)
	GGSERROR GgsController::AddDlsA(LPCSTR fileName)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0);
		wchar_t* stringBuff = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, fileName, -1, stringBuff, len);

		GGSERROR err = AddDlsW(stringBuff);
		delete[] stringBuff;

		return err;
	}

	// DLSを追加(UNICODE)
	GGSERROR GgsController::AddDlsW(LPCWSTR fileName)
	{
		GGSERROR err = GuruGuruSmf::Dxmus::Controller::GetInstance()->PortAddDls((wchar_t*)fileName);
		if(err) return err;
		return GgsError::NoError;
	}

	// 一時停止
	void GgsController::Pause()
	{
		if(smfPlayer){
			smfPlayer->Pause();
		}
	}
	
	// 一時停止解除
	void GgsController::Restart()
	{
		if(smfPlayer){
			smfPlayer->Restart();
		}
	}

	// 一時停止/解除(Legacy)
	void GgsController::PauseRestart()
	{
		if(smfPlayer){
			PlayerStatus status;
			smfPlayer->GetStatus(&status);
			switch(status.State){
			case PlayerState::Play:
				Pause();
				break;
			case PlayerState::Pause:
				Restart();
				break;
			}
		}
	}

		
	// SMFの情報を取得
	GGSERROR GgsController::GetSmfInformation(SmfInformation* info, int size, int musicId)
	{
		if(smfList.find(musicId) == smfList.end()) return GgsError::UndefinedId;
		
		Smf* smf = smfList[musicId]->GetSmf();
		if(!smf->GetIsReady()) return GgsError::NotReady;
		if(size < 4) return GgsError::NoError;
		info->TimeBase = smf->GetTimeBase();
		if(size < 8) return GgsError::NoError;
		info->TotalTick = smf->GetData()->TotalTick;
		if(size < 12) return GgsError::NoError;
		info->TotalTime = (int)smf->GetData()->TotalTime;
		if(size < 16) return GgsError::NoError;
		info->FirstNoteTick = smf->GetData()->FirstNoteTick;
		if(size < 20) return GgsError::NoError;
		info->FirstNoteTime = (int)smf->GetData()->FirstNoteTime;
		if(size < 24) return GgsError::NoError;
		info->LastNoteTick = smf->GetData()->LastNoteTick;
		if(size < 28) return GgsError::NoError;
		info->LastNoteTime = (int)smf->GetData()->LastNoteTime;
		if(size < 32) return GgsError::NoError;
		info->LoopTick = smf->GetData()->LoopTick;
		if(size < 36) return GgsError::NoError;
		info->LoopTime = (int)smf->GetData()->LoopTime;
		return GgsError::NoError;
	}

	// SMFの情報を取得(Legacy)
	GGSERROR GgsController::GetSmfInformation3(SmfInformation3* info, int musicId)
	{
		int size = info->Size;
		if(smfList.find(musicId) == smfList.end()) return GgsError::UndefinedId;
		
		Smf* smf = smfList[musicId]->GetSmf();
		if(!smf->GetIsReady()) return GgsError::NotReady;
		if(size < 8) return GgsError::NoError;
		info->TimeBase = smf->GetTimeBase();
		if(size < 12) return GgsError::NoError;
		info->TotalTick = smf->GetData()->TotalTick;
		if(size < 16) return GgsError::NoError;
		info->TotalTime = (int)smf->GetData()->TotalTime;
		if(size < 20) return GgsError::NoError;
		info->FirstNoteTick = smf->GetData()->FirstNoteTick;
		if(size < 24) return GgsError::NoError;
		info->LastNoteTick = smf->GetData()->LastNoteTick;
		if(size < 28) return GgsError::NoError;
		int loop = smf->GetData()->LoopTick;
		if(loop == -1) loop = 0;
		info->LoopTick = loop;

		return GgsError::NoError;
	}


	// 演奏状態を取得
	void GgsController::GetPlayerStatus(PlayerStatus* status, int size)
	{
		PlayerStatus innerStatus;
		ZeroMemory(&innerStatus, sizeof(PlayerStatus));
		if(smfPlayer){
			smfPlayer->GetStatus(&innerStatus);
		}else{
			innerStatus.State = PlayerState::Stop;
		}

		if(size < 4) return;
		status->State = innerStatus.State;
		if(size < 8) return;
		status->Tempo = innerStatus.Tempo;
		if(size < 12) return;
		status->Tick = innerStatus.Tick;
		if(size < 16) return;
		status->Time = innerStatus.Time;
		if(size < 20) return;
		status->LoopCount = innerStatus.LoopCount;
	}

	// 演奏状態を取得(Legacy)
	void GgsController::GetPlayerStatus3(PlayerStatus3* status)
	{
		PlayerStatus innerStatus;
		ZeroMemory(&innerStatus, sizeof(PlayerStatus));
		if (smfPlayer){
			smfPlayer->GetStatus(&innerStatus);
		}else{
			innerStatus.State = PlayerState::Stop;
		}

		int size = status->Size;

		if(size < 8) return;
		status->State = innerStatus.State;
		if(size < 12) return;
		status->Tempo = innerStatus.Tempo;
		if(size < 16) return;
		status->Tick = innerStatus.Tick;
		if(size < 20) return;
		status->Time = innerStatus.Time;
		if(size < 24) return;
		status->LoopCount = innerStatus.LoopCount;
	}

	// マスターテンポ
	void GgsController::SetMasterTempo(float tempo)
	{
		if(smfPlayer){
			if(tempo < 0.1){
				tempo = (float)0.1;
			}else if(tempo > 8){
				tempo = 8;
			}
			smfPlayer->SetMasterTempo(tempo);
		}
	}
	
	// マスターボリューム
	void GgsController::SetMasterVolume(int volume)
	{
		if(smfPlayer){
			smfPlayer->SetMasterVolume(volume);
		}
	}
	
	// マスターピッチ
	void GgsController::SetMasterPitch(int pitch)
	{
		if(smfPlayer){
			if(pitch < -8192){
				pitch = -8192;
			}else if(pitch > 8191){
				pitch = 8191;
			}
			smfPlayer->SetMasterPitch(pitch);
		}
	}


	//
	// Ggs4Controller
	//

	Ggs4Controller* Ggs4Controller::instance = 0;	// インスタンス置き場


	// インスタンスを取得
	Ggs4Controller* Ggs4Controller::GetInstance()
	{
		if(instance == 0){
			instance = new Ggs4Controller();
		}
		return instance;
	}

	// インスタンスを破棄
	void Ggs4Controller::FreeInstance()
	{
		delete instance;
		instance = 0;
	}

	// プレイヤーオブジェクトを作成(Factory)
	IPlayer* Ggs4Controller::MakePlayer(int deviceId)
	{
		IPlayer* playerObject;
		switch(deviceId){
		case Device::DirectMusic:
		case Device::DirectMusicPerformance:
			playerObject = new DirectMusicPort::Player();
			break;
		default:
			playerObject = new WindowsMultimedia::Player();
			break;
		}
		return playerObject;
	}

	// SMFコントローラーを作成(Factory)
	SmfController* Ggs4Controller::MakeSmfController(bool buffered)
	{
		SmfController* smfController;
		if(buffered){
			smfController = new SmfController4Buffered();
		}else{
			smfController = new SmfController4NotBuffered();
		}
		return smfController;
	}
	
	// マスターボリューム
	void Ggs4Controller::SetMasterVolume(int volume)
	{
		if(volume > 0) volume = 0;
		if(volume < -127) volume = -127;

		GgsController::SetMasterVolume(volume);

	}

	//
	// Ggs3Controller
	//

	Ggs3Controller* Ggs3Controller::instance = 0;	// インスタンス置き場


	// インスタンスを取得
	Ggs3Controller* Ggs3Controller::GetInstance()
	{
		if(instance == 0){
			instance = new Ggs3Controller();
		}
		return instance;
	}

	// インスタンスを破棄
	void Ggs3Controller::FreeInstance()
	{
		delete instance;
		instance = 0;
	}

	// プレイヤーオブジェクトを作成(Factory)
	IPlayer* Ggs3Controller::MakePlayer(int deviceId)
	{
		isPerformanceMode = false;
		IPlayer* playerObject;
		switch(deviceId){
		case Device::DirectMusic:
			playerObject = new DirectMusicPort::Player();
			break;
		case Device::DirectMusicPerformance:
			playerObject = new DirectMusicPerformance::Player();
			isPerformanceMode = true;
			break;
		default:
			playerObject = new WindowsMultimedia::Player();
			break;
		}
		return playerObject;
	}

	// SMFコントローラーを作成(Factory)
	SmfController* Ggs3Controller::MakeSmfController(bool buffered)
	{
		SmfController* smfController;
		if(buffered){
			smfController = new SmfController3Buffered();
		}else{
			smfController = new SmfController3NotBuffered();
		}
		return smfController;
	}

	// マスターボリューム
	void Ggs3Controller::SetMasterVolume(int volume)
	{
		if(!isPerformanceMode){
			if(volume > 0) volume = 0;
			if(volume < -127) volume = -127;
		}

		GgsController::SetMasterVolume(volume);

	}



}}

