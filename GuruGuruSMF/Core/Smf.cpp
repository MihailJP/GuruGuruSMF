
#include "stdafx.h"

#include "Smf.h"


namespace GuruGuruSmf { namespace Core {

	//
	// VoiceList
	//

	// コンストラクタ
	VoiceList::VoiceList()
	{
		count = 0;
		items = 0;
	}

	// デストラクタ
	VoiceList::~VoiceList()
	{
		delete[] items;
	}

	// set配列からコピー
	void VoiceList::CopyFrom(std::set<uint>* src)
	{
		delete[] items;
		items = 0;
		count = src->size();
		TRACE1(L"VoiceList::CopyFrom(%d)\n: ", count);
		if(count == 0) return;	// ありえないですが一応
		
		items = new uint[count];
		int i=0;
		std::set<uint>::iterator it = src->begin();
		while(it != src->end())
		{
			items[i] = *it;
			TRACE1(L"%x ", items[i]);
			++i;
			++it;
		}
		TRACE(L"\n");
	}


	//
	// ExclusiveData
	//

	// コンストラクタ
	ExclusiveData::ExclusiveData(byte* data, int len)
	{
		length = len + 1;
		if(len < 0){
			binary = 0;
			return;
		}
		TRACE1(L"- ExclusiveData(%d): ", len+1);
		binary = new byte[len+1];
		binary[0] = 0xf0;
		TRACE(L"0xf0 ");
		memcpy(binary+1, data, len);
		TRACE4(L"%x %x %x %x\n", binary[1], binary[2], binary[3], binary[4]);
	}

	// デストラクタ
	ExclusiveData::~ExclusiveData()
	{
		delete[] binary;
	}



	//
	// GgsCode
	//

	// コンストラクタ
	GgsCode::GgsCode()
	{
		binary = 0;
	}

	// デストラクタ
	GgsCode::~GgsCode()
	{
		Clear();
	}

	// 長さを指定してメモリを確保する
	void GgsCode::SetLength(int newLength)
	{
		delete[] binary;

		if(newLength < 0){
			binary = 0;
		}else{
			// 今のところ8バイト固定なのでハードコーディング
			binary = new int[newLength * 2];
		}
		length = newLength;
		position = 0;
	}

	// 現在の位置にイベント書き込み
	void GgsCode::WriteEvent(CodeEvent* ev)
	{
		int pos = position * 2;
		binary[pos++] = ev->Header;
		binary[pos] = ev->IntData;
		++position;
	}

	// 現在位置からイベント読み込み
	void GgsCode::ReadEvent(CodeEvent* ev)
	{
		int pos = position * 2;
		ev->Header = binary[pos++];
		ev->IntData = binary[pos];
		++position;
	}

	// エクスクルーシブデータを登録しそのIDを取得
	int GgsCode::ResisterExclusive(ExclusiveData* exclusive)
	{
		exclusiveList.push_back(exclusive);
		return exclusiveList.size() - 1;
	}
		
	// エクスクルーシブデータを取得
	ExclusiveData* GgsCode::GetExclusive(int index)
	{
		return exclusiveList[index];
	}

	// 長さを0にしてエクスクルーシブデータを開放
	void GgsCode::Clear()
	{
		for(int i=0; i<(int)exclusiveList.size(); i++){
			delete exclusiveList[i];
		}
		exclusiveList.clear();
		SetLength(0);
	}



	//
	// SmfStream
	//

	// コンストラクタ
	SmfStream::SmfStream()
	{
		binary = 0;
		position = 0;
		length = 0;
	}

	// デストラクタ
	SmfStream::~SmfStream()
	{
		delete[] binary;
	}

