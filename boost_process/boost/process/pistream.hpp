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
//! \file boost/process/pistream.hpp
//!
//! Includes the declaration of the pistream class.
//!

#if !defined(BOOST_PROCESS_PISTREAM_HPP)
/** \cond */
#define BOOST_PROCESS_PISTREAM_HPP
/** \endcond */

#include <istream>

#include <boost/noncopyable.hpp>
#include <boost/process/detail/file_handle.hpp>
#include <boost/process/detail/systembuf.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief Child process' output stream.
//!
//! The pistream class represents an output communication channel with the
//! child process.  The child process writes data to this stream and the
//! parent process can read it through the pistream object.  In other
//! words, from the child's point of view, the communication channel is an
//! output one, but from the parent's point of view it is an input one;
//! hence the confusing pistream name.
//!
//! pistream objects cannot be copied because they own the file handle
//! they use to communicate with the child and because they buffer data
//! that flows through the communication channel.
//!
//! A pistream object behaves as a std::istream stream in all senses.
//! The class is only provided because it must provide a method to let
//! the caller explicitly close the communication channel.
//!
//! \remark <b>Blocking remarks</b>: Functions that read data from this
//! stream can block if the associated file handle blocks during the read.
//! As this class is used to communicate with child processes through
//! anonymous pipes, the most typical blocking condition happens when the
//! child has no more data to send to the pipe's system buffer.  When
//! this happens, the buffer eventually empties and the system blocks
//! until the writer generates some data.
//!
class pistream :
    public std::istream, boost::noncopyable
{
    //!
    //! \brief The file handle managed by this stream.
    //!
    detail::file_handle m_handle;

    //!
    //! \brief The systembuf object used to manage this stream's data.
    //!
    detail::systembuf m_systembuf;

public:
    //!
    //! \brief Creates a new process' output stream.
    //!
    //! Given a file handle, this constructor creates a new pistream
    //! object that owns the given file handle \a fh.  Ownership of
    //! \a fh is transferred to the created pistream object.
    //!
    //! \pre \a fh is valid.
    //! \post \a fh is invalid.
    //! \post The new pistream object owns \a fh.
    //!
    explicit pistream(detail::file_handle& fh);

    //!
    //! \brief Closes the file handle managed by this stream.
    //!
    //! Explicitly closes the file handle managed by this stream.  This
    //! function can be used by the user to tell the child process it's
    //! not willing to receive more data.
    //!
    void close(void);
};

// ------------------------------------------------------------------------

inline
pistream::pistream(detail::file_handle& fh) :
    std::istream(NULL),
    m_handle(fh),
    m_systembuf(m_handle.get())
{
    rdbuf(&m_systembuf);
}

// ------------------------------------------------------------------------

inline
void
pistream::close(void)
{
    m_handle.close();
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_PISTREAM_HPP)
