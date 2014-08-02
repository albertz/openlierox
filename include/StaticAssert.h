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

#if __cplusplus <= 199711L

template <bool b> class static_assert_failure;
template <> class static_assert_failure<true>  { char foo; };
template <int s> class static_assert_test{};

#define static_assert(X, desc)  \
	typedef static_assert_test< (int)sizeof(static_assert_failure< (bool)(X) >) > static_assert_typedef_##desc;

#endif

#endif
