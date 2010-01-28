#ifndef OMFGUTIL_DETAIL_COMPARE_H
#define OMFGUTIL_DETAIL_COMPARE_H

namespace
{

template<class T>
struct CompWrapper
{
	explicit CompWrapper(T const& v_, bool value_ = true)
	: v(v_), value(value_)
	{
	}
	
	template<class B>
	CompWrapper<B> operator<(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v < rhs));
	}
	
	template<class B>
	CompWrapper<B> operator<=(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v <= rhs));
	}
	
	template<class B>
	CompWrapper<B> operator>(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v > rhs));
	}
	
	template<class B>
	CompWrapper<B> operator>=(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v >= rhs));
	}
	
	template<class B>
	CompWrapper<B> operator==(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v == rhs));
	}
	
	template<class B>
	CompWrapper<B> operator!=(B const& rhs) const
	{
		return CompWrapper<B>(rhs, value && (v != rhs));
	}
	
	operator bool() const
	{
		return value;
	}
	
private:
	T const& v;
	bool value;
};

}

template<class T>
CompWrapper<T> I_(T const& v)
{
	return CompWrapper<T>(v);
}

#endif //OMFGUTIL_DETAIL_COMPARE_H
