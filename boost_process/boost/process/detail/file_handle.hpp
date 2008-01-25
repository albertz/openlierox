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
//! \file boost/process/detail/file_handle.hpp
//!
//! Includes the declaration of the file_handle class.  This file is for
//! internal usage only and must not be included by the library user.
//!

#if !defined(BOOST_PROCESS_DETAIL_FILE_HANDLE_HPP)
/** \cond */
#define BOOST_PROCESS_DETAIL_FILE_HANDLE_HPP
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

#include <boost/assert.hpp>
#include <boost/process/exceptions.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace process {
namespace detail {

// ------------------------------------------------------------------------

//!
//! \brief Simple RAII model for system file handles.
//!
//! The \a file_handle class is a simple RAII model for native system file
//! handles.  This class wraps one of such handles grabbing its ownership,
//! and automaticaly closes it upon destruction.  It is basically used
//! inside the library to avoid leaking open file handles, shall an
//! unexpected execution trace occur.
//!
//! A \a file_handle object can be copied but doing so invalidates the
//! source object.  There can only be a single valid \a file_handle object
//! for a given system file handle.  This is similar to std::auto_ptr\<\>'s
//! semantics.
//!
//! This class also provides some convenience methods to issue special file
//! operations under their respective platforms.
//!
class file_handle
{
public:
#if defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Opaque name for the native handle type.
    //!
    //! Each operating system identifies file handles using a specific type.
    //! The \a handle_type type is used to transparently refer to file
    //! handles regarless of the operating system in which this class is
    //! used.
    //!
    //! If this class is used in a POSIX system, \a NativeSystemHandle is
    //! an integer type while it is a \a HANDLE in a Win32 system.
    //!
    typedef NativeSystemHandle handle_type;
#elif defined(BOOST_PROCESS_POSIX_API)
    typedef int handle_type;
#elif defined(BOOST_PROCESS_WIN32_API)
    typedef HANDLE handle_type;
#endif

    //!
    //! \brief Constructs an invalid file handle.
    //!
    //! This constructor creates a new \a file_handle object that represents
    //! an invalid file handle.  An invalid file handle can be copied but
    //! cannot be manipulated in any way (except checking for its validity).
    //!
    //! \see is_valid()
    //!
    file_handle(void);

    //!
    //! \brief Constructs a new file handle from a native file handle.
    //!
    //! This constructor creates a new \a file_handle object that takes
    //! ownership of the given \a h native file handle.  The user must not
    //! close \a h on his own during the lifetime of the new object.
    //! Ownership can be reclaimed using disown().
    //!
    //! \pre The native file handle must be valid; a close operation must
    //!      succeed on it.
    //!
    //! \see disown()
    //!
    file_handle(handle_type h);

    //!
    //! \brief Copy constructor; invalidates the source handle.
    //!
    //! This copy constructor creates a new file handle from a given one.
    //! Ownership of the native file handle is transferred to the new
    //! object, effectively invalidating the source file handle.  This
    //! avoids having two live \a file_handle objects referring to the
    //! same native file handle.  The source file handle need not be
    //! valid in the name of simplicity.
    //!
    //! \post The source file handle is invalid.
    //! \post The new file handle owns the source's native file handle.
    //!
    file_handle(const file_handle& fh);

    //!
    //! \brief Releases resources if the handle is valid.
    //!
    //! If the file handle is valid, the destructor closes it.
    //!
    //! \see is_valid()
    //!
    ~file_handle(void);

    //!
    //! \brief Assignment operator; invalidates the source handle.
    //!
    //! This assignment operator transfers ownership of the RHS file
    //! handle to the LHS one, effectively invalidating the source file
    //! handle.  This avoids having two live \a file_handle objects
    //! referring to the same native file handle.  The source file
    //! handle need not be valid in the name of simplicity.
    //!
    //! \post The RHS file handle is invalid.
    //! \post The LHS file handle owns RHS' native file handle.
    //! \return A reference to the LHS file handle.
    //!
    file_handle& operator=(const file_handle& fh);

    //!
    //! \brief Checks whether the file handle is valid or not.
    //!
    //! Returns a boolean indicating whether the file handle is valid or
    //! not.  If the file handle is invalid, no other applications can be
    //! executed other than the destructor.
    //!
    //! \return True if the file handle is valid; false otherwise.
    //!
    bool is_valid(void) const;

    //!
    //! \brief Closes the file handle.
    //!
    //! Explicitly closes the file handle, which must be valid.  Upon
    //! exit, the handle is not valid any more.
    //!
    //! \pre The file handle is valid.
    //! \post The file handle is invalid.
    //! \post The native file handle is closed.
    //!
    void close(void);

