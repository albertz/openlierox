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
//! \file boost/process/process.hpp
//!
//! Includes the declaration of the process class.
//!

#if !defined(BOOST_PROCESS_PROCESS_HPP)
/** \cond */
#define BOOST_PROCESS_PROCESS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
#   include <sys/types.h>
#   include <signal.h>
}
#   include <cerrno>
#elif defined(BOOST_PROCESS_WIN32_API)
#   include <windows.h>
#   include <cstdlib>
#else
#   error "Unsupported platform."
#endif

#include <boost/process/exceptions.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace process {

//template< class Executable, class Arguments > class basic_pipeline;
//class launcher;

// ------------------------------------------------------------------------

//!
//! \brief Generic implementation of the Process concept.
//!
//! The process class implements the Process concept in an operating system
//! agnostic way.
//!
class process
{
public:
#if defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Opaque name for the native process' identifier type.
    //!
    //! Each operating system identifies processes using a specific type.
    //! The \a id_type type is used to transparently refer to a process
    //! regarless of the operating system in which this class is used.
    //!
    //! This type is guaranteed to be an integral type on all supported
    //! platforms.
    //!
    typedef NativeProcessId id_type;
#elif defined(BOOST_PROCESS_WIN32_API)
    typedef DWORD id_type;
#elif defined(BOOST_PROCESS_POSIX_API)
    typedef pid_t id_type;
#endif

    //!
    //! \brief Returns the process' identifier.
    //!
    //! Returns the process' identifier.
    //!
    id_type get_id(void) const;

private:
    //!
    //! \brief The process' identifier.
    //!
    id_type m_id;

public: // XXX?
    //!
    //! \brief Constructs a new process object.
    //!
    //! Creates a new process object that represents a running process
    //! within the system.
    //!
    //! This constructor is protected because the library user has no
    //! business in creating representations of live processes himself;
    //! the library takes care of that in all cases.
    //!
    process(id_type id);
    //template< class Executable, class Arguments >
    //    friend class basic_pipeline;
    //friend class launcher;

    //!
    //! \brief Terminates the process execution.
    //!
    //! Forces the termination of the process execution.  Some platforms
    //! allow processes to ignore some external termination notifications
    //! or to capture them for a proper exit cleanup.  You can set the
    //! \a force flag to true in them to force their termination regardless
    //! of any exit handler.
    //!
    //! After this call, accessing this object can be dangerous because the
    //! process identifier may have been reused by a different process.  It
    //! might still be valid, though, if the process has refused to die.
    //!
    //! \throw system_error If the system call used to terminate the
    //!                     process fails.
    //!
    void terminate(bool force = false) const;
};

// ------------------------------------------------------------------------

inline
process::process(id_type id) :
    m_id(id)
{
}

// ------------------------------------------------------------------------

inline
process::id_type
process::get_id(void)
    const
{
    return m_id;
}

// ------------------------------------------------------------------------

inline
void
process::terminate(bool force)
    const
{
#if defined(BOOST_PROCESS_POSIX_API)
    if (::kill(m_id, force ? SIGKILL : SIGTERM) == -1)
        boost::throw_exception
            (system_error("boost::process::process::terminate",
                          "kill(2) failed", errno));
#elif defined(BOOST_PROCESS_WIN32_API)
    HANDLE h = ::OpenProcess(PROCESS_TERMINATE, FALSE, m_id);
    if (h == INVALID_HANDLE_VALUE)
        boost::throw_exception
            (system_error("boost::process::process::terminate",
                          "OpenProcess failed", ::GetLastError()));
    if (::TerminateProcess(h, EXIT_FAILURE) == 0)
        boost::throw_exception
            (system_error("boost::process::process::terminate",
                          "TerminateProcess failed", ::GetLastError()));
#endif
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_PROCESS_HPP)
