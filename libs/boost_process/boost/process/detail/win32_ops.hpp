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
//! \file boost/process/detail/win32_ops.hpp
//!
//! Provides some convenience functions to start processes under Win32
//! operating systems.
//!

#if !defined(BOOST_PROCESS_DETAIL_WIN32_OPS_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_WIN32_OPS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_WIN32_API)
#   error "Unsupported platform."
#endif

extern "C" {
#include <tchar.h>
#include <windows.h>

// Added for OpenLieroX - Dev-Cpp workarounds
#if defined(WIN32) && defined(__GNUC__)
#define _tcscpy_s(D,N,S) _tcscpy(D,S)
#define _tcscat_s(D,N,S) _tcscat(D,S)
#endif

}

#include <boost/optional.hpp>
#include <boost/process/detail/file_handle.hpp>
#include <boost/process/detail/pipe.hpp>
#include <boost/process/detail/stream_info.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/exceptions.hpp>
#include <boost/process/stream_behavior.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief Converts the command line to a plain string.
//!
//! Converts the command line's list of arguments to the format
//! expected by the \a lpCommandLine parameter in the CreateProcess()
//! system call.
//!
//! This operation is only available in Win32 systems.
//!
//! \return A dynamically allocated string holding the command line
//!         to be passed to the executable.  It is returned in a
//!         shared_array object to ensure its release at some point.
//!
template< class Arguments >
inline
boost::shared_array< TCHAR >
collection_to_win32_cmdline(const Arguments& args)
{
    typedef std::vector< std::string > arguments_vector;
    Arguments args2;

    typename Arguments::size_type i = 0;
    std::size_t length = 0;
    for (typename Arguments::const_iterator iter = args.begin();
         iter != args.end(); iter++) {
        std::string arg = (*iter);

        std::string::size_type pos = 0;
        while ((pos = arg.find('"', pos)) != std::string::npos) {
            arg.replace(pos, 1, "\\\"");
            pos += 2;
        }

        if (arg.find(' ') != std::string::npos)
            arg = '\"' + arg + '\"';

        if (i != args.size() - 1)
            arg += ' ';

        args2.push_back(arg);
        length += arg.size() + 1;

        i++;
    }

    boost::shared_array< TCHAR > cmdline(new TCHAR[length]);
    ::_tcscpy_s(cmdline.get(), length, TEXT(""));
    for (arguments_vector::size_type i = 0; i < args2.size(); i++)
        ::_tcscat_s(cmdline.get(), length, TEXT(args2[i].c_str()));

    return cmdline;
}

// ------------------------------------------------------------------------

//!
//! \brief Converts an environment to a string used by CreateProcess().
//!
//! Converts the environment's contents to the format used by the
//! CreateProcess() system call.  The returned TCHAR* string is
//! allocated in dynamic memory and the caller must free it when not
//! used any more.  This is enforced by the use of a shared pointer.
//! The string is of the form var1=value1\\0var2=value2\\0\\0.
//!
inline
boost::shared_array< TCHAR >
environment_to_win32_strings(const environment& env)
{
    boost::shared_array< TCHAR > strs(NULL);

    // TODO: Add the "" variable to the returned string; it shouldn't
    // be in the environment if the user didn't add it.

    if (env.size() == 0) {
        strs.reset(new TCHAR[2]);
        ::ZeroMemory(strs.get(), sizeof(TCHAR) * 2);
    } else {
        std::string::size_type len = sizeof(TCHAR);
        for (environment::const_iterator iter = env.begin();
             iter != env.end(); iter++)
            len += ((*iter).first.length() + 1 + (*iter).second.length() +
                    1) * sizeof(TCHAR);

        strs.reset(new TCHAR[len]);

        TCHAR* ptr = strs.get();
        for (environment::const_iterator iter = env.begin();
             iter != env.end(); iter++) {
            std::string tmp = (*iter).first + "=" + (*iter).second;
            _tcscpy_s(ptr, len - (ptr - strs.get()) * sizeof(TCHAR),
                      TEXT(tmp.c_str()));
            ptr += (tmp.length() + 1) * sizeof(TCHAR);

            BOOST_ASSERT(static_cast< std::string::size_type >
                (ptr - strs.get()) * sizeof(TCHAR) < len);
        }
        *ptr = '\0';
    }

    BOOST_ASSERT(strs.get() != NULL);
    return strs;
}

// ------------------------------------------------------------------------

//!
//! \brief Helper class to configure a Win32 %child.
//!
//! This helper class is used to hold all the attributes that configure a
//! new Win32 %child process .
//!
//! All its fields are public for simplicity.  It is only intended for
//! internal use and it is heavily coupled with the Launcher
//! implementations.
//!
struct win32_setup
{
    //!
    //! \brief The work directory.
    //!
    //! This string specifies the directory in which the %child process
    //! starts execution.  It cannot be empty.
    //!
    std::string m_work_directory;

    //!
    //! \brief The process startup properties.
    //!
    //! This Win32-specific object holds a list of properties that describe
    //! how the new process should be started.  The STARTF_USESTDHANDLES
    //! flag should not be set in it because it is automatically configured
    //! by win32_start().
    //!
    STARTUPINFO* m_startupinfo;
};

// ------------------------------------------------------------------------

