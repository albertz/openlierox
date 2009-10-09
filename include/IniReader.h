/*
	OpenLieroX
	
	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/

#ifndef __INI_READER_H__
#define __INI_READER_H__

#include "StringUtils.h"
#include <map>

struct Color;

/*
	to use this class, you have to create a subclass from it and
	overload the OnNewSection or/and OnEntry
*/
class IniReader {
public:
	typedef std::map<std::string, int, stringcaseless> KeywordList;


	IniReader(const std::string& filename, KeywordList& keywords = IniReader::DefaultKeywords);
	virtual ~IniReader();
	
	// returns false if there was an error
	// if you break via the callbacks, this is also an error
	bool Parse();

	// if the return value is false, the parsing will break 
	virtual bool OnNewSection (const std::string& section) { return true; }
	virtual bool OnEntry (const std::string& section, const std::string& propname, const std::string& value) { return true; }

	// Reading
	bool ReadString(const std::string& section, const std::string& key, std::string& value, std::string defaultv) const;
	bool ReadInteger(const std::string& section, const std::string& key, int *value, int defaultv) const;
	bool ReadFloat(const std::string& section, const std::string& key, float *value, float defaultv) const;
	bool ReadIntArray(const std::string& section, const std::string& key, int *array, int num_items) const;
	bool ReadColour(const std::string& section, const std::string& key, Color& value, const Color& defaultv) const;
	bool ReadKeyword(const std::string& section, const std::string& key, int *value, int defaultv) const;
	bool ReadKeyword(const std::string& section, const std::string& key, bool *value, bool defaultv) const;
	bool ReadKeywordList(const std::string& section, const std::string& key, int *value, int defaultv)const;

	template<typename T>
	bool ReadArray(const std::string& section, const std::string& key, T* data, size_t num) const {
		std::string string;
		
		if (!ReadString(section, key, string, ""))
			return false;
		
		std::vector<std::string> arr = explode(string,",");
		for (size_t i=0; i< MIN(num,arr.size()); i++)
			data[i] = from_string<T>(arr[i]);
		
		return num == arr.size();
	}

	template<typename T>
	bool ReadVectorD2(const std::string& section, const std::string& key, VectorD2<T>& v, VectorD2<T> defv = VectorD2<T>(), bool acceptSimple = true) const {
		v = defv;
		
		T _v[2] = {0,0};
		if(!ReadArray(section, key, _v, 2)) {
			if(!acceptSimple || !ReadArray(section, key, _v, 1)) return false;
			v.x = v.y = _v[0];
			return true;
		}
		
		v.x = _v[0]; v.y = _v[1];
		return true;
	}

	template<typename T>
	bool ReadMatrixD2(const std::string& section, const std::string& key, MatrixD2<T>& v, MatrixD2<T> defv = MatrixD2<T>(), bool acceptSimple = true) const {
		v = defv;
		
		T _v[4] = {0,0,0,0};
		if(!ReadArray(section, key, _v, 4)) {
			if(!acceptSimple || !ReadArray(section, key, _v, 1)) return false;
			v.v1.x = v.v2.y = _v[0];
			v.v1.y = v.v2.x = 0;
			return true;
		}
		
		v.v1.x = _v[0]; v.v1.y = _v[1]; v.v2.x = _v[2]; v.v2.y = _v[3];
		return true;
	}

	// Keyword
	std::string getFileName() const { return m_filename; }

	static KeywordList DefaultKeywords;
protected:
	typedef std::map<std::string, std::string> Section;
	typedef std::map<std::string, Section> SectionMap;

	std::string m_filename;
	SectionMap m_sections;
	KeywordList& m_keywords;

private:
	Section *m_curSection;

private:
	bool GetString(const std::string& section, const std::string& key, std::string& string) const;
	void NewSection(const std::string& name);
	void NewEntryInSection(const std::string& name, const std::string& value);
};

#endif

