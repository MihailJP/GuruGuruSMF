// GuruGuruSMF4HSP.cpp : DLL アプリケーションのエントリ ポイントを定義します。
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


static Ggs4Controller* ggs;	// コントローラーインスタンス置き場


// DLSを追加
static int GGS4AddDls()
{
	const char* fileName = code_gets();	// ファイル名
	return ggs->AddDlsA(fileName);
}

// マスターテンポ
static void GGS4SetMasterTempo()
{
	double tempo = code_getd();	// マスターテンポ
	ggs->SetMasterTempo((float)tempo);
}

// ファイルからリストへ追加
static int GGS4AddListFromFile()
{
	const char* fileName = code_gets();	// ファイル名
	p2 = code_geti();	// オプション
	p3 = code_geti();	// MusicID
	return ggs->AddListFromFileA(fileName, p2 , p3);
}

// メモリからリストへ追加
static int GGS4AddListFromMemory()
{
	PVal* hspVariant;
	code_getva(&hspVariant);	// SMFが入った変数
	p2 = code_geti();	// サイズ
	p3 = code_geti();	// オプション
	p4 = code_geti();	// MusicID

	HspVarProc* varProc = exinfo->HspFunc_getproc(hspVariant->flag);
	varProc->GetPtr(hspVariant);

	return ggs->AddListFromMemory((const byte*)hspVariant->pt, p2, p3, p4);
}

// リスト中のIDを列挙
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

// 演奏情報を取得
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


// SMFの情報を取得
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


// 命令実行関数
static int cmdfunc(int cmd)
{
	code_next();

	switch(cmd){
	case 0:		// GGS4OpenDevice
		p1 = code_geti();				// 整数値を取得(デフォルトなし)
		stat = ggs->OpenDevice(p1, ((BMSCR*)getbmscr(0))->hwnd);
		break;

	case 1:		// GGS4CloseDevice
		ggs->CloseDevice();
		break;

	case 3:		// GGS4AddDls
		stat = GGS4AddDls();
		break;

	case 4:		// GGS4Play
		p1 = code_geti();	// オプション
		p2 = code_geti();	// MusicID
		p3 = code_getdi(0);	// シーク(省略可)
		p4 = code_getdi(0);	// フェードイン(省略可)
		p5 = code_getdi(0);	// キーシフト(省略可)
		stat = ggs->Play(p1, p2, p3, p4, p5);
		break;

	case 5:		// GGS4Stop
		p1 = code_getdi(0);	// フェードアウト(省略可)
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
		p1 = code_geti();	// マスターボリューム
		ggs->SetMasterVolume(p1);
		break;

	case 10:	// GGS4SetMasterPitch
		p1 = code_geti();	// マスターピッチ
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

// 後始末
static int termfunc(int option)
{
	Ggs4Controller::FreeInstance();
	return 0;
}

// HSP3用エントリーポイント
void WINAPI GuruGuruSMF4(HSP3TYPEINFO *info)
{
	hsp3sdk_init(info);			// SDKの初期化(最初に行なって下さい)
	info->cmdfunc = cmdfunc;		// 実行関数(cmdfunc)の登録
//	info->reffunc = reffunc;		// 参照関数(reffunc)の登録
	info->termfunc = termfunc;		// 終了関数(termfunc)の登録

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

