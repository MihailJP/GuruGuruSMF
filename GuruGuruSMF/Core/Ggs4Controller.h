// GuruGuruSMF4の窓口

#pragma once

#include <Windows.h>
#include <map>

#include "Constants.h"
#include "Smf.h"
#include "Analyzer.h"
#include "Compiler.h"
#include "WinmmPlayer.h"
#include "DmPortPlayer.h"
#include "DmPerformancePlayer.h"

namespace GuruGuruSmf { namespace Core {

	// SMF解析データ構造体(ver.0.3.3)
	struct SmfInformation3{
		int Size;			//この構造体のサイズ
		int TimeBase;		//ベースカウント
		int TotalTick;		//総Tick
		int TotalTime;		//総演奏時間
		int FirstNoteTick;	//最初の発音tick
		int LastNoteTick;	//最後の発音tick
		int LoopTick;		//CC#111の位置
	};

	// 演奏データ構造体(ver.0.3.3)
	struct PlayerStatus3{
		int Size;		//この構造体のサイズ
		int State;		//演奏中かどうか
		int Tempo;		//現在のテンポ
		int Tick;		//現在のティック
		int Time;		//現在の演奏時間
		int LoopCount;	//現在のループ回数
	};

	// コントローラ基底クラス
	class GgsController
	{
	private:
		std::map<int, SmfController*> smfList;
		SmfListKeys smfListKeys;
		int* listKeys;
		std::string listKeysString;
		Analyzer::Analyzer* analyzer;
		Compiler::Compiler* compiler;
		IPlayer* smfPlayer;
		int playingId;
		int nextId;

	protected:
		GgsController();		// コンストラクタ
		virtual ~GgsController();		// デストラクタ

		// SMFコントローラーを作成(Factory)
		virtual SmfController* MakeSmfController(bool buffered) = 0;

		// プレイヤーオブジェクトを作成(Factory)
		virtual IPlayer* MakePlayer(int deviceId) = 0;

	public:
		// デバイスを開く
		GGSERROR OpenDevice(int deviceId, HWND handle);
		
		// デバイスを閉じる
		void CloseDevice();
		
		// DLSを追加
		GGSERROR AddDlsA(LPCSTR fileName);
		GGSERROR AddDlsW(LPCWSTR fileName);
		
		// ファイルからリストへ追加
		GGSERROR AddListFromFileA(LPCSTR fileName, int option, int musicId);
		GGSERROR AddListFromFileW(LPCWSTR fileName, int option, int musicId);
		GGSERROR AddListFromFile3(LPCSTR fileName, int option, int musicId);
		
		// メモリからリストへ追加
		GGSERROR AddListFromMemory(const byte* smfBinary, int length, int option, int musicId);
		GGSERROR AddListFromMemory3(const byte* smfBinary, int length, int option, int musicId);

		// リスト内のアイテムを1つ削除
		void DeleteListItem(int musicId);
		
		// リストをクリア
		void ClearList();
		
		// リスト内のIDを列挙
		const SmfListKeys* EnumList();

		// リスト内のIDを列挙して文字列化(Legacy)
		const char* EnumListToString();

		// 演奏！
		GGSERROR Play(int option, int musicId, int seek, int fadeInTime, int keyShift);
		
		// 停止！
		void Stop(int fadeOutTime);
		
		// 一時停止
		void Pause();
		
		// 一時停止解除
		void Restart();
		
		// 一時停止/解除(Legacy)
		void PauseRestart();

		// SMFの情報を取得
		GGSERROR GetSmfInformation(SmfInformation* info, int size, int musicId);

		// SMFの情報を取得(Legacy)
		GGSERROR GetSmfInformation3(SmfInformation3* info, int musicId);
		
		// 演奏状態を取得
		void GetPlayerStatus(PlayerStatus* status, int size);

		// 演奏状態を取得(Legacy)
		void GetPlayerStatus3(PlayerStatus3* status);
		
		// マスターテンポ
		void SetMasterTempo(float tempo);
		
		// マスターボリューム
		virtual void SetMasterVolume(int volume);
		
		// マスターピッチ
		void SetMasterPitch(int pitch);

	};



	// コントローラ(GuruGuruSMF4)
	class Ggs4Controller : public GgsController
	{
	private:
		static Ggs4Controller* instance;	// インスタンス置き場
		Ggs4Controller(){}		// コンストラクタ
		~Ggs4Controller(){}		// デストラクタ

	protected:
		// SMFコントローラーを作成(Factory)
		SmfController* MakeSmfController(bool buffered);

		// プレイヤーオブジェクトを作成(Factory)
		IPlayer* MakePlayer(int deviceId);

	public:
		// インスタンスを取得(singleton)
		static Ggs4Controller* GetInstance();

		// インスタンスを破棄
		static void FreeInstance();

		// マスターボリューム
		void SetMasterVolume(int volume);
	};
	
	// コントローラ(GuruGuruSMF3)
	class Ggs3Controller : public GgsController
	{
	private:
		static Ggs3Controller* instance;	// インスタンス置き場
		bool isPerformanceMode;

		Ggs3Controller(){}		// コンストラクタ
		~Ggs3Controller(){}		// デストラクタ

	protected:
		// SMFコントローラーを作成(Factory)
		SmfController* MakeSmfController(bool buffered);

		// プレイヤーオブジェクトを作成(Factory)
		IPlayer* MakePlayer(int deviceId);

	public:
		// インスタンスを取得(singleton)
		static Ggs3Controller* GetInstance();

		// インスタンスを破棄
		static void FreeInstance();

		// マスターボリューム
		void SetMasterVolume(int volume);
	};
		
}}