    //!
    //! \brief Reclaims ownership of the native file handle.
    //!
    //! Explicitly reclaims ownership of the native file handle contained
    //! in the \a file_handle object, returning the native file handle.
    //! The caller is responsible of closing it later on.
    //!
    //! \pre The file handle is valid.
    //! \post The file handle is invalid.
    //! \return The native file handle.
    //!
    handle_type disown(void);

    //!
    //! \brief Gets the native file handle.
    //!
    //! Returns the native file handle for the \a file_handle object.
    //! The caller can issue any operation on it except closing it.
    //! If closing is required, disown() shall be used.
    //!
    //! \pre The file handle is valid.
    //! \return The native file handle.
    //!
    handle_type get(void) const;

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Changes the native file handle to the given one.
    //!
    //! Given a new native file handle \a h, this operation assigns this
    //! handle to the current object, closing its old native file handle.
    //! In other words, it first calls dup2() to remap the old handle to
    //! the new one and then closes the old handle.
    //!
    //! If \a h is open, it is automatically closed by dup2().
    //!
    //! This operation is only available in POSIX systems.
    //!
    //! \pre The file handle is valid.
    //! \pre The native file handle \a h is valid; i.e., it must be
    //!      closeable.
    //! \post The file handle's native file handle is \a h.
    //! \throw system_error If the internal remapping operation fails.
    //!
    void posix_remap(handle_type h);
#endif

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Duplicates an open native file handle.
    //!
    //! Given a native file handle \a h1, this routine duplicates it so
    //! that it ends up being identified by the native file handle \a h2
    //! and returns a new \a file_handle owning \a h2.
    //!
    //! This operation is only available in POSIX systems.
    //!
    //! \pre The native file handle \a h1 is open.
    //! \pre The native file handle \a h2 is valid (non-negative).
    //! \post The native file handle \a h1 is closed.
    //! \post The native file handle \a h2 is the same as the old \a h1
    //!       from the operating system's point of view.
    //! \return A new \a file_handle object that owns \a h2.
    //! \throw system_error If dup2() fails.
    //!
    static file_handle posix_dup(int h1, int h2);
#endif

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Duplicates the \a h native file handle.
    //!
    //! Given a native file handle \a h, this routine constructs a new
    //! \a file_handle object that owns a new duplicate of \a h.  The
    //! duplicate's inheritable flag is set to the value of \a inheritable.
    //!
    //! This operation is only available in Win32 systems.
    //!
    //! \pre The native file handle \a h is valid.
    //! \return A file handle owning a duplicate of \a h.
    //! \throw system_error If DuplicateHandle() fails.
    //!
    static file_handle win32_dup(HANDLE h, bool inheritable);
#endif

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Creates a new duplicate of a standard file handle.
    //!
    //! Constructs a new \a file_handle object that owns a duplicate of a
    //! standard file handle.  The \a d parameter specifies which standard
    //! file handle to duplicate and can be one of \a STD_INPUT_HANDLE,
    //! \a STD_OUTPUT_HANDLE or \a STD_ERROR_HANDLE. The duplicate's
    //! inheritable flag is set to the value of \a inheritable.
    //!
    //! This operation is only available in Win32 systems.
    //!
    //! \pre \a d refers to one of the standard handles as described above.
    //! \return A file handle owning a duplicate of the standard handle
    //!         referred to by \a d.
    //! \throw system_error If GetStdHandle() or DuplicateHandle() fail.
    //!
    static file_handle win32_std(DWORD d, bool inheritable);
#endif

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
    //!
    //! \brief Changes the file handle's inheritable flag.
    //!
    //! Changes the file handle's inheritable flag to \a i.  It is not
    //! necessary for the file handle's flag to be different than \a i.
    //!
    //! This operation is only available in Win32 systems.
    //!
    //! \pre The file handle is valid.
    //! \post The native file handle's inheritable flag is set to \a i.
    //! \throw system_error If the property change fails.
    //!
    void win32_set_inheritable(bool i);
#endif

private:
    //!
    //! \brief Internal handle value.
    //!
    //! This variable holds the native handle value for the file handle
    //! hold by this object.  It is interesting to note that this needs
    //! to be mutable because the copy constructor and the assignment
    //! operator invalidate the source object.
    //!
    mutable handle_type m_handle;

