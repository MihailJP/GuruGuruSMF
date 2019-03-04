// dxmus.cpp : DLL �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "Controller.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

using namespace GuruGuruSmf::Dxmus;

// �p�t�H�[�}���X�v���C���[���g�p�\�ɂ���
extern "C" int WINAPI PerformanceOpen(HWND handle, int channels)
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformanceOpen(handle, channels);
}

// �p�t�H�[�}���X�v���C���[�����
extern "C" void WINAPI PerformanceClose()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceClose();
}

// �p�t�H�[�}���X�v���C���[�ŉ��t
extern "C" int WINAPI PerformancePlay(PerformancePlayer::SmfData* smf, int option, int seek)
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformancePlay(smf, option, seek);
}

// �p�t�H�[�}���X�v���C���[�̉��t���~
extern "C" void WINAPI PerformanceStop()
{
	Controller* cc = Controller::GetInstance();
	return cc->PerformanceStop();
}

// �p�t�H�[�}���X�v���C���[�ꎞ��~
extern "C" void WINAPI PerformancePause()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformancePause();
}

// �p�t�H�[�}���X�v���C���[�ꎞ��~����
extern "C" void WINAPI PerformanceRestart()
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceRestart();
}

// �p�t�H�[�}���X�v���C���[�E�}�X�^�[�{�����[��
extern "C" void WINAPI PerformanceSetMasterVolume(long volume)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceSetMasterVolume(volume);
}

// �p�t�H�[�}���X�v���C���[�E�}�X�^�[�e���|
extern "C" void WINAPI PerformanceSetMasterTempo(float tempo)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceSetMasterTempo(tempo);
}

// �p�t�H�[�}���X�v���C���[�̏�Ԏ擾
extern "C" void WINAPI PerformanceGetStatus(GuruGuruSmf::PlayerStatus* status)
{
	Controller* cc = Controller::GetInstance();
	cc->PerformanceGetStatus(status);
}

// �|�[�g���[�h���g�p�\�ɂ���
extern "C" int WINAPI PortOpen(HWND handle)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortOpen(handle);
}

// �|�[�g���[�h�����
extern "C" void WINAPI PortClose()
{
	Controller* cc = Controller::GetInstance();
	cc->PortClose();
}

// ���t�@�����X�^�C�����擾
extern "C" int WINAPI PortGetReferenceTime(long long* time)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortGetReferenceTime(time);
}

// �Z��MIDI���b�Z�[�W�𑗂�
extern "C" int WINAPI PortSendShortMessage(long long time, int midiMessage)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortSendShortMessage(time, midiMessage);
}

// ����MIDI���b�Z�[�W�𑗂�
extern "C" int WINAPI PortSendLongMessage(long long time, LPBYTE midiMessage, int length)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortSendLongMessage(time, midiMessage, length);
}

// �g�p���鉹�F�����X�g�Ŏw��
extern "C" void WINAPI PortSetVoiceList(DWORD* list, int count)
{
	Controller* cc = Controller::GetInstance();
	cc->PortSetVoiceList(list, count);
}

// DLS��ǉ�
extern "C" int WINAPI PortAddDls(wchar_t* fileName)
{
	Controller* cc = Controller::GetInstance();
	return cc->PortAddDls(fileName);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		Controller::FreeInstance();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

