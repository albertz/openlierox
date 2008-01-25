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
//! \file boost/process/detail/pipe.hpp
//!
//! Includes the declaration of the pipe class.  This file is for
//! internal usage only and must not be included by the library user.
//!

#if !defined(BOOST_PROCESS_DETAIL_PIPE_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_PIPE_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
#   include <unistd.h>
}
#   include <cerrno>
#elif defined(BOOST_PROCESS_WIN32_API)
extern "C" {
#   include <windows.h>
}
#else
#   error "Unsupported platform."
#endif

#include <boost/process/detail/file_handle.hpp>
#include <boost/process/exceptions.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief Simple RAII model for anonymous pipes.
//!
//! The pipe class is a simple RAII model for anonymous pipes.  It
//! provides a portable constructor that allocates a new %pipe and creates
//! a pipe object that owns the two file handles associated to it: the
//! read end and the write end.
//!
//! These handles can be retrieved for modification according to
//! file_handle semantics.  Optionally, their ownership can be transferred
//! to external \a file_handle objects which comes handy when the two
//! ends need to be used in different places (i.e. after a POSIX fork()
//! system call).
//!
//! Pipes can be copied following the same semantics as file handles.
//! In other words, copying a %pipe object invalidates the source one.
//!
//! \see file_handle
//!
class pipe
{
    //!
    //! \brief The %pipe's read end file handle.
    //!
    file_handle m_read_end;

    //!
    //! \brief The %pipe's write end file handle.
    //!
    file_handle m_write_end;

public:
    //!
    //! \brief Creates a new %pipe.
    //!
    //! The default pipe constructor allocates a new anonymous %pipe
    //! and assigns its ownership to the created pipe object.
    //!
    //! \throw system_error If the anonymous %pipe creation fails.
    //!
    pipe(void);

    //!
    //! \brief Returns the %pipe's read end file handle.
    //!
    //! Obtains a reference to the %pipe's read end file handle.  Care
    //! should be taken to not duplicate the returned object if ownership
    //! shall remain to the %pipe.
    //!
    //! Duplicating the returned object invalidates its corresponding file
    //! handle in the %pipe.
    //!
    //! \return A reference to the %pipe's read end file handle.
    //!
    file_handle& rend(void);

    //!
    //! \brief Returns the %pipe's write end file handle.
    //!
    //! Obtains a reference to the %pipe's write end file handle.  Care
    //! should be taken to not duplicate the returned object if ownership
    //! shall remain to the %pipe.
    //!
    //! Duplicating the returned object invalidates its corresponding file
    //! handle in the %pipe.
    //!
    //! \return A reference to the %pipe's write end file handle.
    //!
    file_handle& wend(void);
};

// ------------------------------------------------------------------------

inline
pipe::pipe(void)
{
    file_handle::handle_type hs[2];

#if defined(BOOST_PROCESS_WIN32_API)
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    if (!::CreatePipe(&hs[0], &hs[1], &sa, 0))
        boost::throw_exception
            (system_error("boost::process::detail::pipe::pipe",
                          "CreatePipe failed", ::GetLastError()));
#else
    if (::pipe(hs) == -1)
        boost::throw_exception
            (system_error("boost::process::detail::pipe::pipe",
                          "pipe(2) failed", errno));
#endif

    m_read_end = file_handle(hs[0]);
    m_write_end = file_handle(hs[1]);
}

// ------------------------------------------------------------------------

inline
file_handle&
pipe::rend(void)
{
    return m_read_end;
}

// ------------------------------------------------------------------------

inline
file_handle&
pipe::wend(void)
{
    return m_write_end;
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_PIPE_HPP)
