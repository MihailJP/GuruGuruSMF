// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN	// Exclude rarely-used items from Windows headers.
#endif

// Modify the following defines if you have to target an OS before the ones 
// specified in the following code. See MSDN for the latest information
// about corresponding values for different operating systems.
#ifndef WINVER		// Permit use of features specific to Windows 95 and Windows NT 4.0 or later.
#define WINVER 0x0400	// Change this to the appropriate value to target 
#endif                     // Windows 98 and Windows 2000 or later.

#ifndef _WIN32_WINNT	// Permit use of features specific to Windows NT 4.0 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target 
#endif		         // Windows 98 and Windows 2000 or later.			

#ifndef _WIN32_WINDOWS	      // Permit use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410    // Change this to the appropriate value to target 
#endif			      // Windows Millennium Edition or later.


#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから使用されていない部分を除外します。


#include <windows.h>
#include <dmusici.h>
#include <dmusicc.h>

#include <vector>

