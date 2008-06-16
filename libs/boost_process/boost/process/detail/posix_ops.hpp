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
//! \file boost/process/detail/posix_ops.hpp
//!
//! Provides some convenience functions to start processes under POSIX
//! operating systems.
//!

#if !defined(BOOST_PROCESS_DETAIL_POSIX_OPS_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_POSIX_OPS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_POSIX_API)
#   error "Unsupported platform."
#endif

extern "C" {
#   include <fcntl.h>
#   include <unistd.h>
}

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <utility>

#include <boost/optional.hpp>
#include <boost/process/detail/file_handle.hpp>
#include <boost/process/detail/pipe.hpp>
#include <boost/process/detail/stream_info.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/exceptions.hpp>
#include <boost/process/stream_behavior.hpp>
#include <boost/scoped_array.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief Converts the command line to an array of C strings.
//!
//! Converts the command line's list of arguments to the format expected
//! by the \a argv parameter in the POSIX execve() system call.
//!
//! This operation is only available in POSIX systems.
//!
//! \return The first argument of the pair is an integer that indicates
//!         how many strings are stored in the second argument.  The
//!         second argument is a NULL-terminated, dynamically allocated
//!         vector of dynamically allocated strings holding the arguments
//!         to the executable.  The caller is responsible of freeing them.
//!
template< class Arguments >
inline
std::pair< std::size_t, char** >
collection_to_posix_argv(const Arguments& args)
{
    std::size_t nargs = args.size();
    BOOST_ASSERT(nargs > 0);

    char** argv = new char*[nargs + 1];
    typename Arguments::size_type i = 0;
    for (typename Arguments::const_iterator iter = args.begin();
         iter != args.end(); iter++) {
        argv[i] = ::strdup((*iter).c_str());
        i++;
    }
    argv[nargs] = NULL;

    return std::pair< std::size_t, char ** >(nargs, argv);
}

// ------------------------------------------------------------------------

//!
//! \brief Converts an environment to a char** table as used by execve().
//!
//! Converts the environment's contents to the format used by the
//! execve() system call.  The returned char** array is allocated
//! in dynamic memory and the caller must free it when not used any
//! more.  Each entry is also allocated in dynamic memory and is a
//! null-terminated string of the form var=value; these must also be
//! released by the caller.
//!
inline
char**
environment_to_envp(const environment& env)
{
    char** ep = new char*[env.size() + 1];

    environment::size_type i = 0;
    for (environment::const_iterator iter = env.begin();
         iter != env.end(); iter++) {
        std::string tmp = (*iter).first + "=" + (*iter).second;

        char* cstr = new char[tmp.length() + 1];
        std::strncpy(cstr, tmp.c_str(), tmp.length());
        cstr[tmp.length()] = '\0';

        ep[i++] = cstr;
    }

    ep[i] = NULL;

    return ep;
}

// ------------------------------------------------------------------------

//!
//! Holds a mapping between native file descriptors and their corresponding
//! pipes to set up communication between the parent and the %child process.
//!
typedef std::map< int, stream_info > info_map;

// ------------------------------------------------------------------------

//!
//! \brief Helper class to configure a POSIX %child.
//!
//! This helper class is used to hold all the attributes that configure a
//! new POSIX %child process and to centralize all the actions needed to
//! make them effective.
//!
//! All its fields are public for simplicity.  It is only intended for
//! internal use and it is heavily coupled with the Launcher
//! implementations.
//!
struct posix_setup
{
    //!
    //! \brief The work directory.
    //!
    //! This string specifies the directory in which the %child process
    //! starts execution.  It cannot be empty.
    //!
    std::string m_work_directory;

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
    std::string m_chroot;

    //!
    //! \brief Creates a new properties set.
    //!
    //! Creates a new object that has sensible default values for all the
    //! properties.
    //!
    posix_setup(void);

    //!
    //! \brief Sets up the execution environment.
    //!
    //! Modifies the current execution environment (that of the %child) so
    //! that the properties become effective.
    //!
    //! \throw system_error If any error ocurred during environment
    //!                     configuration.  The child process should abort
    //!                     execution if this happens because its start
    //!                     conditions cannot be met.
    //!
    void operator()(void) const;
};

// ------------------------------------------------------------------------

inline
posix_setup::posix_setup(void) :
    m_uid(::getuid()),
    m_euid(::geteuid()),
    m_gid(::getgid()),
    m_egid(::getegid())
{
}

// ------------------------------------------------------------------------

