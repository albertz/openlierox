//
// Boost.Process
//
// Copyright (c) 2006 Julio M. Merino Vidal.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.)
//

//!
//! \file boost/process/detail/config.hpp
//!
//! Defines macros that are used by the library's code to determine
//! features of the operating system used internally by the library.
//!

#if !defined(BOOST_PROCESS_DETAIL_CONFIG_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_CONFIG_HPP
/** \endcond */

#undef BOOST_PROCESS_WIN32_SAFE_FUNCTIONS
#if (defined(BOOST_MSVC) && BOOST_MSVC >= 1400) || \
    defined(BOOST_PROCESS_DOXYGEN)
//!
//! \brief Defined if the Win32-specific safe functions are available.
//!
//! This macro is defined (to no specific value) when Boost.Process is
//! being used on a system that has the Win32-specific safe functions
//! (e.g. strcpy_s).
//!
#   define BOOST_PROCESS_WIN32_SAFE_FUNCTIONS
#endif

#endif // !defined(BOOST_PROCESS_DETAIL_CONFIG_HPP)
