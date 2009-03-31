/*
 *  StaticAssert.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 30.03.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__STATICASSERT_H__
#define __OLX__STATICASSERT_H__

#define OLXJOINSTR(X, Y) OLXJOINSTR2(X,Y)
#define OLXJOINSTR2(X, Y) OLXJOINSTR3(X,Y)
#define OLXJOINSTR3(X, Y) X##Y


// TODO: the following code uses instances, would it be possible to use the MSVC version?
#ifndef _MSC_VER
#define static_assert(X, desc) \
static struct StaticAssertChecker_ ## desc { \
	char desc[((bool)(X)) ? 1 : -1]; \
} desc; \
static struct StaticAssertIgnoreWarning_ ## desc { \
	StaticAssertIgnoreWarning_ ## desc ( StaticAssertChecker_ ## desc & tmp ) \
	{ tmp.desc[0] = 42; } \
} ignoreWarning_ ## desc (desc);
#else

template <bool b> class static_assert_failure;
template <> class static_assert_failure<true>  { char foo; };
template <int s> class static_assert_test{};

#define static_assert(X, desc)  \
	typedef static_assert_test< (int)sizeof(static_assert_failure< (bool)(X) >) > static_assert_typedef_##desc;

	
#endif


#endif
