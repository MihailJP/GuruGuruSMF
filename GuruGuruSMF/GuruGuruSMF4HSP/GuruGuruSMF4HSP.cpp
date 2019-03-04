// GuruGuruSMF4HSP.cpp : DLL �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include <Core/Ggs4Controller.h>
#include "hsp3plugin.h"
#include "hspwnd.h"

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


static Ggs4Controller* ggs;	// �R���g���[���[�C���X�^���X�u����


// DLS��ǉ�
static int GGS4AddDls()
{
	const char* fileName = code_gets();	// �t�@�C����
	return ggs->AddDlsA(fileName);
}

// �}�X�^�[�e���|
static void GGS4SetMasterTempo()
{
	double tempo = code_getd();	// �}�X�^�[�e���|
	ggs->SetMasterTempo((float)tempo);
}

// �t�@�C�����烊�X�g�֒ǉ�
static int GGS4AddListFromFile()
{
	const char* fileName = code_gets();	// �t�@�C����
	p2 = code_geti();	// �I�v�V����
	p3 = code_geti();	// MusicID
	return ggs->AddListFromFileA(fileName, p2 , p3);
}

// ���������烊�X�g�֒ǉ�
static int GGS4AddListFromMemory()
{
	PVal* hspVariant;
	code_getva(&hspVariant);	// SMF���������ϐ�
	p2 = code_geti();	// �T�C�Y
	p3 = code_geti();	// �I�v�V����
	p4 = code_geti();	// MusicID

	HspVarProc* varProc = exinfo->HspFunc_getproc(hspVariant->flag);
	varProc->GetPtr(hspVariant);

	return ggs->AddListFromMemory((const byte*)hspVariant->pt, p2, p3, p4);
}

// ���X�g����ID���
static int GGS4EnumList()
{
	const SmfListKeys* keys = ggs->EnumList();
	int count = keys->Count;

	PVal* hspVariant;
	code_getva(&hspVariant);
	if(count == 0) return count;

	exinfo->HspFunc_dim(hspVariant, HSPVAR_FLAG_INT, count, count, 0, 0, 0);
	HspVarProc* vpInt = exinfo->HspFunc_getproc(HSPVAR_FLAG_INT);

	for(int i=0; i<count; i++){
		hspVariant->offset = i;
		vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &keys->Keys[i]);
	}
	return count;
}

// ���t�����擾
static void GGS4GetPlayerStatus()
{
	PVal* hspVariant;
	APTR a_ptr = code_getva(&hspVariant);

	PlayerStatus status;
	ggs->GetPlayerStatus(&status, sizeof(PlayerStatus));

	exinfo->HspFunc_dim(hspVariant, HSPVAR_FLAG_INT, 9, 9, 0, 0, 0);
	HspVarProc* vpInt = exinfo->HspFunc_getproc(HSPVAR_FLAG_INT);

	hspVariant->offset = 0;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &status.State);
	hspVariant->offset = 1;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &status.Tempo);
	hspVariant->offset = 2;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &status.Tick);
	hspVariant->offset = 3;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &status.Time);
	hspVariant->offset = 4;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &status.LoopCount);
}


// SMF�̏����擾
static int GGS4GetSmfInformation()
{
	PVal* hspVariant;
	code_getva(&hspVariant);
	p2 = code_geti();	// MusicId

	SmfInformation smfInfo;
	int err = ggs->GetSmfInformation(&smfInfo, sizeof(SmfInformation), p2);
	if(err) return err;

	exinfo->HspFunc_dim(hspVariant, HSPVAR_FLAG_INT, 9, 9, 0, 0, 0);
	HspVarProc* vpInt = exinfo->HspFunc_getproc(HSPVAR_FLAG_INT);

	hspVariant->offset = 0;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.TimeBase);
	hspVariant->offset = 1;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.TotalTick);
	hspVariant->offset = 2;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.TotalTime);
	hspVariant->offset = 3;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.FirstNoteTick);
	hspVariant->offset = 4;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.FirstNoteTime);
	hspVariant->offset = 5;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.LastNoteTick);
	hspVariant->offset = 6;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.LastNoteTime);
	hspVariant->offset = 7;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.LoopTick);
	hspVariant->offset = 8;
	vpInt->Set(hspVariant, vpInt->GetPtr(hspVariant), &smfInfo.LoopTime);

	return 0;
}


// ���ߎ��s�֐�
static int cmdfunc(int cmd)
{
	code_next();

	switch(cmd){
	case 0:		// GGS4OpenDevice
		p1 = code_geti();				// �����l���擾(�f�t�H���g�Ȃ�)
		stat = ggs->OpenDevice(p1, ((BMSCR*)getbmscr(0))->hwnd);
		break;

	case 1:		// GGS4CloseDevice
		ggs->CloseDevice();
		break;

	case 3:		// GGS4AddDls
		stat = GGS4AddDls();
		break;

	case 4:		// GGS4Play
		p1 = code_geti();	// �I�v�V����
		p2 = code_geti();	// MusicID
		p3 = code_getdi(0);	// �V�[�N(�ȗ���)
		p4 = code_getdi(0);	// �t�F�[�h�C��(�ȗ���)
		p5 = code_getdi(0);	// �L�[�V�t�g(�ȗ���)
		stat = ggs->Play(p1, p2, p3, p4, p5);
		break;

	case 5:		// GGS4Stop
		p1 = code_getdi(0);	// �t�F�[�h�A�E�g(�ȗ���)
		ggs->Stop(p1);
		break;

	case 6:		// GGS4Pause
		ggs->Pause();
		break;

	case 7:		// GGS4Restart
		ggs->Restart();
		break;

	case 8:		// GGS4SetMasterTempo
		GGS4SetMasterTempo();
		break;

	case 9:		// GGS4SetMasterVolume
		p1 = code_geti();	// �}�X�^�[�{�����[��
		ggs->SetMasterVolume(p1);
		break;

	case 10:	// GGS4SetMasterPitch
		p1 = code_geti();	// �}�X�^�[�s�b�`
		ggs->SetMasterPitch(p1);
		break;

	case 11:	// GGS4GetPlayerStatus
		GGS4GetPlayerStatus();
		break;

	case 13:	// GGS4AddListFromFile
		stat = GGS4AddListFromFile();
		break;

	case 14:	// GGS4AddListFromMemory
		stat = GGS4AddListFromMemory();
		break;

	case 15:	// GGS4DeleteListItem
		p1 = code_geti();	// MusicID
		ggs->DeleteListItem(p1);
		break;

	case 16:	// GGS4ClearList
		ggs->ClearList();
		break;

	case 17:	// GGS4EnumList
		stat = GGS4EnumList();
		break;

	case 18:	// GGS4GetSmfInformation
		stat = GGS4GetSmfInformation();
		break;

	default:
		puterror(HSPERR_UNSUPPORTED_FUNCTION);
	}

	return RUNMODE_RUN;
}

// ��n��
static int termfunc(int option)
{
	Ggs4Controller::FreeInstance();
	return 0;
}

// HSP3�p�G���g���[�|�C���g
void WINAPI GuruGuruSMF4(HSP3TYPEINFO *info)
{
	hsp3sdk_init(info);			// SDK�̏�����(�ŏ��ɍs�Ȃ��ĉ�����)
	info->cmdfunc = cmdfunc;		// ���s�֐�(cmdfunc)�̓o�^
//	info->reffunc = reffunc;		// �Q�Ɗ֐�(reffunc)�̓o�^
	info->termfunc = termfunc;		// �I���֐�(termfunc)�̓o�^

	ggs = Ggs4Controller::GetInstance();
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

