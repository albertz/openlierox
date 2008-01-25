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
//! \file boost/process/posix_child.hpp
//!
//! Includes the declaration of the posix_child class.
//!

#if !defined(BOOST_PROCESS_POSIX_CHILD_HPP)
/** \cond */
#define BOOST_PROCESS_POSIX_CHILD_HPP
/** \endcond */

#include <boost/process/config.hpp>

#if !defined(BOOST_PROCESS_POSIX_API)
#   error "Unsupported platform."
#endif

#include <cstdlib>
#include <map>

#include <boost/process/child.hpp>
#include <boost/process/detail/pipe.hpp>
#include <boost/process/detail/posix_ops.hpp>
#include <boost/process/pistream.hpp>
#include <boost/process/postream.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {
namespace process {

// ------------------------------------------------------------------------

//!
//! \brief POSIX implementation of the Child concept.
//!
//! The posix_child class implements the Child concept in a POSIX
//! operating system.
//!
//! A POSIX child differs from a regular child (represented by a
//! child object) in that it supports more than three communication
//! channels with its parent.  These channels are identified by regular
//! file descriptors (integers).
//!
//! This class is built on top of the generic child so as to allow its
//! trivial adoption.  When a program is changed to use the POSIX-specific
//! launcher (posix_launcher), it will most certainly need to migrate its
//! use of the child class to posix_child.  Doing so is only a matter of
//! redefining the appropriate object and later using the required extra
//! features: there should be no need to modify the existing code (e.g.
//! method calls) in any other way.
//!
class posix_child :
    public child
{
public:
    //!
    //! \brief Gets a reference to the child's input stream \a desc.
    //!
    //! Returns a reference to a postream object that represents one of
    //! the multiple input communication channels with the child process.
    //! The channel is identified by \a desc as seen from the child's
    //! point of view.  The parent can use the returned stream to send
    //! data to the child.
    //!
    //! Giving this function the STDIN_FILENO constant (defined in
    //! cstdlib) is a synonym for the get_stdin() call inherited from
    //! child.
    //!
    postream& get_input(int desc) const;

    //!
    //! \brief Gets a reference to the child's output stream \a desc.
    //!
    //! Returns a reference to a pistream object that represents one of
    //! the multiple output communication channels with the child process.
    //! The channel is identified by \a desc as seen from the child's
    //! point of view.  The parent can use the returned stream to retrieve
    //! data from the child.
    //!
    //! Giving this function the STDOUT_FILENO or STDERR_FILENO constants
    //! (both defined in cstdlib) are synonyms for the get_stdout() and
    //! get_stderr() calls inherited from child, respectively.
    //!
    pistream& get_output(int desc) const;

private:
    //!
    //! \brief Maps child's file descriptors to postream objects.
    //!
    typedef std::map< int, boost::shared_ptr< postream > > input_map;

    //!
    //! \brief Maps child's file descriptors to pistream objects.
    //!
    typedef std::map< int, boost::shared_ptr< pistream > > output_map;

    //!
    //! \brief Contains all relationships between child's input file
    //!        descriptors and their corresponding postream objects.
    //!
    input_map m_input_map;

    //!
    //! \brief Contains all relationships between child's output file
    //!        descriptors and their corresponding pistream objects.
    //!
    output_map m_output_map;

public: // XXX
    //!
    //! \brief Constructs a new POSIX child object representing a just
    //!        spawned child process.
    //!
    //! Creates a new child object that represents the just spawned process
    //! \a h.
    //!
    //! The \a infoin and \a infoout maps contain the pipes used to handle
    //! the redirections of the child process; at the moment, no other
    //! stream_info types are supported.  If the launcher was asked to
    //! redirect any of the three standard flows, their pipes must be
    //! present in these maps.
    //!
    //! This constructor is protected because the library user has no
    //! business in creating representations of live processes himself;
    //! the library takes care of that in all cases.
    //!
    posix_child(id_type id,
                detail::info_map& infoin,
                detail::info_map& infoout);
};

// ------------------------------------------------------------------------

inline
posix_child::posix_child(id_type id,
                         detail::info_map& infoin,
                         detail::info_map& infoout) :
    child(id,
          posix_info_locate_pipe(infoin, STDIN_FILENO, false),
          posix_info_locate_pipe(infoout, STDOUT_FILENO, true),
          posix_info_locate_pipe(infoout, STDERR_FILENO, true))
{
    for (detail::info_map::iterator iter = infoin.begin();
         iter != infoin.end(); iter++) {
        detail::stream_info& si = (*iter).second;

        if (si.m_type == detail::stream_info::use_pipe) {
            BOOST_ASSERT(si.m_pipe->wend().is_valid());
            boost::shared_ptr< postream > st(new postream(si.m_pipe->wend()));
            m_input_map.insert(input_map::value_type((*iter).first, st));
        }
    }

    for (detail::info_map::iterator iter = infoout.begin();
         iter != infoout.end(); iter++) {
        detail::stream_info& si = (*iter).second;

        if (si.m_type == detail::stream_info::use_pipe) {
            BOOST_ASSERT(si.m_pipe->rend().is_valid());
            boost::shared_ptr< pistream > st(new pistream(si.m_pipe->rend()));
            m_output_map.insert(output_map::value_type((*iter).first, st));
        }
    }
}

// ------------------------------------------------------------------------

inline
postream&
posix_child::get_input(int desc)
    const
{
    if (desc == STDIN_FILENO)
        return posix_child::get_stdin();
    else {
        input_map::const_iterator iter = m_input_map.find(desc);
        BOOST_ASSERT(iter != m_input_map.end());
        return *((*iter).second);
    }
}

// ------------------------------------------------------------------------

inline
pistream&
posix_child::get_output(int desc)
    const
{
    if (desc == STDOUT_FILENO)
        return posix_child::get_stdout();
    else if (desc == STDERR_FILENO)
        return posix_child::get_stderr();
    else {
        output_map::const_iterator iter = m_output_map.find(desc);
        BOOST_ASSERT(iter != m_output_map.end());
        return *((*iter).second);
    }
}

// ------------------------------------------------------------------------

} // namespace process
} // namespace boost

#endif // !defined(BOOST_PROCESS_POSIX_CHILD_HPP)
