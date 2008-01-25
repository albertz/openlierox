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
//! \file boost/process/win32_operations.hpp
//!
//! Provides miscellaneous free functions specific to Win32 operating
//! systems.
//!

#if !defined(BOOST_PROCESS_WIN32_OPERATIONS_HPP)
/** \cond */
#define BOOST_PROCESS_WIN32_OPERATIONS_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_WIN32_API)
#   error "Unsupported platform."
#endif

extern "C" {
#include <windows.h>
}

#include <boost/process/detail/file_handle.hpp>
#include <boost/process/detail/stream_info.hpp>
#include <boost/process/detail/win32_ops.hpp>
#include <boost/process/win32_child.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

template< class Executable, class Arguments, class Win32_Context >
inline
win32_child
win32_launch(const Executable& exe,
             const Arguments& args,
             const Win32_Context& ctx)
{
    using detail::stream_info;

    detail::file_handle fhstdin, fhstdout, fhstderr;

    stream_info behin = stream_info(ctx.m_stdin_behavior, false);
    if (behin.m_type == stream_info::use_pipe)
        fhstdin = behin.m_pipe->wend();
    stream_info behout = stream_info(ctx.m_stdout_behavior, true);
    if (behout.m_type == stream_info::use_pipe)
        fhstdout = behout.m_pipe->rend();
    stream_info beherr = stream_info(ctx.m_stderr_behavior, true);
    if (beherr.m_type == stream_info::use_pipe)
        fhstderr = beherr.m_pipe->rend();

    detail::win32_setup s;
    s.m_work_directory = ctx.m_work_directory;

    STARTUPINFO sitmp;
    if (ctx.m_startupinfo == NULL) {
        ZeroMemory(&sitmp, sizeof(sitmp));
        sitmp.cb = sizeof(sitmp);
        s.m_startupinfo = &sitmp;
    } else
        s.m_startupinfo = ctx.m_startupinfo;

    PROCESS_INFORMATION pi =
        detail::win32_start(exe, args, ctx.m_environment,
                            behin, behout, beherr, s);

    return win32_child(pi, fhstdin, fhstdout, fhstderr);
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_WIN32_OPERATIONS_HPP)
