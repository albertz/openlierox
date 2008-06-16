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
//! \file boost/process/posix_context.hpp
//!
//! Includes the declaration of the posix_context class.
//!

#if !defined(BOOST_PROCESS_POSIX_CONTEXT_HPP)
/** \cond */
#define BOOST_PROCESS_POSIX_CONTEXT_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_POSIX_API)
#   error "Unsupported platform."
#endif

#include <unistd.h>

#include <map>
#include <set>
#include <utility>

#include <boost/process/context.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/stream_behavior.hpp>

// XXX Merge the contents of this file with context.hpp and rename that
// to contexts.hpp?  This could remove the need for the user to mess
// around with preprocessor conditional to detect the current platform and
// would reduce the number of headers in the library, which is IMHO a good
// thing.

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! Holds a mapping between native file descriptors and their corresponding
//! pipes to set up communication between the parent and the %child process.
//!
typedef std::map< int, stream_behavior > behavior_map;

// ------------------------------------------------------------------------

template< class Path >
class posix_basic_context :
    public basic_work_directory_context< Path >,
    public environment_context
{
public:
    //!
    //! \brief Constructs a new POSIX-specific context.
    //!
    //! Constructs a new context.  It is configured as follows:
    //! * All communcation channels with the child process are closed.
    //! * There are no channel mergings.
    //! * The initial work directory of the child processes is set to the
    //!   current working directory.
    //! * The environment variables table is empty.
    //! * The credentials are the same as those of the current process.
    //!
    posix_basic_context(void);

    // XXX Add constructor to create a posix_basic_context from a
    // basic_context.

    //!
    //! \brief List of input streams that will be redirected.
    //!
    behavior_map m_input_behavior;

    //!
    //! \brief List of output streams that will be redirected.
    //!
    behavior_map m_output_behavior;

    //!
    //! \brief The user credentials.
    //!
    //! UID that specifies the user credentials to use to run the %child
    //! process.  Defaults to the current UID.
    //!
    uid_t m_uid;

    //!
    //! \brief The effective user credentials.
    //!
    //! EUID that specifies the effective user credentials to use to run
    //! the %child process.  Defaults to the current EUID.
    //!
    uid_t m_euid;

    //!
    //! \brief The group credentials.
    //!
    //! GID that specifies the group credentials to use to run the %child
    //! process.  Defaults to the current GID.
    //!
    uid_t m_gid;

    //!
    //! \brief The effective group credentials.
    //!
    //! EGID that specifies the effective group credentials to use to run
    //! the %child process.  Defaults to the current EGID.
    //!
    uid_t m_egid;

    //!
    //! \brief The chroot directory, if any.
    //!
    //! Specifies the directory in which the %child process is chrooted
    //! before execution.  Empty if this feature is not desired.
    //!
    Path m_chroot;
};

typedef posix_basic_context< std::string > posix_context;

// ------------------------------------------------------------------------

template< class Path >
inline
posix_basic_context< Path >::posix_basic_context(void) :
    m_uid(::getuid()),
    m_euid(::geteuid()),
    m_gid(::getgid()),
    m_egid(::getegid())
{
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_POSIX_CONTEXT_HPP)
