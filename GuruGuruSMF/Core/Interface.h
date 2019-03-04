// 内部で使用するインターフェイス

#pragma once

#include <Windows.h>
#include "Smf.h"


namespace GuruGuruSmf { namespace Core
{
	/// <summary>
	/// SMFコントローラーのインターフェイス
	/// </summary>
	struct ISmfController
	{
		// SMFオブジェクトを取得
		virtual Smf* GetSmf() = 0;

		// 開く
		virtual GGSERROR Open() = 0;

		// 閉じる
		virtual void Close() = 0;
	};

	/// <summary>
	/// プレイヤーのインターフェイス
	/// </summary>
	struct IPlayer
	{
		// デストラクタ(これを書かなくて罠にハマりました)
		virtual ~IPlayer(){}

		// デバイスを開く
		virtual GGSERROR OpenDevice(int deviceId, HWND handle) = 0;
		
		// デバイスを閉じる
		virtual void CloseDevice() = 0;
		
		// 演奏！
		virtual GGSERROR Play(ISmfController* smfCtrl, int option, int seek, int fadeInTime, int keyShift) = 0;
		
		// 停止！
		virtual void Stop(int fadeOutTime) = 0;
		
		// 一時停止
		virtual void Pause() = 0;
		
		// 一時停止解除
		virtual void Restart() = 0;
		
		// 演奏状態を取得
		virtual void GetStatus(PlayerStatus* status) = 0;
		
		// マスターテンポ
		virtual void SetMasterTempo(float tempo) = 0;
		
		// マスターボリューム
		virtual void SetMasterVolume(int volume) = 0;
		
		// マスターピッチ
		virtual void SetMasterPitch(int pitch) = 0;
	};
	
	/// <summary>
	/// デバイスのインターフェイス
	/// </summary>
	struct IDevice
	{
		// 長いMIDIメッセージを送信
		virtual void SendLongMessage(const byte* data, int length) = 0;
		
		// 短いMIDIメッセージを送信
		virtual void SendShortMessage(int status, int data1, int data2) = 0;
	};
	
	/// <summary>
	/// Player内部アクセス用のインターフェイス
	/// </summary>
	struct IPlayerCore
	{
		// MIDIメッセージ送信デバイスを取得
		virtual IDevice* GetDevice() = 0;
		
		// マーカーTickに加算処理
		virtual void AddMarkerTick(int deltaTime) = 0;
		
		// 終点処理
		virtual void MusicEnd() = 0;
		
		// テンポをセット
		virtual void SetTempo(int newTempo) = 0;
		
		// ボリュームメッセージを送信
		virtual void SendVolume(int channel, int volume) = 0;
		
		// ピッチベンドメッセージを送信
		virtual void SendPitchBend(int channel, int pitch) = 0;
		
		// エクスクルーシブを送信
		virtual void SendExclusive(CodeEvent* codeEvent) = 0;
		
		// Seek用にイベントを登録
		virtual void RegisterEvent(CODE_EVENTTYPE type, int channel, int data1, CodeEvent* codeEvent) = 0;

		// シフトされたノート番号を取得
		virtual int GetShiftedNote(int note, int channel) = 0;
	};
	
	/// <summary>
	/// Analyzerの内部アクセス用のインターフェイス
	/// </summary>
	struct IAnalyzerCore
	{
		virtual void SetTempo(int newTempo) = 0;
		virtual void NoteEvent(bool isNoteOn) = 0;
		virtual void ProgramChangeEvent(int channel, int voice) = 0;
		virtual void BankSelectEvent(int channel, bool isMsb, int bank) = 0;
		virtual void UseRhythmPartEvent(int channel, bool isRhythmPart) = 0;
		virtual void VolumeEvent(int volume) = 0;
		virtual void MarkLoopPoint() = 0;
	};
	
	/// <summary>
	/// Compilerの内部アクセス用のインターフェイス
	/// </summary>
	struct ICompilerCore
	{
		// Codeオブジェクト
		virtual GgsCode* GetCode() = 0;
		
		// 2バイトのデータを持ったイベントを書き込む
		virtual void Write2ByteEvent(CODE_EVENTTYPE type, SmfEvent* smfEvent) = 0;
		
		// エクスクルーシブイベントを書き込む
		virtual void WriteExclusiveEvent(SmfEvent* smfEvent) = 0;
		
		// リズムパートイベントを書き込む
		virtual void WriteUseRhythmPartEvent(SmfEvent* smfEvent, int channel, bool rhythm) = 0;
	};
	
	
	
	/// <summary>
	/// DirectMusicのタイマー機能
	/// </summary>
	struct IDirectMusicTimer
	{
		virtual __int64 GetMarkerDmTime() = 0;
		virtual void IncrementMarkerDmTime() = 0;
	};
	
}}

