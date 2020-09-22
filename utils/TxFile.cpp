#include "stdafx.h"
#include "TxFile.h"

TxFile::TxFile(const char* file)
{
	m_File = NULL;
	m_strFile = new char[strlen(file)+1];
	strcpy(m_strFile, file);
}

TxFile::~TxFile(void)
{
	delete m_strFile;
	Close();
}

int TxFile::Open()
{
	if (m_File)
	{
		fclose(m_File);
		m_File = NULL;
	}

	m_File = fopen(m_strFile, "rt");
	if (m_File)
	{
		printf("File %s opened\r\n",m_strFile);
		return 0;
	}
	else
	{
		printf("File %s could not be opened\r\n",m_strFile);
		return 1;
	}
}

void TxFile::Close()
{
	if (m_File)
	{
		fclose(m_File);
		m_File = NULL;
	}
}

void TxFile::Reset()
{
	if (m_File) 
		rewind(m_File);
}

int TxFile::NextTextLine(char* buffer)
{
	buffer[0]=0;
	if (!m_File) return -1;

	char* res = fgets(m_fileBuff, MAX_LINE_LEN - 1, m_File);
	m_fileBuff[MAX_LINE_LEN-1] = 0;

	if (res == NULL) return -1;//eof

	int len = strlen(m_fileBuff); 
	if (m_fileBuff[len - 1] == 10) m_fileBuff[len - 1] = 0;
	len = strlen(m_fileBuff);
	if (m_fileBuff[len - 1] == 13) m_fileBuff[len - 1] = 0;
	len = strlen(m_fileBuff);

	if (len>0)
	{
		if (m_fileBuff[0] == '#')
		{
			return 3;
		}
		
		memcpy(buffer,m_fileBuff,len);
		buffer[len]=0;

		return 1;
	}
	//else thearray->setValue(NULL, 0);

	return 2;
}

void TxFile::NextArray(HArray* thearray)
{
	thearray->erase();

	int ret = NextLine(thearray);
	while ((ret != -1) && (ret != 1))
		ret = NextLine(thearray);
}

int TxFile::NextLine(HArray* thearray)
{
	m_chSpecChar = 0;
	m_nSpecValue = 0;
	if (!m_File) return -1;

	char* res = fgets(m_fileBuff, MAX_LINE_LEN - 1, m_File);
	m_fileBuff[MAX_LINE_LEN-1] = 0;

	if (res == NULL) return -1;//eof

	int len = strlen(m_fileBuff); 
	if (m_fileBuff[len - 1] == 10) m_fileBuff[len - 1] = 0;
	len = strlen(m_fileBuff);

	if (len>0)
	{
		if (m_fileBuff[0] == '#')
		{
			//thearray->setValue(NULL, 0);
			return 3;
		}
		else if (m_fileBuff[0] == '$')
		{
			//thearray->setValue(NULL, 0);
			if (len > 1) 
				m_chSpecChar = m_fileBuff[1];
			if (len > 2)
				m_nSpecValue = atoi(m_fileBuff + 2);
			return 4;
		}

		int pos = 0;

		unsigned char pkg[256];
		int pkg_len   =0;
		while (pos<len)
		{
			pkg[pkg_len++] = getmchar(pos);
			pos += 3;
		}
		thearray->setValue(pkg, pkg_len);
		return 1;
	}
	//else thearray->setValue(NULL, 0);

	return 2;
}

unsigned char TxFile::getmchar(int pos)
{
	char str[3];
	strncpy(str, m_fileBuff+pos, 2);
	str[2] = 0;

	return (unsigned char)strtoul(str, NULL, 16);
}