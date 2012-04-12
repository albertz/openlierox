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
#include "util/macros.h"
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

/* from TML:
	:copyright: 2010-2011 by the TML Team, see AUTHORS for more details.
	:license: GNU GPL, see LICENSE for more details.
  */

// references:
// http://sushitee.github.com/tml/mapformat.html
// https://github.com/erdbeere/tml/blob/master/tml/datafile.py
// https://github.com/teeworlds/teeworlds/blob/master/src/engine/shared/datafile.cpp
// https://github.com/teeworlds/teeworlds/blob/master/datasrc/content.py

enum ItemType {
	ITEM_VERSION, ITEM_INFO, ITEM_IMAGE, ITEM_ENVELOPE, ITEM_GROUP, ITEM_LAYER,
	ITEM_ENVPOINT
};

enum LayerType {
	LAYERTYPE_INVALID, LAYERTYPE_GAME, LAYERTYPE_TILES, LAYERTYPE_QUADS
};

struct CDatafileItemType
{
	int32_t m_Type;
	int32_t m_Start;
	int32_t m_Num;

	void read(FILE* f) {
		fread_endian<int32_t>(f, m_Type);
		fread_endian<int32_t>(f, m_Start);
		fread_endian<int32_t>(f, m_Num);
	}
};

struct CDatafileItemOffset {
	int32_t m_Offset;
	void read(FILE* f) { fread_endian<int32_t>(f, m_Offset); }
};

typedef CDatafileItemOffset CDatafileDataOffset;

struct CDatafileItem
{
	int32_t m_TypeAndID;
	int32_t m_Size;

	void read(FILE* f) {
		fread_endian<int32_t>(f, m_TypeAndID);
		fread_endian<int32_t>(f, m_Size);
	}
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

struct TWMapInfo {
	std::string author;
	std::string map_version;
	std::string credits;
	std::string license;
	std::string settings;
};

struct ML_Teeworlds : MapLoad {
	CDatafileHeader teeHeader;
	std::vector<CDatafileItemType> itemTypes;
	std::vector<CDatafileItemOffset> itemOffsets;
	std::vector<CDatafileDataOffset> dataOffsets;
	TWMapInfo info;
	typedef std::string Raw;

	virtual std::string format() { return "Teeworlds"; }
	virtual std::string formatShort() { return "Tee"; }

	virtual Result parseHeader(bool printErrors) {
		head.name = GetBaseFilenameWithoutExt(filename);
		fseek(fp, 0, SEEK_SET);
		if(NegResult r = teeHeader.read(fp)) return r.res;
		return true;
	}

	size_t baseOffset() {
		return
				36 + // CDatafileHeader
				teeHeader.m_NumItemTypes * 12 + // CDatafileItemType
				teeHeader.m_NumItems * 4 + // CDatafileItemOffset
				teeHeader.m_NumRawData * 4 + // CDatafileDataOffset
				teeHeader.m_NumRawData * 4; // uncompressed sizes
	}

	CDatafileItemType* getItemType(ItemType type) {
		foreach(t, itemTypes) {
			if(t->m_Type == type)
				return &*t;
		}
		return NULL;
	}

	size_t getItemSize(uint32_t itemIndex) {
		if(itemIndex >= itemOffsets.size()) return 0;
		if(itemIndex == itemOffsets.size() - 1)
			return teeHeader.m_ItemSize - itemOffsets[itemIndex].m_Offset - 8; // -8 to cut out type_and_id and size
		return itemOffsets[itemIndex+1].m_Offset - itemOffsets[itemIndex].m_Offset - 8;
	}

	Raw getItem(uint32_t itemIndex) {
		if(itemIndex >= itemOffsets.size()) return Raw();
		size_t off = baseOffset() + itemOffsets[itemIndex].m_Offset + 8; // +8 to cut out type_and_id and size
		size_t size = getItemSize(itemIndex);
		Raw data;
		data.resize(size);
		fseek(fp, off, SEEK_SET);
		fread(&data[0], 1, size, fp);
		return data;
	}

	Raw getItem(ItemType itemType, uint32_t relIndex) {
		CDatafileItemType* t = getItemType(itemType);
		if(!t) return Raw();
		if((int)relIndex >= t->m_Num) return Raw();
		return getItem(t->m_Start + relIndex);
	}

	size_t getDataSize(uint32_t index) {
		if(index >= dataOffsets.size()) return 0;
		if(index == dataOffsets.size() - 1)
			return teeHeader.m_DataSize - dataOffsets[index].m_Offset;
		return dataOffsets[index + 1].m_Offset - dataOffsets[index].m_Offset;
	}

	Raw getData(uint32_t index) {
		if(index >= dataOffsets.size()) return Raw();
		size_t off = baseOffset() + teeHeader.m_ItemSize + dataOffsets[index].m_Offset;
		size_t size = getDataSize(index);
		Raw data;
		data.resize(size);
		fseek(fp, off, SEEK_SET);
		fread(&data[0], 1, size, fp);
		return data;
	}

	Result parseVersion() {
		Raw data = getItem(ITEM_VERSION, 0);
		if(data.size() < 4)
			return "version item not found / invalid";
		uint32_t version = pread_endian<uint32_t>(&data[0]);
		if(version != 1)
			return "wrong version";
		return true;
	}

	Result parseInfo() {
		Raw data = getItem(ITEM_INFO, 0);
		// TODO...
		return true; // not important
	}

	virtual Result parseData(CMap* m) {
		if(NegResult r = parseHeader(false)) return r.res; // just do it again to seek to right pos

		if(teeHeader.m_NumItemTypes < 0)
			return "invalid numItemTypes";
		itemTypes.resize(teeHeader.m_NumItemTypes);
		for(int i = 0; i < teeHeader.m_NumItemTypes; ++i)
			itemTypes[i].read(fp);

		if(teeHeader.m_NumItems < 0)
			return "invalid numItems";
		itemOffsets.resize(teeHeader.m_NumItems);
		for(int i = 0; i < teeHeader.m_NumItems; ++i)
			itemOffsets[i].read(fp);

		if(teeHeader.m_NumRawData < 0)
			return "invalid numRawData";
		dataOffsets.resize(teeHeader.m_NumRawData);
		for(int i = 0; i < teeHeader.m_NumRawData; ++i)
			dataOffsets[i].read(fp);

		if(NegResult r = parseVersion()) return r.res;

		// ...

		return false;
	}

};

MapLoad* createMapLoad_Teeworlds() {
	return new ML_Teeworlds();
}
