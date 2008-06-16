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
//! \file boost/process/environment.hpp
//!
//! Includes the declaration of the environment class and related
//! free functions.
//!

#if !defined(BOOST_PROCESS_ENVIRONMENT_HPP)
/** \cond */
#define BOOST_PROCESS_ENVIRONMENT_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
#   include <cstring>
#elif defined(BOOST_PROCESS_WIN32_API)
extern "C" {
#   include <tchar.h>
#   include <windows.h>
}
#else
#   error "Unsupported platform."
#endif

#include <map>
#include <string>

#include <boost/process/exceptions.hpp>
#include <boost/throw_exception.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
    extern char** environ;
}
#endif

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Representation of a process' environment variables.
//!
//! The environment is a map that stablishes an unidirectional
//! association between variable names and their values and is
//! represented by a string to string map.
//!
//! Variables may be defined to the empty string.  Be aware that doing so
//! is not portable: POSIX systems will treat such variables as being
//! defined to the empty value, but Win32 systems are not able to
//! distinguish them from undefined variables.
//!
//! Similarly, Win32 systems support a variable with no name that holds
//! the path to the current working directory; you may set it if you want
//! to, but the library will do the job for you if unset.  Contrarywise
//! POSIX systems do not support variables without names.
//!
//! It is worthy to note that the environment is sorted alphabetically.
//! This is provided for-free by the map container used to implement this
//! type, and this behavior is required by Win32 systems.
//!
typedef std::map< std::string, std::string > environment;

// ------------------------------------------------------------------------

//!
//! \brief Returns a snapshot of the current environment.
//!
//! This function grabs a snapshot of the current environment and returns
//! it to the caller.  The returned object can be modified later on but
//! changes to it do \b not modify the current environment.
//!
//! XXX self.environment() does the same as this.  One of the two has to
//! go away, most likely this one.
//!
inline
environment
current_environment(void)
{
    environment env;

#if defined(BOOST_PROCESS_POSIX_API)
    char** ptr = ::environ;
    while (*ptr != NULL) {
        std::string str = *ptr;
        std::string::size_type pos = str.find('=');
        env.insert
            (environment::value_type(str.substr(0, pos),
                                     str.substr(pos + 1, str.length())));
        ptr++;
    }
#elif defined(BOOST_PROCESS_WIN32_API)
    TCHAR* es = ::GetEnvironmentStrings();
    if (es == NULL)
        boost::throw_exception
            (system_error("boost::process::current_environment",
                          "GetEnvironmentStrings failed", ::GetLastError()));

    try {
        TCHAR* escp = es;
        while (*escp != '\0') {
            std::string str = escp;
            std::string::size_type pos = str.find('=');
            env.insert
                (environment::value_type(str.substr(0, pos),
                                         str.substr(pos + 1, str.length())));
            escp += str.length() + 1;
        }
    } catch (...) {
        ::FreeEnvironmentStrings(es);
        throw;
    }

    ::FreeEnvironmentStrings(es);
#endif

    return env;
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_ENVIRONMENT_HPP)
