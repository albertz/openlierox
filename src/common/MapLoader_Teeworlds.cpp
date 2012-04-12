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
#include <stdio.h>
#include <errno.h>

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
// https://github.com/teeworlds/teeworlds/blob/master/src/game/mapitems.h
// https://github.com/teeworlds/teeworlds/blob/master/src/game/editor/io.cpp
// https://github.com/teeworlds/teeworlds/blob/master/datasrc/content.py

struct ML_Teeworlds;

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

struct TWImage {
	int32_t version;
	int32_t width;
	int32_t height;
	bool external;
	int32_t name_idx;
	int32_t data_idx;
	std::string name;
	std::string data;

	Result read(ML_Teeworlds* l, char* p, char* end);
};

struct TWTileLayer {
	int32_t version;
	int32_t width;
	int32_t height;
	int32_t game_type;
	int32_t color[4];
	int32_t color_env;
	int32_t color_env_offset;
	int32_t image_id;
	int32_t data_idx;
	std::string name;
	Result read(ML_Teeworlds* l, char* p, char* end);
};

struct TWQuadLayer {
	int32_t version;
	int32_t num_quads;
	int32_t data_idx;
	int32_t image_id;
	std::string name;
	Result read(ML_Teeworlds* l, char* p, char* end);
};

struct TWLayer {
	int32_t layer_version;
	LayerType type;
	int32_t flags;
	bool detail;
	TWTileLayer tileLayer;
	TWQuadLayer quadLayer;

	Result read(ML_Teeworlds* l, char* p, char* end);
};

struct TWGroup {
	int32_t version;
	int32_t offset_x;
	int32_t offset_y;
	int32_t parallax_x;
	int32_t parallax_y;
	int32_t start_layer;
	int32_t num_layers;
	bool use_clipping;
	int32_t clip_x;
	int32_t clip_y;
	int32_t clip_w;
	int32_t clip_h;
	std::string name;
	std::vector<TWLayer> layers;

	Result read(ML_Teeworlds* l, char* p, char* end);
	Result readLayers(ML_Teeworlds* l);
};

typedef std::string Raw; // or std::vector. doesn't really matter. std::string is usually copy-on-write which is preferable for us

static std::string rawReadStr(char* start, char* end) {
	size_t s = 0;
	while(start + s < end && start[s] != 0)
		++s;
	return std::string(start, s);
}

struct ML_Teeworlds : MapLoad {
	CDatafileHeader teeHeader;
	std::vector<CDatafileItemType> itemTypes;
	std::vector<CDatafileItemOffset> itemOffsets;
	std::vector<CDatafileDataOffset> dataOffsets;
	TWMapInfo info;
	std::vector<TWImage> images;
	std::vector<TWGroup> groups;

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

	Result getItem(uint32_t itemIndex, Raw& data) {
		if(itemIndex >= itemOffsets.size()) return "getItem: index " + itoa(itemIndex) + " out of bounds (size: " + itoa(itemOffsets.size()) + ")";
		size_t off = baseOffset() + itemOffsets[itemIndex].m_Offset + 8; // +8 to cut out type_and_id and size
		size_t size = getItemSize(itemIndex);
		data.resize(size);
		if(fseek(fp, off, SEEK_SET) != 0)
			return "getItem: fseek failed: " + std::string(strerror(errno));
		if(fread(&data[0], 1, size, fp) != size) {
			if(ferror(fp)) return "getItem: file read error";
			if(feof(fp)) return "getItem: eof reached";
			return "getItem: unknown fread error";
		}
		return true;
	}

	Result getItem(ItemType itemType, uint32_t relIndex, Raw& data) {
		CDatafileItemType* t = getItemType(itemType);
		if(!t) return "getItem: item type " + itoa((int)itemType) + " not found";
		if((int)relIndex >= t->m_Num) return "getItem: relIndex " + itoa(relIndex) + " out of bounds (size: " + itoa(t->m_Num) + ")";
		return getItem(t->m_Start + relIndex, data);
	}

