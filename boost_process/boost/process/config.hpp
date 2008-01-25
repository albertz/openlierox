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
//! \file boost/process/config.hpp
//!
//! Defines macros that are used by the library's code to determine the
//! operating system it is running under and the features it supports.
//!

#if !defined(BOOST_PROCESS_CONFIG_HPP)
/** \cond */
#define BOOST_PROCESS_CONFIG_HPP
/** \endcond */

#include <boost/config.hpp>

#undef BOOST_PROCESS_POSIX_API
#undef BOOST_PROCESS_WIN32_API

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || \
    defined(BOOST_PROCESS_DOXYGEN)
//!
//! \brief Defined if running under a Win32 system.
//!
//! This macro is defined (to no specific value) if Boost.Process is
//! used on a system with Win32 process management semantics.  It has to
//! be clear that, although Windows has some support for POSIX functions,
//! it is not considered to be a POSIX system.
//!
#   define BOOST_PROCESS_WIN32_API
#endif

#if !defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
//!
//! \brief Defined if running under a POSIX system.
//!
//! This macro is defined (to no specific value) if Boost.Process is
//! used on a system with POSIX process management semantics.  It is not
//! sufficient for the system to provide some common POSIX functions;
//! child processes need to be spawned using fork() and the whole
//! procedure management needs to follow Unix semantics.
//!
#   define BOOST_PROCESS_POSIX_API
#endif

#include <boost/process/detail/config.hpp>

#endif // !defined(BOOST_PROCESS_CONFIG_HPP)
