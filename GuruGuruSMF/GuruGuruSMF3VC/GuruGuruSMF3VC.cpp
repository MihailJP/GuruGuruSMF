// GuruGuruSMF3VC.cpp : DLL �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include <Core/Ggs4Controller.h>

#define DLLEXPORT __declspec(dllexport)

#ifdef _DEBUG
#pragma comment(lib, "Core_d.lib")
#pragma comment(lib, "dxmus.core_d.lib")
#else
#pragma comment(lib, "Core.lib")
#pragma comment(lib, "dxmus.core.lib")
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")



#ifdef _MANAGED
#pragma managed(push, off)
#endif

using namespace GuruGuruSmf;
using namespace GuruGuruSmf::Core;

extern "C" {

	// �f�o�C�X�ݒ�
	DLLEXPORT GGSERROR WINAPI GGSSetDevice(int deviceId, HWND handle)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->OpenDevice(deviceId, handle);
	}

	// ���X�g0�ԂɃt�@�C�������[�h
	DLLEXPORT GGSERROR WINAPI GGSLoadFile(LPCSTR fileName,int option)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromFileA(fileName, option, 0);
	}

	// ���X�g0�ԂɃ���������ǂݍ���
	DLLEXPORT GGSERROR WINAPI GGSReadMemory(const byte* smfBinary, int length, int option)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromMemory(smfBinary, length, option, 0);
	}

	// �t�@�C�����烊�X�g�ɒǉ�
	DLLEXPORT GGSERROR WINAPI GGSAddList(LPCSTR fileName, int option, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromFile3(fileName, option, musicId);
	}

	// ���������烊�X�g�֒ǉ�
	DLLEXPORT GGSERROR WINAPI GGSReadList(const byte* smfBinary, int length, int option, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->AddListFromMemory3(smfBinary, length, option, musicId);
	}

	// ���X�g���̃A�C�e����1�N���A
	DLLEXPORT void WINAPI GGSDeleteList(int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->DeleteListItem(musicId);
	}

	// ���X�g���N���A
	DLLEXPORT void WINAPI GGSFlushList(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->ClearList();
	}

	// ���X�g����ID���
	DLLEXPORT const char* WINAPI GGSViewList(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->EnumListToString();
	}

	// ���Z�b�g�𑗐M
	DLLEXPORT void WINAPI GGSSendReset(int reset)
	{
	}

	// ���t�I
	DLLEXPORT GGSERROR WINAPI GGSPlay(int option, int musicId, int seek)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->Play(option, musicId, seek, 0, 0);
	}

	// ��~
	DLLEXPORT void WINAPI GGSStop(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->Stop(0);
	}

	// �ꎞ��~
	DLLEXPORT void WINAPI GGSPause(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->PauseRestart();
	}

	// �}�X�^�[�e���|
	DLLEXPORT void WINAPI GGSTempoDepth(int tempo)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		float t;
		if(tempo < 0){
			t = (float)(1 + (double)tempo / 200);
		}else{
			t = (float)(1 + (double)tempo / 50);
		}
		ggs->SetMasterTempo(t);
	}

	// �}�X�^�[�s�b�`
	DLLEXPORT void WINAPI GGSPitchDepth(int pitch)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->SetMasterPitch(pitch);
	}

	// �}�X�^�[�{�����[��
	DLLEXPORT void WINAPI GGSMasterVolume(int volume)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->SetMasterVolume(volume);
	}

	// SMF�̃f�[�^���擾
	DLLEXPORT GGSERROR WINAPI GGSGetSMFDatas(SmfInformation3* info, int musicId)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		return ggs->GetSmfInformation3(info, musicId);
	}

	// ���t�󋵎擾
	DLLEXPORT void WINAPI GGSGetStatus(PlayerStatus3* status)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->GetPlayerStatus3(status);
	}

	// DLS�ǂݍ���
	DLLEXPORT void WINAPI GGSAddDLS(LPCSTR fileName)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->AddDlsA(fileName);
	}

	// �ݒ�_�C�A���O
	DLLEXPORT void WINAPI GGSConfigDlg(HWND handle)
	{
	}
	
	// ��n��(�f�o�C�X�����)
	DLLEXPORT void WINAPI GGSFree(void)
	{
		Ggs3Controller* ggs = Ggs3Controller::GetInstance();
		ggs->CloseDevice();
	}

}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Ggs3Controller::Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Ggs3Controller::FreeInstance();
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

