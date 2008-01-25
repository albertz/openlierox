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
//! \file boost/process/detail/stream_info.hpp
//!
//! Provides the definition of the stream_info structure.
//!

#if !defined(BOOST_PROCESS_DETAIL_STREAM_INFO_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_STREAM_INFO_HPP
/** \endcond */

#include <string>

#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/process/detail/file_handle.hpp>
#include <boost/process/detail/pipe.hpp>
#include <boost/process/stream_behavior.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief Configuration data for a file descriptor.
//!
//! This convenience structure provides a compact way to pass information
//! around on how to configure a file descriptor.  It is a lower-level
//! representation of stream_behavior, as it can hold the same information
//! but in a way that can be used by the underlying operating system.
//!
struct stream_info
{
    enum type {
        //!
        //! \brief Matches stream_behavior::close.
        //!
        close,

        //!
        //! \brief Matches stream_behavior::inherit.
        //!
        inherit,

        //!
        //! \brief Matches stream_behavior::redirect_to_stdout and
        //! stream_behavior::posix_redirect.
        //!
        redirect,

        //!
        //! \brief Matches stream_behavior::silence.
        //!
        use_file,

        //!
        //! \brief Matches NOTHING YET.  XXX.
        //!
        use_handle,

        //!
        //! \brief Matches stream_behavior::capture.
        //!
        use_pipe
    };
    enum type m_type;

    // Valid when m_type == redirect.
    int m_desc_to;

    // Valid when m_type == use_file.
    std::string m_file;

    // Valid when m_type == use_handle.
    file_handle m_handle;

    // Valid when m_type == use_pipe.
    boost::optional< pipe > m_pipe;

    stream_info(const stream_behavior& sb, bool out);
};

// ------------------------------------------------------------------------

inline
stream_info::stream_info(const stream_behavior& sb, bool out)
{
    switch (sb.m_type) {
    case stream_behavior::capture:
        m_type = use_pipe;
        m_pipe = pipe();
        break;

    case stream_behavior::close:
        m_type = close;
        break;

    case stream_behavior::inherit:
        m_type = inherit;
        break;

#if defined(BOOST_PROCESS_POSIX_API)
    case stream_behavior::posix_redirect:
        m_type = redirect;
        m_desc_to = sb.m_desc_to;
        break;
#endif

    case stream_behavior::redirect_to_stdout:
        m_type = redirect;
#if defined(BOOST_PROCESS_POSIX_API)
        m_desc_to = STDOUT_FILENO;
#elif defined(BOOST_PROCESS_WIN32_API)
        m_desc_to = 1;
#endif
        break;

    case stream_behavior::silence:
        m_type = use_file;
#if defined(BOOST_PROCESS_POSIX_API)
        m_file = out ? "/dev/null" : "/dev/zero";
#elif defined(BOOST_PROCESS_WIN32_API)
        m_file = "NUL";
#endif
        break;

    default:
        BOOST_ASSERT(false);
    }
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_STREAM_INFO_HPP)
