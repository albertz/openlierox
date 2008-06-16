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
//! \file boost/process/detail/systembuf.hpp
//!
//! Includes the declaration of the systembuf class.  This file is for
//! internal usage only and must not be included by the library user.
//!

#if !defined(BOOST_PROCESS_DETAIL_SYSTEMBUF_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_SYSTEMBUF_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if defined(BOOST_PROCESS_POSIX_API)
extern "C" {
#   include <unistd.h>
}
#elif defined(BOOST_PROCESS_WIN32_API)
extern "C" {
#   include <windows.h>
}
#else
#   error "Unsupported platform."
#endif

#include <streambuf>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief std::streambuf implementation for system file handles.
//!
//! systembuf provides a std::streambuf implementation for system file
//! handles.  Contrarywise to file_handle, this class does \b not take
//! ownership of the native file handle; this should be taken care of
//! somewhere else.
//!
//! This class follows the expected semantics of a std::streambuf object.
//! However, it is not copyable to avoid introducing inconsistences with
//! the on-disk file and the in-memory buffers.
//!
class systembuf :
    public std::streambuf, boost::noncopyable
{
public:
#if defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Opaque name for the native handle type.
    //!
    typedef NativeHandleType handle_type;
#elif defined(BOOST_PROCESS_POSIX_API)
    typedef int handle_type;
#elif defined(BOOST_PROCESS_WIN32_API)
    typedef HANDLE handle_type;
#endif

    //!
    //! \brief Constructs a new systembuf for the given file handle.
    //!
    //! This constructor creates a new systembuf object that reads or
    //! writes data from/to the \a h native file handle.  This handle
    //! is \b not owned by the created systembuf object; the code
    //! should take care of it externally.
    //!
    //! This class buffers input and output; the buffer size may be
    //! tuned through the \a bufsize parameter, which defaults to 8192
    //! bytes.
    //!
    //! \see pistream and postream.
    //!
    explicit systembuf(handle_type h, std::size_t bufsize = 8192);

private:
    //!
    //! \brief Native file handle used by the systembuf object.
    //!
    handle_type m_handle;

    //!
    //! \brief Internal buffer size used during read and write operations.
    //!
    std::size_t m_bufsize;

    //!
    //! \brief Internal buffer used during read operations.
    //!
    boost::scoped_array< char > m_read_buf;

    //!
    //! \brief Internal buffer used during write operations.
    //!
    boost::scoped_array< char > m_write_buf;

protected:
    //!
    //! \brief Reads new data from the native file handle.
    //!
    //! This operation is called by input methods when there are no more
    //! data in the input buffer.  The function fills the buffer with new
    //! data, if available.
    //!
    //! \pre All input positions are exhausted (gptr() >= egptr()).
    //! \post The input buffer has new data, if available.
    //! \returns traits_type::eof() if a read error occurrs or there are
    //!          no more data to be read.  Otherwise returns
    //!          traits_type::to_int_type(*gptr()).
    //!
    virtual int_type underflow(void);

    //!
    //! \brief Makes room in the write buffer for additional data.
    //!
    //! This operation is called by output methods when there is no more
    //! space in the output buffer to hold a new element.  The function
    //! first flushes the buffer's contents to disk and then clears it to
    //! leave room for more characters.  The given \a c character is
    //! stored at the beginning of the new space.
    //!
    //! \pre All output positions are exhausted (pptr() >= epptr()).
    //! \post The output buffer has more space if no errors occurred
    //!       during the write to disk.
    //! \post *(pptr() - 1) is \a c.
    //! \returns traits_type::eof() if a write error occurrs.  Otherwise
    //!          returns traits_type::not_eof(c).
    //!
    virtual int_type overflow(int c);

    //!
    //! \brief Flushes the output buffer to disk.
    //!
    //! Synchronizes the systembuf buffers with the contents of the file
    //! associated to this object through the native file handle.  The
    //! output buffer is flushed to disk and cleared to leave new room
    //! for more data.
    //!
    //! \returns 0 on success, -1 if an error occurred.
    //!
    virtual int sync(void);
};

// ------------------------------------------------------------------------

inline
systembuf::systembuf(handle_type h, std::size_t bufsize) :
    m_handle(h),
    m_bufsize(bufsize),
    m_read_buf(new char[bufsize]),
    m_write_buf(new char[bufsize])
{
#if defined(BOOST_PROCESS_WIN32_API)
    BOOST_ASSERT(m_handle != INVALID_HANDLE_VALUE);
#else
    BOOST_ASSERT(m_handle >= 0);
#endif
    BOOST_ASSERT(m_bufsize > 0);

    setp(m_write_buf.get(), m_write_buf.get() + m_bufsize);
}

// ------------------------------------------------------------------------

inline
systembuf::int_type
systembuf::underflow(void)
{
    BOOST_ASSERT(gptr() >= egptr());

    bool ok;
#if defined(BOOST_PROCESS_WIN32_API)
    DWORD cnt;
    BOOL res = ::ReadFile(m_handle, m_read_buf.get(), m_bufsize, &cnt, NULL);
    ok = (res && cnt > 0);
#else
    ssize_t cnt = ::read(m_handle, m_read_buf.get(), m_bufsize);
    ok = (cnt != -1 && cnt != 0);
#endif

    if (!ok)
        return traits_type::eof();
    else {
        setg(m_read_buf.get(), m_read_buf.get(), m_read_buf.get() + cnt);
        return traits_type::to_int_type(*gptr());
    }
}

// ------------------------------------------------------------------------

inline
systembuf::int_type
systembuf::overflow(int c)
{
    BOOST_ASSERT(pptr() >= epptr());
    if (sync() == -1)
        return traits_type::eof();
    if (!traits_type::eq_int_type(c, traits_type::eof())) {
        traits_type::assign(*pptr(), c);
        pbump(1);
    }
    return traits_type::not_eof(c);
}

// ------------------------------------------------------------------------

inline
int
systembuf::sync(void)
{
#if defined(BOOST_PROCESS_WIN32_API)
    long cnt = pptr() - pbase();
#else
    ssize_t cnt = pptr() - pbase();
#endif

    bool ok;
#if defined(BOOST_PROCESS_WIN32_API)
    DWORD rcnt;
    BOOL res = ::WriteFile(m_handle, pbase(), cnt, &rcnt, NULL);
    ok = (res && rcnt == cnt);
#else
    ok = ::write(m_handle, pbase(), cnt) == cnt;
#endif

    if (ok)
        pbump(-cnt);
    return ok ? 0 : -1;
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_SYSTEMBUF_HPP)
