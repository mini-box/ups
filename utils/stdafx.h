// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>
#else
#define TCHAR char
#define _TCHAR char
#define _T(s) s
#define _tcstol strtol
#define _strcmpi strcmp
#define _tcstod strtod
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define _CRTDBG_MAP_ALLOC


unsigned char hex2bcd (unsigned char x);
unsigned char bcd2hex (unsigned char x);
void char2bin(char* destination, unsigned char ch);
unsigned char bin2char(char* str, bool* ok);

void str_mid(char* dest, char* source, int from, int cnt);
void str_left(char* dest, char* source, int cnt);
void str_right(char* dest, char* source, int cnt);

#ifdef _DEBUG
#include <stdarg.h>
void DBGOutput(const char* szFormat, ...);
#else
#define DBGOutput 
#endif

#define TRACE DBGOutput