	size_t getDataSize(uint32_t index) {
		if(index >= dataOffsets.size()) return 0;
		if(index == dataOffsets.size() - 1)
			return teeHeader.m_DataSize - dataOffsets[index].m_Offset;
		return dataOffsets[index + 1].m_Offset - dataOffsets[index].m_Offset;
	}

	Result getData(uint32_t index, Raw& data) {
		if(index >= dataOffsets.size()) return "getData: index out of bounds";
		size_t off = baseOffset() + teeHeader.m_ItemSize + dataOffsets[index].m_Offset;
		size_t size = getDataSize(index);
		data.resize(size);
		if(fseek(fp, off, SEEK_SET) != 0)
			return "getData: fseek failed: " + std::string(strerror(errno));
		if(fread(&data[0], 1, size, fp) != size) {
			if(ferror(fp)) return "getData: file read error";
			if(feof(fp)) return "getData: eof reached";
			return "getData: unknown fread error";
		}
		return true;
	}

	Result getDecompressedData(uint32_t index, Raw& data) {
		Raw compressedData;
		if(NegResult r = getData(index, compressedData)) return r.res;
		if(!Decompress(compressedData, &data))
			return "decompress failed";
		return true;
	}

	Result getDecompressedDataString(uint32_t index, std::string& str) {
		Raw data;
		if(NegResult r = getDecompressedData(index, data)) return r.res;
		if(data.size() == 0) return "getDecompressedDataString: data is empty";
		if(data[data.size() - 1] != '\0') return "getDecompressedDataString: non-zero at end";
		str = data.substr(0, data.size() - 1);
		return true;
	}

	Result parseVersion() {
		Raw data;
		if(NegResult r = getItem(ITEM_VERSION, 0, data))
			return "parseVersion failed: " + r.res.humanErrorMsg;
		if(data.size() < sizeof(uint32_t))
			return "version item not found / invalid";
		char* p = &data[0];
		uint32_t version = pread_endian<uint32_t>(p, &data[data.size()]);
		if(version != 1)
			return "wrong version";
		return true;
	}

	Result parseInfo() {
		Raw data;
		getItem(ITEM_INFO, 0, data);
		// TODO...
		return true; // not important
	}

	Result parseImages() {
		CDatafileItemType* t = getItemType(ITEM_IMAGE);
		if(!t) return "no images found";

		for(int i = 0; i < t->m_Num; ++i) {
			Raw item;
			if(NegResult r = getItem(t->m_Start + i, item))
				return "parseImages failed: " + r.res.humanErrorMsg;
			images.push_back(TWImage());
			if(NegResult r = images.back().read(this, &item[0], &item[item.size()]))
				return r.res;
		}
		return true;
	}

	Result parseGroups() {
		CDatafileItemType* t = getItemType(ITEM_GROUP);
		if(!t) return "no groups found";

		for(int i = 0; i < t->m_Num; ++i) {
			Raw item;
			if(NegResult r = getItem(t->m_Start + i, item))
				return "parseGroups failed: " + r.res.humanErrorMsg;
			groups.push_back(TWGroup());
			if(NegResult r = groups.back().read(this, &item[0], &item[item.size()]))
				return r.res;
		}
		return true;
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
		if(NegResult r = parseImages()) return r.res;
		if(NegResult r = parseGroups()) return r.res;

		// ...

		return "implementation incomplete";
	}

};

Result TWImage::read(ML_Teeworlds *l, char *p, char *end) {
	version = pread_endian<int32_t>(p, end);
	width = pread_endian<int32_t>(p, end);
	height = pread_endian<int32_t>(p, end);
	external = (bool) pread_endian<int32_t>(p, end);
	name_idx = pread_endian<int32_t>(p, end);
	data_idx = pread_endian<int32_t>(p, end);
	if(p > end) return "image item data is invalid, read behind end";

	if(NegResult r = l->getDecompressedDataString(name_idx, name)) return r.res;
	if(!external)
		if(NegResult r = l->getDecompressedData(data_idx, data))
			return r.res;

	return true;
}

