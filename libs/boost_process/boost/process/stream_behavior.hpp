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
//! \file boost/process/stream_behavior.hpp
//!
//! Includes the declaration of the stream_behavior class and associated
//! free functions.
//!

#if !defined(BOOST_PROCESS_STREAM_BEHAVIOR_HPP)
/** \cond */
#define BOOST_PROCESS_STREAM_BEHAVIOR_HPP
/** \endcond */

#include <boost/process/config.hpp>

namespace boost {
namespace process {

namespace detail {
struct stream_info;
} // namespace detail

// ------------------------------------------------------------------------

//!
//! \brief Describes the possible states for a communication stream.
//!
//! Describes the possible states for a child's communication stream.
//!
class stream_behavior
{
public:
    enum type {
        //!
        //! The child's stream is connected to the parent by using an
        //! anonymous pipe so that they can send and receive data to/from
        //! each other.
        //!
        capture,

        //!
        //! The child's stream is closed upon startup so that it will not
        //! have any access to it.
        //!
        close,

        //!
        //! The child's stream is connected to the same stream used by the
        //! parent.  In other words, the corresponding parent's stream is
        //! inherited.
        //!
        inherit,

        //!
        //! The child's stream is connected to child's standard output.
        //! This is typically used when configuring the standard error
        //! stream.
        //!
        redirect_to_stdout,

        //!
        //! The child's stream is redirected to a null device so that its
        //! input is always zero or its output is lost, depending on
        //! whether the stream is an input or an output one.  It is
        //! important to notice that this is different from close because
        //! the child is still able to write data.  If we closed, e.g.
        //! stdout, the child might not work at all!
        //!
        silence,

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
        //!
        //! The child redirects the stream's output to the provided file
        //! descriptor.  This is a generalization of the portable
        //! redirect_to_stdout behavior.
        //!
        posix_redirect,
#endif
    };

    //!
    //! \brief Constructs a new stream behavior of type close.
    //!
    //! This public constructor creates a new stream behavior that defaults
    //! to the close behavior.  In general, you will want to use the
    //! available free functions to construct a stream behavior (including
    //! the close one).
    //!
    stream_behavior(void);

    //!
    //! \brief Returns this stream behavior's type.
    //!
    //! Returns the type of this stream behavior object.
    //!
    type get_type(void) const;

private:
    //!
    //! \brief This stream's behavior type.
    //!
    enum type m_type;

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
    // Valid when m_type == posix_redirect.
    int m_desc_to;
#endif

    //!
    //! \brief Constructs a new stream behavior of type \a t.
    //!
    //! Constructs a new stream behavior of type \a t.  It is the
    //! responsibility of the caller to fill in any other attributes
    //! required by the specified type, if any.
    //!
    stream_behavior(type t);

    // detail::stream_info provides a lower-level representation of
    // stream_behavior.  It is allowed to freely access this class'
    // contents for simplicity.
    friend struct detail::stream_info;

    // Pseudo-constructors for each stream_behavior type.  These are
    // needed because some types need to store additional information and
    // we must ensure it is initialized appropriately.  We would have used
    // a class hierarchy to represent this fact, but this could cause
    // problems when passing around regular objects (not pointers).
    //
    // XXX Individual classes look saner to me so it is worth to
    // investigate that design and see if it is possible to implement
    // without pointers.  As long as the classes are named the same as the
    // functions below, user code will require no changes.
    friend inline stream_behavior capture_stream(void);
    friend inline stream_behavior close_stream(void);
    friend inline stream_behavior inherit_stream(void);
    friend inline stream_behavior redirect_stream_to_stdout(void);
    friend inline stream_behavior silence_stream(void);
#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
    friend inline stream_behavior posix_redirect_stream(int to);
#endif
};

// ------------------------------------------------------------------------

inline
stream_behavior::stream_behavior(void) :
    m_type(stream_behavior::close)
{
}

// ------------------------------------------------------------------------

inline
stream_behavior::stream_behavior(stream_behavior::type t) :
    m_type(t)
{
}

// ------------------------------------------------------------------------

inline
stream_behavior::type
stream_behavior::get_type(void)
    const
{
    return m_type;
}

// ------------------------------------------------------------------------

//!
//! \brief Creates a new stream_behavior of type stream_behavior::capture.
//!
//! Creates a new stream_behavior of type stream_behavior::capture,
//! meaning that the child's stream is connected to the parent by using an
//! anonymous pipe so that they can send and receive data to/from each
//! other.
//!
inline
stream_behavior
capture_stream(void)
{
    return stream_behavior(stream_behavior::capture);
}

// ------------------------------------------------------------------------

//!
//! \brief Creates a new stream_behavior of type stream_behavior::close.
//!
//! Creates a new stream_behavior of type stream_behavior::close,
//! meaning that the child's stream is closed upon startup so that it
//! will not have any access to it.
//!
inline
stream_behavior
close_stream(void)
{
    return stream_behavior(stream_behavior::close);
}

// ------------------------------------------------------------------------

//!
//! \brief Creates a new stream_behavior of type stream_behavior::inherit.
//!
//! Creates a new stream_behavior of type stream_behavior::inherit,
//! meaning that the child's stream is connected to the same stream used
//! by the parent.  In other words, the corresponding parent's stream is
//! inherited.
//!
inline
stream_behavior
inherit_stream(void)
{
    return stream_behavior(stream_behavior::inherit);
}

// ------------------------------------------------------------------------

//!
//! \brief Creates a new stream_behavior of type
//! stream_behavior::redirect_to_stdout.
//!
//! Creates a new stream_behavior of type
//! stream_behavior::redirect_to_stdout, meaning that the child's stream is
//! connected to child's standard output.  This is typically used when
//! configuring the standard error stream.
//!
inline
stream_behavior
redirect_stream_to_stdout(void)
{
    return stream_behavior(stream_behavior::redirect_to_stdout);
}

// ------------------------------------------------------------------------

//!
//! \brief Creates a new stream_behavior of type stream_behavior::silence.
//!
//! Creates a new stream_behavior of type stream_behavior::capture,
//! meaning that the child's stream is redirected to a null device so that
//! its input is always zero or its output is lost, depending on whether
//! the stream is an input or an output one.  It is important to notice
//! that this is different from close because the child is still able to
//! write data.  If we closed, e.g. stdout, the child might not work at
//! all!
//!
inline
stream_behavior
silence_stream(void)
{
    return stream_behavior(stream_behavior::silence);
}

// ------------------------------------------------------------------------

#if defined(BOOST_PROCESS_POSIX_API) || defined(BOOST_PROCESS_DOXYGEN)
//!
//! \brief Creates a new stream_behavior of type
//! stream_behavior::posix_redirect.
//!
//! Creates a new stream_behavior of type stream_behavior::posix_redirect,
//! meaning that the child's stream is redirected to the \a to child's
//! file descriptor.  This is a generalization of the portable
//! redirect_stream_to_stdout() behavior.
//!
inline
stream_behavior
posix_redirect_stream(int to)
{
    stream_behavior sb(stream_behavior::posix_redirect);
    sb.m_desc_to = to;
    return sb;
}
#endif

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_STREAM_BEHAVIOR_HPP)