//!
//! \brief Starts a new child process in a Win32 operating system.
//!
//! This helper functions is provided to simplify the Launcher's task when
//! it comes to starting up a new process in a Win32 operating system.
//!
//! \param cl The command line used to execute the child process.
//! \param env The environment variables that the new child process
//!            receives.
//! \param infoin Information that describes stdin's behavior.
//! \param infoout Information that describes stdout's behavior.
//! \param infoerr Information that describes stderr's behavior.
//! \param setup A helper object holding extra child information.
//! \return The new process' information as returned by the ::CreateProcess
//!         system call.  The caller is responsible of creating an
//!         appropriate Child representation for it.
//! \pre \a setup.m_startupinfo cannot have the \a STARTF_USESTDHANDLES set
//!      in the \a dwFlags field.
//!
template< class Executable, class Arguments >
inline
PROCESS_INFORMATION
win32_start(const Executable& exe,
            const Arguments& args,
            const environment& env,
            stream_info& infoin,
            stream_info& infoout,
            stream_info& infoerr,
            const win32_setup& setup)
{
    file_handle chin, chout, cherr;

    BOOST_ASSERT(setup.m_startupinfo->cb >= sizeof(STARTUPINFO));
    BOOST_ASSERT(!(setup.m_startupinfo->dwFlags & STARTF_USESTDHANDLES));
    // XXX I'm not sure this usage of scoped_ptr is correct...
    boost::scoped_ptr< STARTUPINFO > si
        ((STARTUPINFO*)new char[setup.m_startupinfo->cb]);
    ::CopyMemory(si.get(), setup.m_startupinfo, setup.m_startupinfo->cb);
    si->dwFlags |= STARTF_USESTDHANDLES;

    if (infoin.m_type == stream_info::close) {
    } else if (infoin.m_type == stream_info::inherit) {
        chin = file_handle::win32_std(STD_INPUT_HANDLE, true);
    } else if (infoin.m_type == stream_info::use_file) {
        HANDLE h = ::CreateFile(TEXT(infoin.m_file.c_str()), GENERIC_READ,
                                0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h == INVALID_HANDLE_VALUE)
            boost::throw_exception
                (system_error("boost::process::detail::win32_start",
                              "CreateFile failed", ::GetLastError()));
        chin = file_handle(h);
    } else if (infoin.m_type == stream_info::use_handle) {
        chin = infoin.m_handle;
        chin.win32_set_inheritable(true);
    } else if (infoin.m_type == stream_info::use_pipe) {
        infoin.m_pipe->rend().win32_set_inheritable(true);
        chin = infoin.m_pipe->rend();
    } else
        BOOST_ASSERT(false);
    si->hStdInput = chin.is_valid() ? chin.get() : INVALID_HANDLE_VALUE;

    if (infoout.m_type == stream_info::close) {
    } else if (infoout.m_type == stream_info::inherit) {
        chout = file_handle::win32_std(STD_OUTPUT_HANDLE, true);
    } else if (infoout.m_type == stream_info::use_file) {
        HANDLE h = ::CreateFile(TEXT(infoout.m_file.c_str()), GENERIC_WRITE,
                                0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h == INVALID_HANDLE_VALUE)
            boost::throw_exception
                (system_error("boost::process::detail::win32_start",
                              "CreateFile failed", ::GetLastError()));
        chout = file_handle(h);
    } else if (infoout.m_type == stream_info::use_handle) {
        chout = infoout.m_handle;
        chout.win32_set_inheritable(true);
    } else if (infoout.m_type == stream_info::use_pipe) {
        infoout.m_pipe->wend().win32_set_inheritable(true);
        chout = infoout.m_pipe->wend();
    } else
        BOOST_ASSERT(false);
    si->hStdOutput = chout.is_valid() ? chout.get() : INVALID_HANDLE_VALUE;

    if (infoerr.m_type == stream_info::close) {
    } else if (infoerr.m_type == stream_info::inherit) {
        cherr = file_handle::win32_std(STD_ERROR_HANDLE, true);
    } else if (infoerr.m_type == stream_info::redirect) {
        BOOST_ASSERT(infoerr.m_desc_to == 1);
        BOOST_ASSERT(chout.is_valid());
        cherr = file_handle::win32_dup(chout.get(), true);
    } else if (infoerr.m_type == stream_info::use_file) {
        HANDLE h = ::CreateFile(TEXT(infoerr.m_file.c_str()), GENERIC_WRITE,
                                0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h == INVALID_HANDLE_VALUE)
            boost::throw_exception
                (system_error("boost::process::detail::win32_start",
                              "CreateFile failed", ::GetLastError()));
        cherr = file_handle(h);
    } else if (infoerr.m_type == stream_info::use_handle) {
        cherr = infoerr.m_handle;
        cherr.win32_set_inheritable(true);
    } else if (infoerr.m_type == stream_info::use_pipe) {
        infoerr.m_pipe->wend().win32_set_inheritable(true);
        cherr = infoerr.m_pipe->wend();
    } else
        BOOST_ASSERT(false);
    si->hStdError = cherr.is_valid() ? cherr.get() : INVALID_HANDLE_VALUE;

    PROCESS_INFORMATION pi;
    ::ZeroMemory(&pi, sizeof(pi));

    boost::shared_array< TCHAR > cmdline = collection_to_win32_cmdline(args);
    boost::scoped_array< TCHAR > executable(::_tcsdup(TEXT(exe.c_str())));
    boost::scoped_array< TCHAR > workdir
        (::_tcsdup(TEXT(setup.m_work_directory.c_str())));
    boost::shared_array< TCHAR > envstrs = environment_to_win32_strings(env);
    if (!::CreateProcess(executable.get(), cmdline.get(), NULL, NULL, TRUE,
                         0, envstrs.get(), workdir.get(),
                         si.get(), &pi)) {
        boost::throw_exception
            (system_error("boost::process::detail::win32_start",
                          "CreateProcess failed", ::GetLastError()));
    }

    return pi;
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_WIN32_OPS_HPP)
