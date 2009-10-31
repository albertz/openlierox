/*
	OpenLieroX

	reader for MaxMind's database format
	
	code under LGPL
	by Albert Zeyer and Dark Charlie
*/

// Some parts of the code were taken from MaxMind's GeoIPLite software (licensed under LGPL)

#ifndef __GEOIPDATABASE_H__
#define __GEOIPDATABASE_H__

class GeoRecord  {
public:
	std::string continentCode;
	std::string continent;
	std::string countryCode;
	std::string countryCode3;
	std::string countryName;
	bool hasCityLevel;
	std::string region;
	std::string city;
	std::string postalCode;
	double latitude;
	double longitude;
	union  {  // These are an alias
		int metroCode;
		int dmaCode;
	};

	int areaCode;

	GeoRecord& operator= (const GeoRecord& oth);
	GeoRecord() : latitude(0), longitude(0), metroCode(0), areaCode(1), 
		continentCode("UN"), continent("Unknown Continent"), countryCode("UN"), countryCode3("UNK"),
		countryName("Unknown Country"), hasCityLevel(false), region(""), city("Unknown City") {}
	GeoRecord(const GeoRecord& oth) { operator=(oth); }
};

class GeoIPDatabase  {
	FILE *m_file;
	std::string m_fileName;

	unsigned int *m_dbSegments;
	int m_dbType;
	int m_recordLength;

	bool setupSegments();
	unsigned int seekRecord(unsigned long ipnum) const;
	GeoRecord extractRecordCity(unsigned int seekRecord) const;
	GeoRecord extractRecordCtry(unsigned int seekRecord) const;
	unsigned long convertIp(const std::string& strIp) const;
	void fillContinent(GeoRecord& res) const;

public:
	GeoIPDatabase() : m_file(NULL), m_dbSegments(NULL), m_dbType(0), m_recordLength(0) {}
	~GeoIPDatabase();

	bool load(const std::string& filename);
	void close();
	bool loaded() const { return m_file != NULL; }

	GeoRecord lookup(const std::string& ip) const;

};

#endif  // __GEOIPDATABASE_H__
