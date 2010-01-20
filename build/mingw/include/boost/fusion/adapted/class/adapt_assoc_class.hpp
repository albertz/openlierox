/*=============================================================================
    Copyright (c) 2001-2009 Joel de Guzman
    Copyright (c) 2007 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_ADAPT_ASSOC_CLASS_OCTOBER_4_2009_840PM)
#define BOOST_FUSION_ADAPT_ASSOC_CLASS_OCTOBER_4_2009_840PM

#include <boost/fusion/support/tag_of_fwd.hpp>
#include <boost/fusion/adapted/class/extension.hpp>
#include <boost/fusion/adapted/class/class_iterator.hpp>
#include <boost/fusion/adapted/class/detail/is_view_impl.hpp>
#include <boost/fusion/adapted/class/detail/is_sequence_impl.hpp>
#include <boost/fusion/adapted/class/detail/category_of_impl.hpp>
#include <boost/fusion/adapted/class/detail/begin_impl.hpp>
#include <boost/fusion/adapted/class/detail/end_impl.hpp>
#include <boost/fusion/adapted/class/detail/size_impl.hpp>
#include <boost/fusion/adapted/class/detail/at_impl.hpp>
#include <boost/fusion/adapted/class/detail/value_at_impl.hpp>
#include <boost/fusion/adapted/class/detail/has_key_impl.hpp>
#include <boost/fusion/adapted/class/detail/at_key_impl.hpp>
#include <boost/fusion/adapted/class/detail/value_at_key_impl.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/mpl/int.hpp>
#include <boost/config/no_tr1/utility.hpp>

namespace boost { namespace fusion { namespace extension {
    template<typename Class, typename Key>
    struct class_assoc_member;
}}}


#define BOOST_FUSION_ADAPT_ASSOC_CLASS(name, bseq)                             \
    BOOST_FUSION_ADAPT_ASSOC_CLASS_I(                                           \
        name, BOOST_PP_CAT(BOOST_FUSION_ADAPT_ASSOC_CLASS_X bseq, 0))           \
    /***/

#define BOOST_FUSION_ADAPT_ASSOC_CLASS_X(x, y, z) ((x, y, z)) BOOST_FUSION_ADAPT_ASSOC_CLASS_Y
#define BOOST_FUSION_ADAPT_ASSOC_CLASS_Y(x, y, z) ((x, y, z)) BOOST_FUSION_ADAPT_ASSOC_CLASS_X
#define BOOST_FUSION_ADAPT_ASSOC_CLASS_X0
#define BOOST_FUSION_ADAPT_ASSOC_CLASS_Y0

// BOOST_FUSION_ADAPT_ASSOC_CLASS_I generates the overarching structure and uses
// SEQ_FOR_EACH_I to generate the "linear" substructures.
// Thanks to Paul Mensonides for the PP macro help

#define BOOST_FUSION_ADAPT_ASSOC_CLASS_I(name, seq)                             \
    namespace boost { namespace fusion { namespace traits                       \
    {                                                                           \
        template <>                                                             \
        struct tag_of<name>                                                     \
        {                                                                       \
            typedef class_tag type;                                             \
        };                                                                      \
    }}}                                                                         \
    namespace boost { namespace fusion { namespace extension                    \
    {                                                                           \
        template <>                                                             \
        struct class_size<name> : mpl::int_<BOOST_PP_SEQ_SIZE(seq)> {};         \
        BOOST_PP_SEQ_FOR_EACH_I(BOOST_FUSION_ADAPT_ASSOC_CLASS_C, name, seq)    \
    }}}                                                                         \
    /***/

#define BOOST_FUSION_ADAPT_ASSOC_CLASS_C(r, name, i, xy)                        \
    template <>                                                                 \
    struct class_member<name, i>                                                \
    {                                                                           \
        typedef BOOST_PP_TUPLE_ELEM(3, 0, xy) type;                             \
        static type& call(name& class_)                                         \
        {                                                                       \
            return class_.BOOST_PP_TUPLE_ELEM(3, 1, xy);                        \
        };                                                                      \
    };                                                                          \
    template<>                                                                  \
    struct class_assoc_member<name, BOOST_PP_TUPLE_ELEM(3, 2, xy)>              \
    {                                                                           \
        typedef BOOST_PP_TUPLE_ELEM(3, 0, xy) type;                             \
        static type& call(name& class_)                                         \
        {                                                                       \
            return class_.BOOST_PP_TUPLE_ELEM(3, 1, xy);                        \
        };                                                                      \
    };
    /***/

#endif
