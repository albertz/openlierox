//
// Boost.Process
//
// Copyright (c) 2006, 2007 Julio M. Merino Vidal.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.)
//

//!
//! \file boost/process/self.hpp
//!
//! Includes the declaration of the self class.
//!

#if !defined(BOOST_PROCESS_SELF_HPP)
/** \cond */
#define BOOST_PROCESS_SELF_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
#   include <sys/types.h>
#   include <unistd.h>
}
#elif defined(BOOST_PROCESS_WIN32_API)
#   include <windows.h>
#else
#   error "Unsupported platform."
#endif

#include <boost/noncopyable.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/process.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Generic implementation of the Process concept.
//!
//! The self singleton provides access to the current process.
//!
class self : public process, boost::noncopyable
{
    //!
    //! \brief Constructs a new self object.
    //!
    //! Creates a new self object that represents the current process.
    //!
    self(void);

public:
    //!
    //! \brief Returns the self instance representing the caller's process.
    //!
    //! Returns a reference to the self instance that represents the
    //! caller's process.
    //!
    static self& get_instance(void);

    //!
    //! \brief Returns the current environment.
    //!
    //! Returns the current process' environment variables.  Modifying the
    //! returned object has no effect on the current environment.
    //!
    const environment get_environment(void) const;

    //
    // TODO: Add methods to modify the current environment.
    // A preliminary approach could be:
    // has_environment(variable)
    // get_environment(variable)
    // set_environment(variable, value)
    // unset_environment(variable)
    //
};

// ------------------------------------------------------------------------

inline
self::self(void) :
#if defined(BOOST_PROCESS_POSIX_API)
    process(::getpid())
#elif defined(BOOST_PROCESS_WIN32_API)
    process(::GetCurrentProcessId())
#endif
{
}

// ------------------------------------------------------------------------

inline
self&
self::get_instance(void)
{
    static self *instance = NULL;

    if (instance == NULL)
        instance = new self;

    return *instance;
}

// ------------------------------------------------------------------------

inline
const environment
self::get_environment(void)
    const
{
    return current_environment();
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_SELF_HPP)
