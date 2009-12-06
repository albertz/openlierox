#ifndef OMFG_BLITTERS_CONTEXT_H
#define OMFG_BLITTERS_CONTEXT_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "blitters.h"

#define FUNC(name_, params_, none_, sendfact_) \
	void name_ params_ const { \
	switch(m_type) { \
		case None: name_##_solid none_; break; \
		case Add: name_##_add sendfact_; break; \
		case Alpha: name_##_blend sendfact_; break; \
		case AlphaChannel: name_##_blendalpha sendfact_; break; } }

#define FUNC_6(name_, P1, P2, P3, P4, P5, P6) \
	FUNC(name_ \
		, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) \
		, (p1, p2, p3, p4, p5, p6) \
		, (p1, p2, p3, p4, p5, p6, m_fact))
	
#define FUNC_5(name_, P1, P2, P3, P4, P5) \
	FUNC(name_ \
		, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) \
		, (p1, p2, p3, p4, p5) \
		, (p1, p2, p3, p4, p5, m_fact))
		
#define FUNC_4(name_, P1, P2, P3, P4) \
	FUNC(name_ \
		, (P1 p1, P2 p2, P3 p3, P4 p4) \
		, (p1, p2, p3, p4) \
		, (p1, p2, p3, p4, m_fact))
		
#define FUNC_8(name_, P1, P2, P3, P4, P5, P6, P7, P8) \
	FUNC(name_ \
		, (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) \
		, (p1, p2, p3, p4, p5, p6, p7, p8) \
		, (p1, p2, p3, p4, p5, p6, p7, p8, m_fact))
	

struct BlitterContext
{
	// Blender tags
	struct none {};
	struct add {};
	struct alpha {};
	
	enum Type
	{
		None = 0,
		Add,
		Alpha,
		AlphaChannel,
	};
	
	BlitterContext()
	: m_type(None)
	{
	}
	
	BlitterContext(Type type_, int fact_)
	: m_type(type_), m_fact(fact_)
	{
	}
	
	BlitterContext(none)
	: m_type(None)
	{
	}
	
	BlitterContext(add, int fact_)
	: m_type(Add), m_fact(fact_)
	{
	}
	
	BlitterContext(alpha, int fact_)
	: m_type(Alpha), m_fact(fact_)
	{
	}
	
	void set(Type type_, int fact_)
	{
		m_type = type_;
		m_fact = fact_;
	}
	
	void set(none)
	{
		m_type = None;
	}
	
	void set(add, int fact_)
	{
		m_type = Add;
		m_fact = fact_;
	}
	
	void set(alpha, int fact_)
	{
		m_type = Alpha;
		m_fact = fact_;
	}
		
	Type type() const
	{
		return m_type;
	}
	
	int fact() const
	{
		return m_fact;
	}
	
	FUNC_6(rectfill, BITMAP*, int, int, int, int, Pixel)
	FUNC_4(drawSprite, BITMAP*, BITMAP*, int, int)
	FUNC_8(drawSpriteCut, BITMAP*, BITMAP*, int, int, int, int, int, int)
	FUNC_4(putpixel, BITMAP*, int, int, Pixel)
	FUNC_4(putpixelwu, BITMAP*, float, float, Pixel)
	FUNC_6(linewu, BITMAP*, float, float, float, float, Pixel)
	FUNC_6(line, BITMAP*, int, int, int, int, Pixel)
	FUNC_5(hline, BITMAP*, int, int, int, Pixel)

private:
	Type m_type;
	int m_fact;
	
};

#endif //OMFG_BLITTERS_CONTEXT_H_
