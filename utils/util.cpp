#include "util.h"

unsigned char hex2bcd (unsigned char x)
{
    unsigned char y;
    y = (x / 10) << 4;
    y = y | (x % 10);
    return (y);
}

unsigned char bcd2hex (unsigned char x)
{
    unsigned char y;
	y = (x >> 4)*10;
	y = y + (x & 0x0F);
    return (y);
}

void char2bin(char* destination, unsigned char ch)
{
	int cnt = 0;

	for (int i=7;i>=0;i--)
		if ((ch>>i)&1)
		{
			destination[cnt] = '1';
			cnt++;
		}
		else
		{
			destination[cnt] = '0';
			cnt++;
		}
 
	destination[cnt] = 0;
}

unsigned char bin2char(char* str, bool* ok)
{
	int len = strlen(str);

	*ok = false;
	if (len == 0)
		return 0;

	unsigned char value = 0;

	for (int i=0;i<len;i++)
	{
		if ((str[i] != '0')&&(str[i] != '1')) return 0;

		if (str[i] == '1')
			value = value | (1 << (len-1-i));
	}

	*ok = true;
	return value;
}

void str_mid(char* dest, char* source, int from, int cnt)
{
	strncpy(dest, source+from, cnt);
	dest[cnt] = 0;
}

void str_left(char* dest, char* source, int cnt)
{
	strncpy(dest, source, cnt);
	dest[cnt] = 0;
}

void str_right(char* dest, char* source, int cnt)
{
	strncpy(dest, source+strlen(source)-2, cnt);
	dest[cnt] = 0;
}

#ifdef _DEBUG
void DBGOutput(const char* szFormat, ...)
{
    char szBuff[1024];
    va_list arg;
    va_start(arg, szFormat);
    vsnprintf(szBuff, sizeof(szBuff), szFormat, arg);
    va_end(arg);
	fprintf(stderr, "%s", szBuff);
}
#endif