Result TWGroup::read(ML_Teeworlds* l, char* p, char* end) {
	version = pread_endian<int32_t>(p, end);
	offset_x = pread_endian<int32_t>(p, end);
	offset_y = pread_endian<int32_t>(p, end);
	parallax_x = pread_endian<int32_t>(p, end);
	parallax_y = pread_endian<int32_t>(p, end);
	start_layer = pread_endian<int32_t>(p, end);
	num_layers = pread_endian<int32_t>(p, end);
	if(version >= 2) {
		use_clipping = (bool) pread_endian<int32_t>(p, end);
		clip_x = pread_endian<int32_t>(p, end);
		clip_y = pread_endian<int32_t>(p, end);
		clip_w = pread_endian<int32_t>(p, end);
		clip_h = pread_endian<int32_t>(p, end);
	}
	else {
		use_clipping = false;
		clip_x = clip_y = clip_w = clip_h = 0;
	}
	if(version >= 3 && end - p >= 3*4)
		name = rawReadStr(p, p + 3*4);
	else
		name = "";

	if(p > end) return "group item data is invalid, read behind end by " + itoa(p - end) + " bytes";
	return true;
}

Result TWGroup::readLayers(ML_Teeworlds* l) {
	CDatafileItemType* t = l->getItemType(ITEM_LAYER);
	if(!t) return "item layer type not found";
	if(start_layer + num_layers > t->m_Num)
		return "start_layer + num_layers are going out of bounds of item layers";
	layers.resize(num_layers);
	for(int j = 0; j < num_layers; ++j) {
		Raw data;
		if(NegResult r = l->getItem(t->m_Start + start_layer + j, data))
			return "readLayers: " + r.res.humanErrorMsg;
		if(NegResult r = layers[j].read(l, &data[0], &data[data.size()]))
			return "readLayer: " + r.res.humanErrorMsg;
	}
	return true;
}

Result TWLayer::read(ML_Teeworlds* l, char* p, char* end) {
	layer_version = pread_endian<int32_t>(p, end);
	type = (LayerType) pread_endian<int32_t>(p, end);
	flags = pread_endian<int32_t>(p, end);
	detail = (bool) flags;

	if(type == LAYERTYPE_TILES) {
		return tileLayer.read(l, p, end);
	} else if(type == LAYERTYPE_QUADS) {
		return quadLayer.read(l, p, end);
	} else {
		// ignore other types for now...
		if(p > end) return "layer itemdata is invalid, read behind end";
		return true;
	}
	assert(false); return false;
}

Result TWTileLayer::read(ML_Teeworlds* l, char* p, char* end) {
	version = pread_endian<int32_t>(p, end);
	width = pread_endian<int32_t>(p, end);
	height = pread_endian<int32_t>(p, end);
	game_type = pread_endian<int32_t>(p, end);
	color[0] = pread_endian<int32_t>(p, end);
	color[1] = pread_endian<int32_t>(p, end);
	color[2] = pread_endian<int32_t>(p, end);
	color[3] = pread_endian<int32_t>(p, end);
	color_env = pread_endian<int32_t>(p, end);
	color_env_offset = pread_endian<int32_t>(p, end);
	image_id = pread_endian<int32_t>(p, end);
	data_idx = pread_endian<int32_t>(p, end);
	name = "";
	if(version >= 3 && end - p >= 3*4)
		name = rawReadStr(p, p + 3*4);

	Raw tileData;
	if(NegResult r = l->getDecompressedData(data_idx, tileData))
		return "tile data: " + r.res.humanErrorMsg;

	// TODO: tiles

	// ignore tele_list, speedup_list

	if(p > end) return "tilelayer itemdata is invalid, read behind end";
	return true;
}

Result TWQuadLayer::read(ML_Teeworlds* l, char* p, char* end) {
	version = pread_endian<int32_t>(p, end);
	num_quads = pread_endian<int32_t>(p, end);
	data_idx = pread_endian<int32_t>(p, end);
	image_id = pread_endian<int32_t>(p, end);
	name = "";
	if(version >= 2 && end - p >= 3*4)
		name = rawReadStr(p, p + 3*4);

	// TODO ...

	if(p > end) return "quadlayer itemdata is invalid, read behind end";
	return true;
}

MapLoad* createMapLoad_Teeworlds() {
	return new ML_Teeworlds();
}
