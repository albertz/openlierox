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
struct OLXJOINSTR(StaticAssertTest, __LINE__) { \
void dummy() { char desc[((bool)(X)) ? 1 : -1]; desc[0] = 42; } \
};


#endif
