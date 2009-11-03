/*
	OpenLieroX

	various utilities

	code under LGPL
	created 01-05-2007
	by Albert Zeyer and Dark Charlie
*/

#ifndef __INTERNDATACLASS_H__
#define __INTERNDATACLASS_H__

/*
	helpers for declaration/definition of classes
*/

#define INTERNDATA_CLASS_BEGIN(_classname) \
	class _classname { \
		struct Intern; \
	public: \
		_classname(); \
		_classname(const _classname& b); \
		_classname(const Intern& d); \
		_classname& operator=(const _classname& b); \
		void swap(_classname& b) { Intern* tmp = intern_data; intern_data = b.intern_data; b.intern_data = tmp; } \
		~_classname(); \
		bool operator==(const _classname& b); \
		bool operator!=(const _classname& b) { return ! ( *this == b ); } ; \
		Intern* intern_data; \
	private: \

#define INTERNDATA_CLASS_END \
	private: \
		void INTERNDATA__init(); \
		void INTERNDATA__reset(); \
	};

#define DEFINE_INTERNDATA_CLASS(_classname) \
	INTERNDATA_CLASS_BEGIN(_classname) \
	INTERNDATA_CLASS_END

#define	DECLARE_INTERNDATA_CLASS__WITH_INIT(_classname, _datatype, init_val) \
	struct _classname::Intern { \
		_datatype data; \
		Intern() : data(init_val) {} \
		Intern(const _datatype& d) : data(d) {} \
	}; \
	_classname::_classname() { INTERNDATA__init(); } \
	void _classname::INTERNDATA__init() { \
		intern_data = new Intern(); \
	} \
	_classname::~_classname() { INTERNDATA__reset(); } \
	void _classname::INTERNDATA__reset() { \
		if(!intern_data) return; \
		delete intern_data; \
		intern_data = NULL; \
	} \
	_classname::_classname(const _classname& b) { \
		INTERNDATA__init(); \
		if(intern_data) \
			*intern_data = *b.intern_data; \
	} \
	_classname::_classname(const Intern& d) { \
		INTERNDATA__init(); \
		if(intern_data) \
			*intern_data = d; \
	} \
	_classname& _classname::operator=(const _classname& b) { \
		if(&b == this || !intern_data) return *this; \
		*intern_data = *b.intern_data; \
		return *this; \
	} \
	_datatype& _classname##Data(_classname& obj) { \
		return obj.intern_data->data; \
	} \
	const _datatype& _classname##Data(const _classname& obj) { \
		return obj.intern_data->data; \
	} \
	bool _classname::operator==(const _classname& b) { \
		return ::Are ## _classname ## Equal( *this, b ); \
	}


#define	DECLARE_INTERNDATA_CLASS(_classname, _datatype) \
	DECLARE_INTERNDATA_CLASS__WITH_INIT(_classname, _datatype, _datatype())

#endif

