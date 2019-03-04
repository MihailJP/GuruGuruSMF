
#pragma once
#include <dmusici.h>
#include <dmusicc.h>

#include <vector>

namespace GuruGuruSmf {	namespace Dxmus
{

	// DLS1つ分を扱うクラス
	class Dls
	{
	private:
		IDirectMusicLoader8* loader;
		IDirectMusicCollection8* collection;

	public:
		Dls(IDirectMusicLoader8* ldr);
		~Dls();

		int LoadFromFile(wchar_t* fileName);
		int GetInstrument(int voice, IDirectMusicInstrument8** instrument);
	};



	// DirectMusicPortを扱うクラス
	class Port
	{
	private:
		IDirectMusic8* music;
		IDirectMusicPort8* port;
		IDirectMusicBuffer8* buffer;
		IDirectMusicLoader8 *loader;
		IDirectMusicCollection8	*defaultDls;
		IReferenceClock *clock;

		std::vector<Dls*> dlsList;
		bool isOpen;
		IDirectMusicDownloadedInstrument8** downloadedList;
		int downloadedCount;

		void ResetVoiceList();

		DWORD GetPortProperty(GUID guid);


	public:
		Port();
		~Port();
		int Open(HWND handle);
		void Close();
		int GetReferenceTime(long long* time);
		int SendShortMessage(long long time, int midiMessage);
		int SendLongMessage(long long time, LPBYTE midiMessage, int length);
		void SetVoiceList(DWORD* list, int count);
		int AddDls(wchar_t* fileName);
		int GetInstrument(int voice, IDirectMusicInstrument8** instrument);

	};



}}

