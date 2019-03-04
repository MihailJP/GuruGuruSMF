
#pragma once
#include <dmusici.h>
#include <dmusicc.h>

#include <vector>

#include <Core/Constants.h>

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

		GGSERROR LoadFromFile(wchar_t* fileName);
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

	public:
		Port();
		~Port();
		GGSERROR Open(HWND handle);
		void Close();
		GGSERROR GetReferenceTime(long long* time);
		GGSERROR SendShortMessage(long long time, int midiMessage);
		GGSERROR SendLongMessage(long long time, LPBYTE midiMessage, int length);
		void SetVoiceList(DWORD* list, int count);
		GGSERROR AddDls(wchar_t* fileName);
		int GetInstrument(int voice, IDirectMusicInstrument8** instrument);

	};



}}

