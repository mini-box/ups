#ifndef _UTIL_H_
#define _UTIL_H_
#pragma once

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

#endif //_UTIL_H_