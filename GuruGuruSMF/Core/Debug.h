// デバッグ関係

#pragma once


#if defined(_DEBUG) || defined(DEBUG)
void MyOutputDebugString(LPCWSTR pszFormat, ...);

// Debugのとき
#define TRACE(x)   OutputDebugString(x)
#define TRACE0(x)   OutputDebugString(x)
#define TRACE1(x, a)            MyOutputDebugString(x, a)
#define TRACE2(x, a, b)         MyOutputDebugString(x, a, b)
#define TRACE3(x, a, b, c)      MyOutputDebugString(x, a, b, c)
#define TRACE4(x, a, b, c, d)   MyOutputDebugString(x, a, b, c, d)
#else
// Releaseのとき
#define TRACE(x)
#define TRACE0(x)
#define TRACE1(x, a)
#define TRACE2(x, a, b)
#define TRACE3(x, a, b, c)
#define TRACE4(x, a, b, c, d)
#endif

//#define GGS_LOGGING 1

#ifdef GGS_LOGGING

void GgsLoggingInitialize();
void GgsLoggingPuts(const char* str);
void GgsLoggingShortMsg(__int64 time, int status, int data1, int data2);
void GgsLoggingLongMsg(__int64 time, int length);
__int64 GgsLoggingGetTime();

#define GGS_LOGGING_INITIALIZE() GgsLoggingInitialize()
#define GGS_LOGGING_SHORTMSG(t, s, a, b) GgsLoggingShortMsg(t, s, a, b)
#define GGS_LOGGING_LONGMSG(t, l) GgsLoggingLongMsg(t, l)
#define GGS_LOGGING_GETTIME() GgsLoggingGetTime()

#else

#define GGS_LOGGING_INITIALIZE()
#define GGS_LOGGING_SHORTMSG(t, s, a, b)
#define GGS_LOGGING_LONGMSG(t, l)
#define GGS_LOGGING_GETTIME() 0

#endif
