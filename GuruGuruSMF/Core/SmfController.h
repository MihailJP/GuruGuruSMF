// SMF操作クラス

#pragma once

#include "Interface.h"
#include "Analyzer.h"
#include "Compiler.h"

namespace GuruGuruSmf { namespace Core {

	/// <summary>
	/// SMF操作クラス
	/// </summary>
	class SmfController : public ISmfController
	{
	private:
		Smf* smf;
		Analyzer::Analyzer* analyzer;
		Compiler::Compiler* compiler;
		
	public:
		SmfController();			// コンストラクタ
		virtual ~SmfController();	// デストラクタ
		
		//
		// ISmfControllerを実装
		//

		// SMFオブジェクトを取得
		Smf* GetSmf(){ return smf; }
		
		// SMFを使用可能状態にする
		virtual GGSERROR Open();
		
		// 閉じる
		virtual void Close(){ smf->ReleaseCodeBinary(); }
		
		//

		// ファイルから読み込み
		virtual GGSERROR LoadFromFile(LPCWSTR fileName);
		
		// メモリから読み込み
		virtual GGSERROR LoadFromMemory(const byte* smfBinary, int length);
		
		//

		// 初期化
		void Initialize(Analyzer::Analyzer* analyzerObject, Compiler::Compiler* compilerObject);
		
		//
		
	protected:
		// Analyzerオブジェクト
		Analyzer::Analyzer* GetAnalyzer(){ return analyzer; }
		
		// Compilerオブジェクト
		Compiler::Compiler* GetCompiler(){ return compiler; }
		
		// 解析後に呼び出し(通常は何もしない)
		virtual void OnAfterAnalyzed(){}
		
	};

	/// <summary>
	/// SMF操作クラス ver4系統・Buffered用
	/// </summary>
	class SmfController4Buffered : public SmfController
	{
	protected:
		// この時だけコンパイルしてしまう
		void OnAfterAnalyzed(){ GetCompiler()->Compile(GetSmf()); }

	public:

		// SMFを使用可能状態にする(何もしない)
		GGSERROR Open(){ return GgsError::NoError; }
		
		// 閉じる(何もしない)
		void Close(){}

	};
	
	/// <summary>
	/// SMF操作クラス ver4系統・Not Buffered用
	/// </summary>
	class SmfController4NotBuffered : public SmfController
	{
	};
	
	/// <summary>
	/// SMF操作クラス ver3系統・Buffered用
	/// </summary>
	class SmfController3Buffered : public SmfController
	{
	private:
		byte* smfBuffer;

	public:
		SmfController3Buffered();	// コンストラクタ
		~SmfController3Buffered();	// デストラクタ

		// SMFを使用可能状態にする
		GGSERROR Open();

		// メモリから読み込み
		GGSERROR LoadFromMemory(const byte* smfBinary, int length);
	};
	
	/// <summary>
	/// SMF操作クラス ver3系統・Not Buffered用
	/// </summary>
	class SmfController3NotBuffered : public SmfController
	{
	};
	


}}


