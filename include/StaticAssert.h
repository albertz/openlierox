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


#define static_assert(X, desc) \
static struct StaticAssertChecker_ ## desc { \
	char desc[((bool)(X)) ? 1 : -1]; \
} desc; \
static struct StaticAssertIgnoreWarning_ ## desc { \
	StaticAssertIgnoreWarning_ ## desc ( StaticAssertChecker_ ## desc & tmp ) \
	{ tmp.desc[0] = 42; } \
} ignoreWarning_ ## desc (desc);


#endif
