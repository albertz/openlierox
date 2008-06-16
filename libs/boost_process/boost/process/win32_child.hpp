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
//! \file boost/process/win32_child.hpp
//!
//! Includes the declaration of the win32_child class.
//!

#if !defined(BOOST_PROCESS_WIN32_CHILD_HPP)
/** \cond */
#define BOOST_PROCESS_WIN32_CHILD_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_WIN32_API)
#   error "Unsupported platform."
#endif

extern "C" {
#include <windows.h>
}

#include <boost/process/child.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Win32 implementation of the Child concept.
//!
//! The win32_child class implements the Child concept in a Win32
//! operating system.
//!
//! A Win32 child differs from a regular child (represented by a
//! child object) in that it holds additional information about a process.
//! Aside from the standard handle, it also includes a handle to the
//! process' main thread, together with identifiers to both entities.
//!
//! This class is built on top of the generic child so as to allow its
//! trivial adoption.  When a program is changed to use the Win32-specific
//! launcher (win32_launcher), it will most certainly need to migrate its
//! use of the child class to win32_child.  Doing so is only a matter of
//! redefining the appropriate object and later using the required extra
//! features: there should be no need to modify the existing code (e.g.
//! method calls) in any other way.
//!
class win32_child :
    public child
{
    //!
    //! \brief Win32-specific process information.
    //!
    PROCESS_INFORMATION m_process_information;

public: // XXX
    //!
    //! \brief Constructs a new Win32 child object representing a just
    //!        spawned child process.
    //!
    //! Creates a new child object that represents the process described by
    //! the \a si structure.
    //!
    //! The \a fhstdin, \a fhstdout and \a fhstderr parameters hold the
    //! communication streams used to interact with the child process if
    //! the launcher configured redirections.  See the parent class'
    //! constructor for more details on these.
    //!
    //! This constructor is protected because the library user has no
    //! business in creating representations of live processes himself;
    //! the library takes care of that in all cases.
    //!
    win32_child(const PROCESS_INFORMATION& pi,
                detail::file_handle fhstdin,
                detail::file_handle fhstdout,
                detail::file_handle fhstderr);

public:
    //!
    //! \brief Returns the process' handle.
    //!
    //! Returns a process-specific handle that can be used to access the
    //! process.  This is the value of the \a hProcess field in the
    //! PROCESS_INFORMATION structure returned by CreateProcess().
    //!
    //! \see get_id()
    //!
    HANDLE get_handle(void) const;

    //!
    //! \brief Returns the primary thread's handle.
    //!
    //! Returns a handle to the primary thread of the new process.  This is
    //! the value of the \a hThread field in the PROCESS_INFORMATION
    //! structure returned by CreateProcess().
    //!
    //! \see get_primary_thread_id()
    //!
    HANDLE get_primary_thread_handle(void) const;

    //!
    //! \brief Returns the primary thread's identifier.
    //!
    //! Returns a system-wide value that identifies the process's primary
    //! thread.  This is the value of the \a dwThreadId field in the
    //! PROCESS_INFORMATION structure returned by CreateProcess().
    //!
    //! \see get_primary_thread_handle()
    //!
    DWORD get_primary_thread_id(void) const;
};

// ------------------------------------------------------------------------

inline
win32_child::win32_child(const PROCESS_INFORMATION& pi,
                         detail::file_handle fhstdin,
                         detail::file_handle fhstdout,
                         detail::file_handle fhstderr) :
    child(pi.dwProcessId, fhstdin, fhstdout, fhstderr),
    m_process_information(pi)
{
}

// ------------------------------------------------------------------------

inline
HANDLE
win32_child::get_handle(void)
    const
{
    return m_process_information.hProcess;
}

// ------------------------------------------------------------------------

inline
HANDLE
win32_child::get_primary_thread_handle(void)
    const
{
    return m_process_information.hThread;
}

// ------------------------------------------------------------------------

inline
DWORD
win32_child::get_primary_thread_id(void)
    const
{
    return m_process_information.dwThreadId;
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_WIN32_CHILD_HPP)