inline
void
posix_setup::operator()(void)
    const
{
    if (!m_chroot.empty()) {
        if (::chroot(m_chroot.c_str()) == -1)
            boost::throw_exception
                (system_error("boost::process::detail::posix_setup",
                              "chroot(2) failed", errno));
    }

    if (m_gid != ::getgid()) {
        if (::setgid(m_gid) == -1)
            boost::throw_exception
                (system_error("boost::process::detail::posix_setup",
                              "setgid(2) failed", errno));
    }

    if (m_egid != ::getegid()) {
        if (::setegid(m_egid) == -1)
            boost::throw_exception
                (system_error("boost::process::detail::posix_setup",
                              "setegid(2) failed", errno));
    }

    if (m_uid != ::getuid()) {
        if (::setuid(m_uid) == -1)
            boost::throw_exception
                (system_error("boost::process::detail::posix_setup",
                              "setuid(2) failed", errno));
    }

    if (m_euid != ::geteuid()) {
        if (::seteuid(m_euid) == -1)
            boost::throw_exception
                (system_error("boost::process::detail::posix_setup",
                              "seteuid(2) failed", errno));
    }

    BOOST_ASSERT(m_work_directory != "");
    if (chdir(m_work_directory.c_str()) == -1)
        boost::throw_exception
            (system_error("boost::process::detail::posix_setup",
                          "chdir(2) failed", errno));
}

// ------------------------------------------------------------------------

//!
//! \brief Configures child process' input streams.
//!
//! Sets up the current process' input streams to behave according to the
//! information in the \a info map.  \a closeflags is modified to reflect
//! those descriptors that should not be closed because they where modified
//! by the function.
//!
//! Modifies the current execution environment, so this should only be run
//! on the child process after the fork(2) has happened.
//!
//! \throw system_error If any error occurs during the configuration.
//!
inline
void
setup_input(info_map& info, bool* closeflags, int maxdescs)
{
    for (info_map::iterator iter = info.begin(); iter != info.end(); iter++) {
        int d = (*iter).first;
        stream_info& si = (*iter).second;

        BOOST_ASSERT(d < maxdescs);
        closeflags[d] = false;

        if (si.m_type == stream_info::use_file) {
            int fd = ::open(si.m_file.c_str(), O_RDONLY);
            if (fd == -1)
                boost::throw_exception
                    (system_error("boost::process::detail::setup_input",
                                  "open(2) of " + si.m_file + " failed",
                                  errno));
            if (fd != d) {
                file_handle h(fd);
                h.posix_remap(d);
                h.disown();
            }
        } else if (si.m_type == stream_info::use_handle) {
            if (si.m_handle.get() != d)
                si.m_handle.posix_remap(d);
        } else if (si.m_type == stream_info::use_pipe) {
            si.m_pipe->wend().close();
            if (d != si.m_pipe->rend().get())
                si.m_pipe->rend().posix_remap(d);
        } else
            BOOST_ASSERT(si.m_type == stream_info::inherit);
    }
}

// ------------------------------------------------------------------------

//!
//! \brief Configures child process' output streams.
//!
//! Sets up the current process' output streams to behave according to the
//! information in the \a info map.  \a closeflags is modified to reflect
//! those descriptors that should not be closed because they where modified
//! by the function.
//!
//! Modifies the current execution environment, so this should only be run
//! on the child process after the fork(2) has happened.
//!
//! \throw system_error If any error occurs during the configuration.
//!
inline
void
setup_output(info_map& info, bool* closeflags, int maxdescs)
{
    for (info_map::iterator iter = info.begin(); iter != info.end(); iter++) {
        int d = (*iter).first;
        stream_info& si = (*iter).second;

        BOOST_ASSERT(d < maxdescs);
        closeflags[d] = false;

        if (si.m_type == stream_info::redirect) {
            // Handled once all other descriptors have been configured.
            // XXX This is most likely a hack begging for problems.
            // Should replace info_map with another collection that
            // respects ordering so that the user can control this.
        } else if (si.m_type == stream_info::use_file) {
            int fd = ::open(si.m_file.c_str(), O_WRONLY);
            if (fd == -1)
                boost::throw_exception
                    (system_error("boost::process::detail::setup_output",
                                  "open(2) of " + si.m_file + " failed",
                                  errno));
            if (fd != d) {
                file_handle h(fd);
                h.posix_remap(d);
                h.disown();
            }
        } else if (si.m_type == stream_info::use_handle) {
            if (si.m_handle.get() != d)
                si.m_handle.posix_remap(d);
        } else if (si.m_type == stream_info::use_pipe) {
            si.m_pipe->rend().close();
            if (d != si.m_pipe->wend().get())
                si.m_pipe->wend().posix_remap(d);
        } else
            BOOST_ASSERT(si.m_type == stream_info::inherit);
    }

    for (info_map::iterator iter = info.begin(); iter != info.end(); iter++) {
        int d = (*iter).first;
        stream_info& si = (*iter).second;

        if (si.m_type == stream_info::redirect)
            file_handle::posix_dup(si.m_desc_to, d).disown();
    }
}