	// ファイルから読み込み
	GGSERROR SmfStream::LoadFromFile(LPCWSTR fileName)
	{
		TRACE(L"●SmfStream::LoadFromFile\n");

		HANDLE handle = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		DWORD errCode;
		if(handle == INVALID_HANDLE_VALUE){	// 普通に失敗したケース
			errCode = GetLastError();
			if(errCode == 2)	return GgsError::FileNotFound;
 			return GgsError::CannotOpenFile;
		}else if(handle == NULL){	// Windows9x?
			char fileNameA[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, fileName, -1,
				fileNameA, MAX_PATH, NULL, NULL);
			handle = CreateFileA(fileNameA, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(handle == INVALID_HANDLE_VALUE){
				errCode = GetLastError();
				if(errCode == 2)	return GgsError::FileNotFound;
	 			return GgsError::CannotOpenFile;
			}
		}

		length = GetFileSize(handle, NULL);
		if(length < 1){
			CloseHandle(handle);
			return GgsError::BadFileSize;
		}

		binary = new byte[length];
		baseAddress = binary;

		unsigned long xlen;
		ReadFile(handle, binary, length, &xlen, NULL);
		CloseHandle(handle);

		TRACE2(L"  Length: %d/%d\n", xlen, length);

		position = 0;
		return GgsError::NoError;
	}

	// メモリから読み込み
	GGSERROR SmfStream::LoadFromMemory(const byte* smfBinary, int newLength)
	{
		delete[] binary;
		binary = 0;

		baseAddress = smfBinary;
		length = newLength;
		position = 0;

		return GgsError::NoError;
	}


	// 破棄
	void SmfStream::Unload()
	{
		if(binary){
			delete[] binary;
			binary = 0;
			length = 0;	// メモリーから読んだ時にこれが保存されたままなのがミソ
		}
		position = 0;
	}

	// シーク
	GGSERROR SmfStream::Seek(int pos)
	{
		if(pos < 0 || length <= pos) return GgsError::IllegalPointer;
		position = pos;
		return GgsError::NoError;
	}

	// 現在位置から2バイトの数値を取得
	GGSERROR SmfStream::ReadUShort(int& intData)
	{
		if(position+2 >= length) return GgsError::IllegalPointer;	// 範囲外
		
		Int32BE data;
		data.B[3] = 0;
		data.B[2] = 0;
		data.B[1] = baseAddress[position++];
		data.B[0] = baseAddress[position++];
		intData = data.Value;
		
		return GgsError::NoError;
	}

	// 現在位置から4バイトの数値を取得
	GGSERROR SmfStream::ReadInt(int& intData)
	{
		if(position+4 >= length) return GgsError::IllegalPointer;	// 範囲外
		
		Int32BE data;
		data.B[3] = baseAddress[position++];
		data.B[2] = baseAddress[position++];
		data.B[1] = baseAddress[position++];
		data.B[0] = baseAddress[position++];
		intData = data.Value;
		
		return GgsError::NoError;
	}

	// 現在位置からデルタタイムを取得
	GGSERROR SmfStream::ReadDeltaTime(int& deltaTime, int endPosition)
	{
		return ReadVInt(deltaTime, endPosition);
	}

	// 現在位置からSMFイベントを1つ＋デルタタイムを取得
	GGSERROR SmfStream::ReadSmfEvent(SmfEvent* smfEvent)
	{
		int endPos = smfEvent->EndPosition;
		if(position >= endPos) return GgsError::TrackLimit;
		
		int status = baseAddress[position];
		if(status < 0x80){	// ランニングステータス
			status = smfEvent->MidiStatus;
		}else{
			++position;
		}
		
		GGSERROR err = GgsError::NoError;
		if (status < 0xf0){	// MIDIイベント
			smfEvent->MidiStatus = status;
			switch(status & 0xf0){
			case 0x90:	// ノートオン
				err = ReadEvent2Data(smfEvent, SmfEventType::NoteOn);
				break;
			case 0x80:	// ノートオフ
				err = ReadEvent2Data(smfEvent, SmfEventType::NoteOff);
				break;
			case 0xe0:	// ピッチベンド
				err = ReadPitchBend(smfEvent);
				break;
			case 0xa0: // ポリフォニックキープレッシャー
				err = ReadEvent2Data(smfEvent, SmfEventType::PAf);
				break;
			case 0xb0: // コントロールチェンジ
				err = ReadEvent2Data(smfEvent, SmfEventType::ControlChange);
				break;
			case 0xc0: // プログラムチェンジ
				err = ReadEvent1Data(smfEvent, SmfEventType::ProgramChange);
				break;
			case 0xd0: // チェンネルプレッシャー
				err = ReadEvent1Data(smfEvent, SmfEventType::CAf);
				break;
			}
		}else{
			switch(status){
			case 0xf0: case 0xf7:	// エクスクルーシブ
				err = ReadExclusive(smfEvent, status);
				break;
			case 0xff:	// メタイベント
				err = ReadMetaEvent(smfEvent);
				break;
			default:
				return GgsError::BadSmf;
			}
		}
		if (err != GgsError::NoError) return err;

		// デルタタイム取得
		int data;
		err = ReadVInt(data, endPos);
		smfEvent->DeltaTime = data;
		return err;
	}



	// 可変長の数値を読み込む
	GGSERROR SmfStream::ReadVInt(int& intData, int endPosition)
	{
		intData = 0;

		for(int i=0; i<4; i++)
		{
			intData <<= 7;
			if(position >= endPosition) return GgsError::TrackLimit;
			int a = baseAddress[position++];
			intData += a & 0x7f;
			if(a < 0x80)	break;
		}
		return GgsError::NoError;
	}

	// 2バイトデータのイベントを取得
	GGSERROR SmfStream::ReadEvent2Data(SmfEvent* smfEvent, SMF_EVENTTYPE type)
	{
		if(position+1 >= smfEvent->EndPosition) return GgsError::TrackLimit;
		
		smfEvent->Data1 = baseAddress[position++];
		smfEvent->Data2 = baseAddress[position++];
		smfEvent->Type = type;
		
		return GgsError::NoError;
	}

	// 1バイトデータのイベントを取得
	GGSERROR SmfStream::ReadEvent1Data(SmfEvent* smfEvent, SMF_EVENTTYPE type)
	{
		if(position >= smfEvent->EndPosition) return GgsError::TrackLimit;
		
		smfEvent->Data1 = baseAddress[position++];
		smfEvent->Type = type;
		
		return GgsError::NoError;
	}

	// ピッチベンドイベントを取得
	GGSERROR SmfStream::ReadPitchBend(SmfEvent* smfEvent)
	{
		if(position+1 >= smfEvent->EndPosition) return GgsError::TrackLimit;
		
		int data = baseAddress[position++];
		data += baseAddress[position++] << 7;
		smfEvent->Data1 = data - 8192;
		
		smfEvent->Type = SmfEventType::PitchBend;
		return GgsError::NoError;
	}

	// エクスクルーシブイベントを取得(0xf0) 0xf7から始まるものは無視
	GGSERROR SmfStream::ReadExclusive(SmfEvent* smfEvent, int status)
	{
		int endPos = smfEvent->EndPosition;
		int len;
		GGSERROR err = ReadVInt(len, endPos);
		if(err != GgsError::NoError) return err;
		
		int nextPos = position + len;
		if(nextPos >= endPos) return GgsError::TrackLimit;
		
		// 長さ99まで有効
		if(len < 100 && status == 0xf0){
			TRACE1(L"- Exculusive 0xf0: %d\n", len);
			smfEvent->ExclusiveLength = len;
			smfEvent->ExclusiveData = (byte*)&baseAddress[position];
		}else{
			smfEvent->ExclusiveLength = -1;	// 長すぎる場合は-1
		}
		
		position = nextPos;
		smfEvent->Type = SmfEventType::Exclusive;
		
		return GgsError::NoError;
	}

	// メタイベントを取得
	GGSERROR SmfStream::ReadMetaEvent(SmfEvent* smfEvent)
	{
		int endPos = smfEvent->EndPosition;
		if(position >= endPos) return GgsError::TrackLimit;
		int type = baseAddress[position++];
		
		if(type == 0x2f){	// End of Track
			smfEvent->Type = SmfEventType::TrackEnd;
			return GgsError::NoError;	// これだけは無条件で返す
		}
		
		int len;
		GGSERROR err = ReadVInt(len, endPos);
		if(err != GgsError::NoError) return err;
		
		smfEvent->Type = SmfEventType::MetaEvent;
		int nextPos = position + len;
		if(nextPos >= endPos) return GgsError::TrackLimit;
		
		switch(type)
		{
		case 0x51: // Tempo: 対応は現在これのみ
			if(len == 3){	// データ長が3バイトでないものはスルー
				Int32BE data;
				data.B[3] = 0;
				data.B[2] = baseAddress[position];
				data.B[1] = baseAddress[position+1];
				data.B[0] = baseAddress[position+2];
				
				smfEvent->Type = SmfEventType::Tempo;
				smfEvent->Data1 = data.Value;
			}
			break;
		}
		position = nextPos;
		return GgsError::NoError;
	}



	//
	// SmfTrack
	//

	// 初期化
	void SmfTrack::Initialize(InitParam* initParam)
	{
		smfStream = initParam->SmfStream;
		beginPosition = initParam->BeginPosition;
		length = initParam->Length;
		
		smfEvent.EndPosition = beginPosition + length;
		Rewind();
	}

	// 最初に巻き戻し
	void SmfTrack::Rewind()
	{
		position = beginPosition;
		smfEvent.MidiStatus = 0;
	}

	// 現在位置からデルタタイムを取得
	int SmfTrack::ReadDeltaTime(GGSERROR& error)
	{
		int dt;
		smfStream->Seek(position);
		error = smfStream->ReadDeltaTime(dt, smfEvent.EndPosition);
		position = smfStream->GetPosition();
		return dt;
	}

	// 現在位置からSMFイベントを1つ＋デルタタイムを取得
	SmfEvent* SmfTrack::ReadSmfEvent(GGSERROR& error)
	{
		smfStream->Seek(position);
		error = smfStream->ReadSmfEvent(&smfEvent);
		position = smfStream->GetPosition();
		return &smfEvent;
	}


	//
	// Smf
	//

	// コンストラクタ
	Smf::Smf()
	{
		code = new GgsCode();
		smfStream = new SmfStream();
		tracks = 0;
		voiceList = 0;
		trackCount = 0;
		voiceList = new VoiceList();

		data.VoiceList = voiceList;

		isReady = false;
		buffered = false;
	}

	// デストラクタ
	Smf::~Smf()
	{
		ClearData();

		delete smfStream;
		delete code;
		delete voiceList;

		for (int i=0; i<trackCount; i++){
			delete tracks[i];
		}
		delete[] tracks;
	}

	// ファイルから読み込み
	GGSERROR Smf::LoadFromFile(LPCWSTR fileName)
	{
		TRACE(L"●Smf::LoadFromFile: ");
		TRACE(fileName);
		TRACE(L"\n");
		isReady = false;
		ClearData();
		GGSERROR err = smfStream->LoadFromFile(fileName);
		if (err != GgsError::NoError) return err;
		
		err = ReadHeader();
		TRACE(L"\n");
		if (err != GgsError::NoError) return err;
		
		smfFileName = fileName;
		storedType = StoredType::File;

		isReady = true;
		return GgsError::NoError;
	}

	// メモリーから読み込み
	GGSERROR Smf::LoadFromMemory(const byte* smfBinary, int length)
	{
		isReady = false;
		ClearData();
		GGSERROR err = smfStream->LoadFromMemory(smfBinary, length);
		if (err != GgsError::NoError) return err;
		
		err = ReadHeader();
		if (err != GgsError::NoError) return err;
		
		storedType = StoredType::Memory;

		isReady = true;
		return GgsError::NoError;
	}

	// 再読み込み
	GGSERROR Smf::Reload()
	{
		GGSERROR err = GgsError::NoError;
		if(storedType == StoredType::File){
			err = smfStream->LoadFromFile(smfFileName.c_str());
		}
		return err;
	}

	// 最初へ巻き戻し
	void Smf::Rewind()
	{
		for (int i=0; i<trackCount; i++){
			tracks[i]->Rewind();
		}
	}


	// 内部データをクリア
	void Smf::ClearData()
	{
		for(int i=0; i<trackCount; i++){
			delete tracks[i];
		}
		delete[] tracks;
		tracks = 0;
		trackCount = 0;
	}

	// ヘッダー読み込み
	GGSERROR Smf::ReadHeader()
	{
		TRACE(L"●Smf::ReadHeader");
		smfStream->Seek(0);
		
		int intData;
		if (smfStream->ReadInt(intData) != GgsError::NoError)	return GgsError::NoSmf;
		TRACE1(L":%x", intData);
		if (intData != 0x4d546864) return GgsError::NoSmf;	//'MThd'かチェック
		
		if (smfStream->ReadInt(intData) != GgsError::NoError)	return GgsError::BadSmf;
		TRACE1(L":%d", intData);
		if (intData != 6) return GgsError::UnmatchedSmf;	//ヘッダ長が6バイトでない場合は扱えないです
		
		if (smfStream->ReadUShort(intData) != GgsError::NoError)	return GgsError::BadSmf;
		TRACE1(L":%d", intData);
		if (intData >= 2) return GgsError::UnmatchedSmf;	//フォーマット0か1のみ対応です
			
		if (smfStream->ReadUShort(intData) != GgsError::NoError)	return GgsError::BadSmf;
		TRACE1(L":%d", intData);
		if (intData < 1) return GgsError::BadSmf;
		trackCount = intData;
		
		if (smfStream->ReadUShort(intData) != GgsError::NoError)	return GgsError::BadSmf;
		TRACE1(L":%d", intData);
		if ((intData & 0x8000) != 0) return GgsError::UnmatchedSmf;	//絶対時間系データは未対応
		timeBase = intData;
		
		tracks = new SmfTrack*[trackCount];
		for (int i=0; i<trackCount; i++){
			tracks[i] = 0;	// エラーが出た時のdeleteミス対策
		}

		SmfTrack::InitParam initParam;
		initParam.SmfStream = smfStream;
		

		for (int i=0; i<trackCount; i++){
			SmfTrack* track = new SmfTrack();
			tracks[i] = track;
			if (smfStream->ReadInt(intData) != GgsError::NoError)	return GgsError::BadSmf;
			if (intData != 0x4d54726b) return GgsError::BadSmf;	//'MTrk'かチェック
			
			if (smfStream->ReadInt(intData) != GgsError::NoError)	return GgsError::BadSmf;
			
			int pos = smfStream->GetPosition();
			int nextPos = pos + intData;
			initParam.BeginPosition = pos;
			initParam.Length = intData;
			track->Initialize(&initParam);
		
			if (i == (trackCount-1)){
				// 最後のトラック
				if (nextPos > smfStream->GetLength()) return GgsError::BadSmf;
			}else{
				smfStream->Seek(nextPos);
			}
		}
			
		return GgsError::NoError;
	}


}}


