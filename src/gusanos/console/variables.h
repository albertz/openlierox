#ifndef VARIABLES_H
#define VARIABLES_H

#include "consoleitem.h"
#include "util/text.h"
#include <boost/function.hpp>
//#include <boost/lexical_cast.hpp>
//using boost::lexical_cast;

#include <string>
#include <map>

#define VAR_TYPE_INVALID 0
#define VAR_TYPE_INT 1

class Variable : public ConsoleItem
{
	public:
	
	Variable(std::string const& name)
	: m_name(name)
	{
	}
	
	Variable()
	{
	}
	
	virtual ~Variable()
	{
	}

	std::string const& getName()
	{ return m_name; }
	
	protected:

	std::string m_name;
};

template<class T>
class TVariable : public Variable
{
	public:
	
	typedef boost::function<void (T const&)> CallbackT;
	
	TVariable(std::string name, T* src, T defaultValue, CallbackT const& callback = CallbackT() )
	: Variable(name), m_src(src), m_callback(callback)
	{
		*src = defaultValue;
	}
	
	TVariable()
	: Variable(), m_src(NULL), m_defaultValue(T()), m_callback(NULL)
	{
	}
	
	virtual ~TVariable()
	{
	}
	
	std::string invoke(std::list<std::string> const& args)
	{
		if (!args.empty())
		{
			T oldValue = *m_src;
			*m_src = convert<T>::value(*args.begin());
			if ( m_callback ) m_callback(oldValue);
	
			return std::string();
		}else
		{
			//return m_name + " IS \"" + cast<std::string>(*m_src) + '"';
			return convert<std::string>::value(*m_src);
		}
	}
	
	private:

	T* m_src;
	T m_defaultValue;
	CallbackT m_callback;
};

/*
template<class T, class IT>
inline TVariable<T>* tVariable(std::string name, T* src, IT defaultValue, void (*callback)( T ) = NULL )
{
	return new TVariable<T>(name, src, defaultValue, callback);
}*/

/*
class IntVariable : public Variable
{
	public:
	
	IntVariable(int* src, std::string name, int defaultValue, void (*func)( int ) );
	IntVariable();
	~IntVariable();
	
	std::string invoke(const std::list<std::string> &args);
	
	private:

	void (*callback)( int );
	int* m_src;
	int m_defaultValue;
};

class FloatVariable : public Variable
{
	public:
	
	FloatVariable( float* src, std::string name, float defaultValue, void (*func)( float ) );
	FloatVariable();
	~FloatVariable();
	
	std::string invoke(const std::list<std::string> &args);
	
	private:
	
	void (*callback)( float );
	float* m_src;
	float m_defaultValue;
};*/

typedef TVariable<int> IntVariable;
typedef TVariable<float> FloatVariable;
typedef TVariable<std::string> StringVariable;

class EnumVariable : public Variable
{
public:
	
	typedef std::map<std::string, int, IStrCompare> MapType;
	typedef std::map<int, std::string> ReverseMapType;
	
	typedef boost::function<void (int)> CallbackT;
	
	EnumVariable(std::string name, int* src, int defaultValue, MapType const& mapping, CallbackT const& func = CallbackT());
	EnumVariable();
	virtual ~EnumVariable();
	
	std::string invoke(const std::list<std::string> &args);
	
	virtual std::string completeArgument(int idx, std::string const& beginning);
	
private:
	
	int* m_src;
	int m_defaultValue;	
	ReverseMapType m_reverseMapping;
	MapType m_mapping;
	CallbackT m_callback;
};

#endif  // _VARIABLES_H_
