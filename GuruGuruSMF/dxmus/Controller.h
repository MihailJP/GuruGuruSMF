
#pragma once

#include "PerformancePlayer.h"
#include "Port.h"

namespace GuruGuruSmf{ namespace Dxmus
{

	class Controller
	{
	private:
		static Controller* instance;
		PerformancePlayer* performancePlayer;
		Port* port;
		Controller();
		Controller(const Controller&){}
		~Controller();

	public:
		static Controller* GetInstance();
		static void FreeInstance();
		int PerformanceOpen(HWND handle, int channels)
		{
			return performancePlayer->Open(handle, channels);
		}
		void PerformanceClose()
		{
			performancePlayer->Close();
		}
		int PerformancePlay(PerformancePlayer::SmfData* smf, int option, int seek)
		{
			return  performancePlayer->Play(smf, option, seek);
		}
		void PerformanceStop()
		{
			performancePlayer->Stop();
		}
		void PerformancePause()
		{
			performancePlayer->Pause();
		}
		void PerformanceRestart()
		{
			performancePlayer->Restart();
		}
		void PerformanceSetMasterVolume(long volume)
		{
			performancePlayer->SetMasterVolume(volume);
		}
		void PerformanceSetMasterTempo(float tempo)
		{
			performancePlayer->SetMasterTempo(tempo);
		}
		void PerformanceGetStatus(PlayerStatus* status)
		{
			performancePlayer->GetStatus(status);
		}
		int PortOpen(HWND handle)
		{
			return port->Open(handle);
		}
		void PortClose()
		{
			port->Close();
		}
		int PortGetReferenceTime(long long* time)
		{
			return port->GetReferenceTime(time);
		}
		int PortSendShortMessage(long long time, int midiMessage)
		{
			return port->SendShortMessage(time, midiMessage);
		}
		int PortSendLongMessage(long long time, LPBYTE midiMessage, int length)
		{
			return port->SendLongMessage(time, midiMessage, length);
		}
		void PortSetVoiceList(DWORD* list, int count)
		{
			port->SetVoiceList(list, count);
		}
		int PortAddDls(wchar_t* fileName)
		{
			return port->AddDls(fileName);
		}

	};


}}

