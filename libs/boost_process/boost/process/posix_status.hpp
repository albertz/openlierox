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
//! \file boost/process/posix_status.hpp
//!
//! Includes the declaration of the posix_status class.
//!

#if !defined(BOOST_PROCESS_POSIX_STATUS_HPP)
/** \cond */
#define BOOST_PROCESS_POSIX_STATUS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_POSIX_API)
#   error "Unsupported platform."
#endif

#include <boost/process/status.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Status returned by a finalized child process on a POSIX system.
//!
//! This class represents the %status returned by a child process after it
//! has terminated.  It contains some methods not available in the status
//! class that provide information only available in POSIX systems.
//!
class posix_status :
    public status
{
public:
    //!
    //! \brief Creates a posix_status object from an existing status
    //!        object.
    //!
    //! Creates a new status object representing the exit status of a
    //! child process.  The construction is done based on an existing
    //! status object which already contains all the available
    //! information: this class only provides controlled access to it.
    //!
    posix_status(const status& s);

    //!
    //! \brief Returns whether the process exited due to an external
    //!        signal.
    //!
    //! Returns whether the process exited due to an external signal.
    //! The result is always false in Win32 systems.
    //!
    bool signaled(void) const;

    //!
    //! \brief If signaled, returns the terminating signal code.
    //!
    //! If the process was signaled, returns the terminating signal code.
    //! Cannnot be called under Win32 because the preconditions will not
    //! ever be met.
    //!
    //! \pre signaled() is true.
    //!
    int term_signal(void) const;

    //!
    //! \brief If signaled, returns whether the process dumped core.
    //!
    //! If the process was signaled, returns whether the process
    //! produced a core dump.
    //! Cannnot be called under Win32 because the preconditions will not
    //! ever be met.
    //!
    //! \pre signaled() is true.
    //!
    bool dumped_core(void) const;

    //!
    //! \brief Returns whether the process was stopped by an external
    //!        signal.
    //!
    //! Returns whether the process was stopped by an external signal.
    //! The result is always false in Win32 systems.
    //!
    bool stopped(void) const;

    //!
    //! \brief If stpped, returns the stop signal code.
    //!
    //! If the process was stopped, returns the stop signal code.
    //! Cannnot be called under Win32 because the preconditions will not
    //! ever be met.
    //!
    //! \pre signaled() is true.
    //!
    int stop_signal(void) const;
};

// ------------------------------------------------------------------------

inline
posix_status::posix_status(const status& s) :
    status(s)
{
}

// ------------------------------------------------------------------------

inline
bool
posix_status::signaled(void)
    const
{
    return WIFSIGNALED(m_flags);
}

// ------------------------------------------------------------------------

inline
int
posix_status::term_signal(void)
    const
{
    BOOST_ASSERT(signaled());
    return WTERMSIG(m_flags);
}

// ------------------------------------------------------------------------

inline
bool
posix_status::dumped_core(void)
    const
{
    BOOST_ASSERT(signaled());
    return WCOREDUMP(m_flags);
}

// ------------------------------------------------------------------------

inline
bool
posix_status::stopped(void)
    const
{
    return WIFSTOPPED(m_flags);
}

// ------------------------------------------------------------------------

inline
int
posix_status::stop_signal(void)
    const
{
    BOOST_ASSERT(stopped());
    return WSTOPSIG(m_flags);
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_POSIX_STATUS_HPP)