// ------------------------------------------------------------------------

//!
//! \brief Starts a new child process in a POSIX operating system.
//!
//! This helper functions is provided to simplify the Launcher's task when
//! it comes to starting up a new process in a POSIX operating system.
//! The function hides all the details of the fork/exec pair of calls as
//! well as all the setup of communication pipes and execution environment.
//!
//! \param cl The command line used to execute the child process.
//! \param env The environment variables that the new child process
//!            receives.
//! \param infoin A map describing all input file descriptors to be
//!               redirected.
//! \param infoout A map describing all output file descriptors to be
//!                redirected.
//! \param setup A helper object used to configure the child's execution
//!              environment.
//! \return The new process' PID.  The caller is responsible of creating
//!         an appropriate Child representation for it.
//!
template< class Executable, class Arguments >
inline
pid_t
posix_start(const Executable& exe,
            const Arguments& args,
            const environment& env,
            info_map& infoin,
            info_map& infoout,
            const posix_setup& setup)
{
    pid_t pid = ::fork();
    if (pid == -1) {
        boost::throw_exception
            (system_error("boost::process::detail::posix_start",
                          "fork(2) failed", errno));
    } else if (pid == 0) {
#if defined(F_MAXFD)
        int maxdescs = std::max(::fcntl(0, F_MAXFD), 128); // XXX
#else
        int maxdescs = 128; // XXX
#endif
        try {
            boost::scoped_array< bool > closeflags(new bool[maxdescs]);
            for (int i = 0; i < maxdescs; i++)
                closeflags.get()[i] = true;

            setup_input(infoin, closeflags.get(), maxdescs);
            setup_output(infoout, closeflags.get(), maxdescs);

            for (int i = 0; i < maxdescs; i++)
                if (closeflags.get()[i])
                    ::close(i);

            setup();
        } catch (const system_error& e) {
            ::write(STDERR_FILENO, e.what(), std::strlen(e.what()));
            ::write(STDERR_FILENO, "\n", 1);
            ::exit(EXIT_FAILURE);
        }

        std::pair< std::size_t, char** > argcv =
            collection_to_posix_argv(args);
        char** envp = environment_to_envp(env);
        ::execve(exe.c_str(), argcv.second, envp);
        system_error e("boost::process::detail::posix_start",
                       "execve(2) failed", errno);

        for (std::size_t i = 0; i < argcv.first; i++)
            delete [] argcv.second[i];
        delete [] argcv.second;

        for (std::size_t i = 0; i < env.size(); i++)
            delete [] envp[i];
        delete [] envp;

        ::write(STDERR_FILENO, e.what(), std::strlen(e.what()));
        ::write(STDERR_FILENO, "\n", 1);
        ::exit(EXIT_FAILURE);
    }

    BOOST_ASSERT(pid > 0);

    for (info_map::iterator iter = infoin.begin();
         iter != infoin.end(); iter++) {
        stream_info& si = (*iter).second;

        if (si.m_type == stream_info::use_pipe)
            si.m_pipe->rend().close();
    }

    for (info_map::iterator iter = infoout.begin();
         iter != infoout.end(); iter++) {
        stream_info& si = (*iter).second;

        if (si.m_type == stream_info::use_pipe)
            si.m_pipe->wend().close();
    }

    return pid;
}

// ------------------------------------------------------------------------

//!
//! \brief Locates a communication pipe and returns one of its endpoints.
//!
//! Given a \a info map, and a file descriptor \a desc, searches for its
//! communicataion pipe in the map and returns one of its endpoins as
//! indicated by the \a out flag.  This is intended to be used by a
//! parent process after a fork(2) call.
//!
//! \pre If the info map contains the given descriptor, it is configured
//!      to use a pipe.
//! \post The info map does not contain the given descriptor.
//! \return If the file descriptor is found in the map, returns the pipe's
//!         read end if out is true; otherwise its write end.  If the
//!         descriptor is not found returns an invalid file handle.
//!
inline
file_handle
posix_info_locate_pipe(info_map& info, int desc, bool out)
{
    file_handle fh;

    info_map::iterator iter = info.find(desc);
    if (iter != info.end()) {
        BOOST_ASSERT(iter != info.end());
        stream_info& si = (*iter).second;
        // XXX This conditional was an assertion; should it be?
        if (si.m_type == stream_info::use_pipe) {
            fh = out ? si.m_pipe->rend().disown() : si.m_pipe->wend().disown();
            BOOST_ASSERT(fh.is_valid());
        }
        info.erase(iter);
    }

    return fh;
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_POSIX_OPS_HPP)
