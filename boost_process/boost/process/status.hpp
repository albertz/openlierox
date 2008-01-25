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
//! \file boost/process/status.hpp
//!
//! Includes the declaration of the status class.
//!

#if !defined(BOOST_PROCESS_STATUS_HPP)
/** \cond */
#define BOOST_PROCESS_STATUS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
#   include <sys/wait.h>
}
#elif defined(BOOST_PROCESS_WIN32_API)
#else
#   error "Unsupported platform."
#endif

#include <boost/assert.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Status returned by a finalized child process.
//!
//! This class represents the %status returned by a child process after it
//! has terminated.  It only provides that information available under all
//! supported platforms.
//!
//! \see posix_status
//!
class status
{
protected:
    //!
    //! \brief OS-specific codification of exit status.
    //!
    int m_flags;

    //!
    //! \brief Creates a status object based on exit information.
    //!
    //! Creates a new status object representing the exit status of a
    //! child process.
    //!
    //! \param flags In a POSIX system this parameter contains the
    //!              flags returned by the ::waitpid() call.  In a
    //!              Win32 system it contains the exit code only.
    //!
    status(int flags);
    friend class child;

public:
    //!
    //! \brief Returns whether the process exited gracefully or not.
    //!
    //! Returns whether the process exited gracefully or not.
    //!
    bool exited(void) const;

    //!
    //! \brief If exited, returns the exit code.
    //!
    //! If the process exited, returns the exit code it returned.
    //!
    //! \pre exited() is true.
    //!
    int exit_status(void) const;
};

// ------------------------------------------------------------------------

inline
status::status(int flags) :
    m_flags(flags)
{
}

// ------------------------------------------------------------------------

inline
bool
status::exited(void)
    const
{
#if defined(BOOST_PROCESS_POSIX_API)
    return WIFEXITED(m_flags);
#elif defined(BOOST_PROCESS_WIN32_API)
    return true;
#endif
}

// ------------------------------------------------------------------------

inline
int
status::exit_status(void)
    const
{
    BOOST_ASSERT(exited());
#if defined(BOOST_PROCESS_POSIX_API)
    return WEXITSTATUS(m_flags);
#elif defined(BOOST_PROCESS_WIN32_API)
    return m_flags;
#endif
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_STATUS_HPP)
