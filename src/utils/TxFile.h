/**
 * M4Atx 
 * Copyright (C) 2009 mini-box.com
 *
 * Licensed under the terms of the GNU Lesser General Public License v2.1:
 * 		http://www.opensource.org/licenses/lgpl-2.1.php
 *
 * For further information visit:
 * 		http://www.mini-box.com/M4-ATX
 *
 * File Authors:
 * 		Barna Csenteri
 */

#pragma once

#include "HArray.h"
#define MAX_LINE_LEN 256*2+256

class TxFile
{
public:
	TxFile(const char* file);
	virtual ~TxFile(void);

	/** Open the file (if already opened the old one is closed and repoened again)*/
	int Open();
	/** Close the file */
	void Close();
	/** Jumping to the start of the file*/
	void Reset();

	const char getSpecChar(){return m_chSpecChar;};
	const int  getSpecVal(){return m_nSpecValue;};
	const char* getFileBuf(){return m_fileBuff;};

	/** Reads only hex arrays, ignores the rest*/
	void NextArray(HArray* thearray);

	/** Get one more line from the file
	 * return: 0 = ok, 1 = eof, 2,3,4... - invalid hex file
	 */
	int NextLine(HArray* thearray);

	/** Get one more line from the file
	 * return: 0 = ok, 1 = eof, 2,3,4... - invalid hex file
	 */
	int NextTextLine(char* buffer);
protected:
	unsigned char getmchar(int pos);

	char* m_strFile;
	FILE* m_File;

	unsigned char m_byteBuff[256];
	char m_fileBuff[MAX_LINE_LEN];
	
	char m_chSpecChar;
	int  m_nSpecValue;
};