    //!
    //! \brief Constant function representing an invalid handle value.
    //!
    //! Returns the platform-specific handle value that represents an
    //! invalid handle.  This is a constant function rather than a regular
    //! constant because, in the latter case, we cannot define it under
    //! Win32 due to the value being of a complex type.
    //!
    static const handle_type invalid_value(void);
};

// ------------------------------------------------------------------------

inline
file_handle::file_handle(void) :
    m_handle(invalid_value())
{
}

// ------------------------------------------------------------------------

inline
file_handle::file_handle(handle_type h) :
    m_handle(h)
{
    BOOST_ASSERT(m_handle != invalid_value());
}

// ------------------------------------------------------------------------

inline
file_handle::file_handle(const file_handle& fh) :
    m_handle(fh.m_handle)
{
    fh.m_handle = invalid_value();
}

// ------------------------------------------------------------------------

inline
file_handle::~file_handle(void)
{
    if (is_valid())
        close();
}

// ------------------------------------------------------------------------

inline
file_handle&
file_handle::operator=(const file_handle& fh)
{
    m_handle = fh.m_handle;
    fh.m_handle = invalid_value();

    return *this;
}

// ------------------------------------------------------------------------

inline
bool
file_handle::is_valid(void)
    const
{
    return m_handle != invalid_value();
}

// ------------------------------------------------------------------------

inline
void
file_handle::close(void)
{
    BOOST_ASSERT(is_valid());

#if defined(BOOST_PROCESS_POSIX_API)
    ::close(m_handle);
#elif defined(BOOST_PROCESS_WIN32_API)
    ::CloseHandle(m_handle);
#endif

    m_handle = invalid_value();
}

// ------------------------------------------------------------------------

inline
file_handle::handle_type
file_handle::disown(void)
{
    BOOST_ASSERT(is_valid());

    handle_type h = m_handle;
    m_handle = invalid_value();
    return h;
}

// ------------------------------------------------------------------------

inline
file_handle::handle_type
file_handle::get(void)
    const
{
    BOOST_ASSERT(is_valid());

    return m_handle;
}

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
inline
void
file_handle::posix_remap(handle_type h)
{
    BOOST_ASSERT(is_valid());

    if (::dup2(m_handle, h) == -1)
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::remap",
                          "dup2(2) failed", errno));

    if (::close(m_handle) == -1) {
        ::close(h);
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::remap",
                          "close(2) failed", errno));
    }

    m_handle = h;
}
#endif

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
inline
file_handle
file_handle::posix_dup(int h1, int h2)
{
    if (::dup2(h1, h2) == -1)
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::posix_dup",
                          "dup2(2) failed", errno));

    return file_handle(h2);
}
#endif

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
inline
file_handle
file_handle::win32_dup(HANDLE h, bool inheritable)
{
    HANDLE h2;
    if (::DuplicateHandle(::GetCurrentProcess(), h,
                          ::GetCurrentProcess(), &h2,
                          0, inheritable ? TRUE : FALSE,
                          DUPLICATE_SAME_ACCESS) == 0)
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::win32_dup",
                          "DuplicateHandle failed", ::GetLastError()));

    return file_handle(h2);
}
#endif

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
inline
file_handle
file_handle::win32_std(DWORD d, bool inheritable)
{
    BOOST_ASSERT(d == STD_INPUT_HANDLE ||
                 d == STD_OUTPUT_HANDLE ||
                 d == STD_ERROR_HANDLE);

    HANDLE h = ::GetStdHandle(d);
    if (h == INVALID_HANDLE_VALUE)
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::win32_std",
                          "GetStdHandle failed", ::GetLastError()));

    return win32_dup(h, inheritable);
}
#endif

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_WIN32_API) || defined(BOOST_PROCESS_DOXYGEN)
inline
void
file_handle::win32_set_inheritable(bool b)
{
    BOOST_ASSERT(is_valid());

    if (::SetHandleInformation(m_handle, HANDLE_FLAG_INHERIT,
                               b ? HANDLE_FLAG_INHERIT : 0) == 0)
        boost::throw_exception
            (system_error("boost::process::detail::file_handle::"
                          "win32_set_inheritable",
                          "SetHandleInformation failed",
                          ::GetLastError()));
}
#endif

// ------------------------------------------------------------------------

inline
const file_handle::handle_type
file_handle::invalid_value(void)
{
#if defined(BOOST_PROCESS_POSIX_API)
    return -1;
#elif defined(BOOST_PROCESS_WIN32_API)
    return INVALID_HANDLE_VALUE;
#endif
}

// ------------------------------------------------------------------------

} // namespace detail
} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_DETAIL_FILE_HANDLE_HPP)
