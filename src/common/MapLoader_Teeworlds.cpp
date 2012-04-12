/*
 *  MapLoader_Teeworlds.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.4.12.
 *  code under LGPL
 *
 */

#include "MapLoader.h"
#include "StringUtils.h"
#include "FindFile.h"
#include "EndianSwap.h"
#include "util/Result.h"
#include <zlib.h>
#include <stdint.h>

/* Teeworlds licence.txt:

Copyright (C) 2007-2012 Magnus Auvinen

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

------------------------------------------------------------------------

IMPORTANT NOTE! The source under src/engine/external are stripped
libraries with their own licenses. Mostly BSD or zlib/libpng license but
check the individual libraries.

------------------------------------------------------------------------

With that being said, contact us if there is anything you want to do
that the license does not permit.
*/

/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

// references:
// https://github.com/teeworlds/teeworlds/blob/master/src/engine/shared/datafile.cpp
// https://github.com/teeworlds/teeworlds/blob/master/datasrc/content.py
// https://github.com/erdbeere/tml/blob/master/tml/datafile.py


struct CDatafileItemType
{
	int m_Type;
	int m_Start;
	int m_Num;
} ;

struct CDatafileItem
{
	int m_TypeAndID;
	int m_Size;
};

struct CDatafileHeader
{
	char m_aID[4];
	int32_t m_Version;
	int32_t m_Size;
	int32_t m_Swaplen;
	int32_t m_NumItemTypes;
	int32_t m_NumItems;
	int32_t m_NumRawData;
	int32_t m_ItemSize;
	int32_t m_DataSize;

	Result read(FILE* f) {
		fread(m_aID, 1, sizeof(m_aID), f);
		if(memcmp(m_aID, "DATA", sizeof(m_aID)) != 0 && memcmp(m_aID, "ATAD", sizeof(m_aID)) != 0)
			return "signature wrong";
		fread_endian<int32_t>(f, m_Version);
		if(m_Version != 4)
			return "Teeworlds level version " + itoa(m_Version) + " not supported";
		fread_endian<int32_t>(f, m_Size);
		fread_endian<int32_t>(f, m_Swaplen);
		fread_endian<int32_t>(f, m_NumItemTypes);
		fread_endian<int32_t>(f, m_NumItems);
		fread_endian<int32_t>(f, m_NumRawData);
		fread_endian<int32_t>(f, m_ItemSize);
		fread_endian<int32_t>(f, m_DataSize);
		return true;
	}
};

struct CDatafileData
{
	int m_NumItemTypes;
	int m_NumItems;
	int m_NumRawData;
	int m_ItemSize;
	int m_DataSize;
	char m_aStart[4];
};

struct CDatafileInfo
{
	CDatafileItemType *m_pItemTypes;
	int *m_pItemOffsets;
	int *m_pDataOffsets;
	int *m_pDataSizes;

	char *m_pItemStart;
	char *m_pDataStart;
};

struct CDatafile
{
//	IOHANDLE m_File;
	unsigned m_Crc;
	CDatafileInfo m_Info;
	CDatafileHeader m_Header;
	int m_DataStartOffset;
	char **m_ppDataPtrs;
	char *m_pData;
};


struct ML_Teeworlds : MapLoad {
	CDatafileHeader teeHeader;

	virtual std::string format() { return "Teeworlds"; }
	virtual std::string formatShort() { return "Tee"; }

	virtual bool parseHeader(bool printErrors) {
		head.name = GetBaseFilenameWithoutExt(filename);
		fseek(fp, 0, SEEK_SET);
		if(NegResult r = teeHeader.read(fp)) return r.res;
		return true;
	}

	virtual bool parseData(CMap* m) {

		return false;
	}

};

MapLoad* createMapLoad_Teeworlds() {
	return new ML_Teeworlds();
}
