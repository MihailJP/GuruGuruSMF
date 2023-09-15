
#include "stdafx.h"

#ifdef _DEBUG
void MyOutputDebugString(LPCWSTR pszFormat, ...)
{
	va_list	argp;
	wchar_t pszBuf[ 256];
	va_start(argp, pszFormat);
	vswprintf(pszBuf, pszFormat, argp);
	va_end(argp);
	OutputDebugString( pszBuf);
}
#endif

#ifdef GGS_LOGGING
#include "shlobj.h"

static FILE* debug_fp = 0;
static LPCRITICAL_SECTION debug_cs = 0;

void GgsLoggingInitialize()
{
	if(debug_cs == 0){
		debug_cs = new CRITICAL_SECTION();
		InitializeCriticalSection(debug_cs);
	}
	EnterCriticalSection(debug_cs);
	if(debug_fp != 0){
		fclose(debug_fp);
	}

	// マイドキュメントのパス取得
	char szPath[_MAX_PATH];
	szPath[0] = 0;
	LPITEMIDLIST pidl;

	IMalloc *pMalloc;
	SHGetMalloc( &pMalloc );

	if( SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL,&pidl)) ){
	    SHGetPathFromIDListA(pidl,szPath);
	    pMalloc->Free(pidl);
	}
	pMalloc->Release();

	char logPath[_MAX_PATH];
	sprintf( logPath, "%s\\gurugurusmf.log", szPath);

	debug_fp = fopen(logPath, "w");
	fseek(debug_fp, 0, SEEK_SET);
	LeaveCriticalSection(debug_cs);
}

void GgsLoggingPuts(const char* str)
{
	if(debug_fp == 0) return;
	EnterCriticalSection(debug_cs);
	fputs(str, debug_fp);
	LeaveCriticalSection(debug_cs);
}

void GgsLoggingMessage(__int64 time, const char* str)
{
	char buf[256];
	sprintf(buf, "%I64lld: %s\n", time, str);

	GgsLoggingPuts(buf);
}

void GgsLoggingShortMsg(__int64 time, int status, int data1, int data2)
{
	if((status &0xf0) != 0xb0) return;	// コントロールチェンジ以外はパス
	char buf[256];
	sprintf(buf, "%I64lld: %02x %02x %02x\n", time, status, data1, data2);

	GgsLoggingPuts(buf);
}

void GgsLoggingLongMsg(__int64 time, int length)
{
	char buf[256];
	sprintf(buf, "%I64lld: longmessage: %d\n", time, length);
	GgsLoggingPuts(buf);
}

__int64 GgsLoggingGetTime()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart;
}

#endif


