// MIDIデバイス

#pragma once

#include <MMSystem.h>
#include "Constants.h"
#include "Interface.h"

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// デバイスの基底クラス(?)
	/// </summary>
	class DeviceBase
	{
	protected:
		// 短いMIDIメッセージの整形用
		union ShortMessage
		{
			int Value;
			struct{
				byte Status;
				byte Data1;
				byte Data2;
				byte Reserve;
			};
		};
	};
	
namespace WindowsMultimedia {

	/// <summary>
	/// WindowsMultimediaのデバイスを扱う
	/// </summary>
	class Device : public DeviceBase, public IDevice
	{
	private:
		bool isOpen;
		HMIDIOUT midiOutHandle;

	public:
		Device();	// コンストラクタ
		~Device();	// デストラクタ

		// デバイスが開かれているかどうか
		bool GetIsOpen(){ return isOpen; }

		// デバイスを開く
		GGSERROR Open(int deviceId);
		
		// デバイスを閉じる
		void Close();
		
		//
		// IDeveiceを実装
		//
		
		// 長いMIDIメッセージを送信
		GGSERROR SendLongMessage(const byte* data, int length);
		
		// 短いMIDIメッセージを送信
		GGSERROR SendShortMessage(int status, int data1, int data2);
	};


}

namespace DirectMusicPort {

	/// <summary>
	/// DirectMusic Portのデバイスを扱う
	/// </summary>
	class Device : public DeviceBase, public IDevice
	{
	private:
		bool isOpen;
		IDirectMusicTimer* times;

	public:
		Device(IDirectMusicTimer* timer);	// コンストラクタ
		~Device();	// デストラクタ

		// DirectMusic管理スレッド
		void OnWorker();

		// デバイスが開いているかどうか
		bool GetIsOpen(){ return isOpen; }
		
		// デバイスを開く
		GGSERROR Open(int deviceId, HWND handle);
		
		// デバイスを閉じる
		void Close();
		
		//
		// IDeveiceを実装
		//
		
		// 長いMIDIメッセージを送信
		GGSERROR SendLongMessage(const byte* data, int length);
		
		// 短いMIDIメッセージを送信
		GGSERROR SendShortMessage(int status, int data1, int data2);
		
	};


}

}}



