
#include "stdafx.h"

#include "SmfController.h"

namespace GuruGuruSmf { namespace Core {

	// コンストラクタ
	SmfController::SmfController()
	{
		smf = new Smf();
	}

	// デストラクタ
	SmfController::~SmfController()
	{
		delete smf;
	}
	
	// 初期化
	void SmfController::Initialize(Analyzer::Analyzer* analyzerObject, Compiler::Compiler* compilerObject)
	{
		analyzer = analyzerObject;
		compiler = compilerObject;
	}

	// SMFを使用可能状態にする
	GGSERROR SmfController::Open()
	{
		GGSERROR err = smf->Reload();
		if (err != GgsError::NoError) return err;
		compiler->Compile(smf);
		smf->ReleaseSmfBinary();
		return GgsError::NoError;
	}

	// ファイルから読み込み
	GGSERROR SmfController::LoadFromFile(LPCWSTR fileName)
	{
		GGSERROR err = smf->LoadFromFile(fileName);
		if (err != GgsError::NoError)	return err;
		
		err = analyzer->Analyze(smf);
		if (err != GgsError::NoError)	return err;
		
		OnAfterAnalyzed();
		
		smf->ReleaseSmfBinary();
		
		return GgsError::NoError;
	}

	// メモリから読み込み
	GGSERROR SmfController::LoadFromMemory(const byte* smfBinary, int length)
	{
		GGSERROR err = smf->LoadFromMemory(smfBinary, length);
		if (err != GgsError::NoError)	return err;
		
		err = analyzer->Analyze(smf);
		if (err != GgsError::NoError)	return err;
		
		OnAfterAnalyzed();
		
		smf->ReleaseSmfBinary();
		
		return GgsError::NoError;
	}


	//
	// SmfController3Buffered
	//

	// コンストラクタ
	SmfController3Buffered::SmfController3Buffered()
	{
		smfBuffer = 0;
	}

	// デストラクタ
	SmfController3Buffered::~SmfController3Buffered()
	{
		delete[] smfBuffer;
	}

	// SMFを使用可能状態にする
	GGSERROR SmfController3Buffered::Open()
	{
		GGSERROR err = GetSmf()->Reload();
		if (err != GgsError::NoError) return err;
		GetCompiler()->Compile(GetSmf());
		return GgsError::NoError;
	}

	// メモリから読み込み(この時だけメモリに保存)
	GGSERROR SmfController3Buffered::LoadFromMemory(const byte* smfBinary, int length)
	{
		smfBuffer = new byte[length];
		memcpy(smfBuffer, smfBinary, length);
		return SmfController::LoadFromMemory(smfBuffer, length);
	}

}}





