
#include "stdafx.h"

#include <dxmus.core/Controller.h>
#include "Device.h"


namespace GuruGuruSmf { namespace Core {
namespace WindowsMultimedia {

	// コンストラクタ
	Device::Device()
	{
		isOpen = false;
	}

	// デストラクタ
	Device::~Device()
	{
		Close();
	}

	// デバイスを開く
	GGSERROR Device::Open(int deviceId)
	{
		Close();
		
		if (midiOutOpen(&midiOutHandle, deviceId, 0, 0, 0) != 0)
			return GgsError::CannotOpenDevice;
		isOpen = true;
		return GgsError::NoError;
	}
		
	// デバイスを閉じる
	void Device::Close()
	{
		if (isOpen){
			midiOutClose(midiOutHandle);
			isOpen = false;
		}
	}

	// 長いMIDIメッセージを送信
	GGSERROR Device::SendLongMessage(const byte* data, int length)
	{
		if (!isOpen) return GgsError::NotReady;
		
		MIDIHDR midiHeader;
		int headerSize = sizeof(MIDIHDR);
		ZeroMemory(&midiHeader, headerSize);

		midiHeader.lpData = (LPSTR)data;
		midiHeader.dwBufferLength = length;
		midiHeader.dwFlags = 0;
		midiOutPrepareHeader(midiOutHandle, &midiHeader, headerSize);

		while ((midiHeader.dwFlags & MHDR_PREPARED) != MHDR_PREPARED){
			Sleep(0);
		}

		// MHDR_DONEが立たないことがあるため3回だけ待つ
		if(midiOutLongMsg(midiOutHandle, &midiHeader, headerSize) == MMSYSERR_NOERROR){
			if ((midiHeader.dwFlags & MHDR_DONE) != MHDR_DONE){
				Sleep(1);
				if ((midiHeader.dwFlags & MHDR_DONE) != MHDR_DONE){
					Sleep(5);
					if ((midiHeader.dwFlags & MHDR_DONE) != MHDR_DONE){
						Sleep(5);
					}
				}
			}
		}
		
		midiOutUnprepareHeader(midiOutHandle, &midiHeader, headerSize);
		GGS_LOGGING_LONGMSG(GGS_LOGGING_GETTIME(), length);
		return GgsError::NoError;
	}
		
		
	// 短いMIDIメッセージを送信
	GGSERROR Device::SendShortMessage(int status, int data1, int data2)
	{
		ShortMessage msg;
		msg.Status = (byte)status;
		msg.Data1 = (byte)data1;
		msg.Data2 = (byte)data2;
		msg.Reserve = 0;
		midiOutShortMsg(midiOutHandle, msg.Value);
		GGS_LOGGING_SHORTMSG(GGS_LOGGING_GETTIME(), status, data1, data2);
		return GgsError::NoError;
	}

}


namespace DirectMusicPort {

	// コンストラクタ
	Device::Device(IDirectMusicTimer* timer)
	{
		times = timer;
		isOpen = false;
	}

	// デストラクタ
	Device::~Device()
	{
		Close();
	}

	// デバイスを開く
	GGSERROR Device::Open(int deviceId, HWND handle)
	{
		Close();
		
		GGSERROR err = GuruGuruSmf::Dxmus::Controller::GetInstance()->PortOpen(handle);
		if (err != 0) return err;
		isOpen = true;
		return GgsError::NoError;
	}
		
	// デバイスを閉じる
	void Device::Close()
	{
		if (isOpen){
			GuruGuruSmf::Dxmus::Controller::GetInstance()->PortClose();
			isOpen = false;
		}
	}
	

	// 長いMIDIメッセージを送信
	GGSERROR Device::SendLongMessage(const byte* data, int length)
	{
		TRACE4(L"/SendLongMessage(%d): %x %x %x ", length, data[0], data[1], data[2]);
		TRACE3(L"%x %x %x\n", data[3], data[4], data[5]);

		__int64 t = times->GetMarkerDmTime();
		GGS_LOGGING_LONGMSG(t, length);
		GGSERROR err = GuruGuruSmf::Dxmus::Controller::GetInstance()->PortSendLongMessage(t, (LPBYTE)data, length);
		if(err != GgsError::NoError) return err;
		times->IncrementMarkerDmTime();
		return GgsError::NoError;
	}
	
	
	// 短いMIDIメッセージを送信
	GGSERROR Device::SendShortMessage(int status, int data1, int data2)
	{
		ShortMessage msg;
		msg.Status = (byte)status;
		msg.Data1 = (byte)data1;
		msg.Data2 = (byte)data2;
		msg.Reserve = 0;

		__int64 t = times->GetMarkerDmTime();
		GGS_LOGGING_SHORTMSG(t, status, data1, data2);
		GGSERROR err = GuruGuruSmf::Dxmus::Controller::GetInstance()->PortSendShortMessage(t, msg.Value);
		if(err != GgsError::NoError) return err;
		times->IncrementMarkerDmTime();
		return GgsError::NoError;
	}
	

}

}}
