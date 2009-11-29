#ifndef OMFGUTIL_DETAIL_MACROS_H
#define OMFGUTIL_DETAIL_MACROS_H

#include <boost/preprocessor/cat.hpp>

//NOTE: It's important that these are defined on a single-line
//since all references to __LINE__ must evaluate to the same value.

#define foreach( i, c )\
  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( __typeof__(BOOST_PP_CAT(C_, __LINE__).begin()) i = BOOST_PP_CAT(C_, __LINE__).begin(); i != BOOST_PP_CAT(C_, __LINE__).end(); ++i )
//  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( BOOST_PP_CAT(T_, __LINE__)::iterator i = BOOST_PP_CAT(C_, __LINE__).begin(); i != BOOST_PP_CAT(C_, __LINE__).end(); ++i )

#define foreach_bool( i, c )\
  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( __typeof__(BOOST_PP_CAT(C_, __LINE__).begin()) i = BOOST_PP_CAT(C_, __LINE__).begin(); i; ++i )
  
#define foreach_delete( i, c )\
  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( __typeof__(BOOST_PP_CAT(C_, __LINE__).begin()) i = BOOST_PP_CAT(C_, __LINE__).begin(), next; (i != BOOST_PP_CAT(C_, __LINE__).end()) && (next = i, ++next, true); i = next )
  
#define const_foreach( i, c )\
  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( __typeof__(BOOST_PP_CAT(C_, __LINE__).begin()) i = BOOST_PP_CAT(C_, __LINE__).begin(); i != BOOST_PP_CAT(C_, __LINE__).end(); ++i )

#define reverse_foreach( i, c )\
  typedef __typeof__( c ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__)& BOOST_PP_CAT(C_, __LINE__) = (c); for( __typeof__(BOOST_PP_CAT(C_, __LINE__).rbegin()) i = BOOST_PP_CAT(C_, __LINE__).rbegin(); i != BOOST_PP_CAT(C_, __LINE__).rend(); ++i )
  
#define forrange( i, b, e )\
  typedef __typeof__( b ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__) BOOST_PP_CAT(E_, __LINE__) = (e); for( BOOST_PP_CAT(T_, __LINE__) i = (b); i != BOOST_PP_CAT(E_, __LINE__); ++i )

#define forrange_delete( i, b, e )\
  typedef __typeof__( b ) BOOST_PP_CAT(T_, __LINE__); BOOST_PP_CAT(T_, __LINE__) BOOST_PP_CAT(E_, __LINE__) = (e); for( BOOST_PP_CAT(T_, __LINE__) i = (b), next; (i != BOOST_PP_CAT(E_, __LINE__)) && (next = i, ++next, true); i = next )

#define forrange_bool( i, b )\
  typedef __typeof__( b ) BOOST_PP_CAT(T_, __LINE__); for( BOOST_PP_CAT(T_, __LINE__) i = (b); i; ++i )

#define let_(i, v) __typeof__(v) i = v

#endif //OMFGUTIL_DETAIL_MACROS_H
