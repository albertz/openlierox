/*
	OpenLieroX
	
	INI reader

	18-01-2008, by Albert Zeyer
	code under LGPL
*/

#ifndef __INI_READER_H__
#define __INI_READER_H__

#include <string>

class IniReader {
public:
	IniReader(const std::string& filename);
	~IniReader();
	
	// if the return value is false, the parsing will break 
	typedef bool (*OnNewSection) (void* userData, const std::string& section);
	typedef bool (*OnEntry) (void* userData, const std::string& section, const std::string& propname, const std::string& value);

	bool Parse(OnNewSection onNewSectionCallback, OnEntry onEntryCallback, void* userData = NULL);

private:
	std::string m_filename;

private:
	// TODO: are these wrapper possible in a more general way?
	// I would know how to do it if the function to be called is just the operator(),
	// though in this case it's nicer to have the functions named.
	// Another possibility would be to have 2 userData pointers for the main Parse function
	// and to use just two seperated functors.
	
	template< typename _CallbackHandler >
	static bool OnNewSectionWrapper(void* userData, const std::string& section) {
		return ((_CallbackHandler*)userData)->OnNewSection(section);
	}

	template< typename _CallbackHandler >
	static bool OnEntryWrapper(void* userData, const std::string& section, const std::string& propname, const std::string& value) {
		return ((_CallbackHandler*)userData)->OnEntry(section, propname, value);
	}

public:		
	/*
		_CallbackHandler should be a class with 2 member functions compatible to:
			OnNewSection(string section)
			OnEntry(string section, string propname, string value)
	*/
	template< typename _CallbackHandler >
	bool Parse(_CallbackHandler& handler) {
		return Parse( &OnNewSectionWrapper<_CallbackHandler>, &OnEntryWrapper<_CallbackHandler>, &handler );
	}
};

// we could use this interface if we want to define a local callback handler class
// read http://www.informit.com/articles/article.aspx?p=345948&seqNum=3&rl=1 if interested why I am doing this
// generaly it is not allowed to use a local class as a template argument and this is the workaround
class _IniReaderCallback {
public:
	virtual ~_IniReaderCallback() {}
	virtual bool OnNewSection(const std::string& section) = 0;
	virtual bool OnEntry(const std::string& section, const std::string& propname, const std::string& value) = 0;
};

#endif

