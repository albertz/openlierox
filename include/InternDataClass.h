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
	public: \
		_classname(); \
		_classname(const _classname& b); \
		void operator=(const _classname& b); \
		void swap(_classname& b) { void* tmp = intern_data; intern_data = b.intern_data; b.intern_data = tmp; } \
		~_classname(); \
		void* intern_data; \
	private:

#define INTERNDATA_CLASS_END \
	private: \
		void INTERNDATA__init(); \
		void INTERNDATA__reset(); \
	};

#define DEFINE_INTERNDATA_CLASS(_classname) \
	INTERNDATA_CLASS_BEGIN(_classname) \
	INTERNDATA_CLASS_END

#define	DECLARE_INTERNDATA_CLASS(_classname, _datatype) \
	_classname::_classname() { INTERNDATA__init(); } \
	void _classname::INTERNDATA__init() { \
		intern_data = new _datatype; \
	} \
	_classname::~_classname() { INTERNDATA__reset(); } \
	void _classname::INTERNDATA__reset() { \
		if(!intern_data) return; \
		delete (_datatype*)intern_data; \
		intern_data = NULL; \
	} \
	_classname::_classname(const _classname& b) { \
		INTERNDATA__init(); \
		if (intern_data) \
			*(_datatype*)intern_data = *(const _datatype*)b.intern_data; \
	} \
	void _classname::operator=(const _classname& b) { \
		if(&b == this || !intern_data) return; \
		*(_datatype*)intern_data = *(const _datatype*)b.intern_data; \
	} \
	_datatype* _classname##Data(_classname* obj) { \
		if(obj) return (_datatype*)obj->intern_data; \
		else return NULL; \
	} \
	const _datatype* _classname##Data(const _classname* obj) { \
		if(obj) return (_datatype*)obj->intern_data; \
		else return NULL; \
	}


#endif

