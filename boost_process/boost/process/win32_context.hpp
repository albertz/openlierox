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
//! \file boost/process/win32_context.hpp
//!
//! Includes the declaration of the win32_context class.
//!

#if !defined(BOOST_PROCESS_WIN32_CONTEXT_HPP)
/** \cond */
#define BOOST_PROCESS_WIN32_CONTEXT_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_WIN32_API)
#   error "Unsupported platform."
#endif

extern "C" {
#include <windows.h>
}

#include <string>

#include <boost/process/context.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Generic implementation of the Launcher concept.
//!
//! The context class implements the Launcher concept in an operating
//! system agnostic way; it allows spawning new child process using a
//! single and common interface across different systems.
//!
template< class String >
class win32_basic_context :
    public basic_context< String >
{
public:
    win32_basic_context(void);

    //!
    //! \brief Win32-specific process startup information.
    //!
    STARTUPINFO* m_startupinfo;
};

typedef win32_basic_context< std::string > win32_context;

// ------------------------------------------------------------------------

template< class String >
win32_basic_context< String >::win32_basic_context(void) :
    m_startupinfo(NULL)
{
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_WIN32_CONTEXT_HPP)
