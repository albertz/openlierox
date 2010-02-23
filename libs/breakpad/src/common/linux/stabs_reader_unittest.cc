// Copyright (c) 2009, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// stabs_reader_unittest.cc: Unit tests for StabsReader.

#include <a.out.h>
#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stab.h>

#include "common/linux/stabs_reader.h"

using std::istream;
using std::istringstream;
using std::map;
using std::ostream;
using std::ostringstream;
using std::string;

namespace {

// Mock stabs file parser
//
// In order to test StabsReader, we parse a human-readable input file
// into in-memory .stab and .stabstr sections, and then pass those to
// StabsReader to look at.  We print the results to standard output,
// and then a script compares that with what was expected.
//
// Each line of a mock stabs file should have the following form:
//
//   TYPE OTHER DESC VALUE NAME
// 
// where all data is Latin-1 bytes and fields are separated by single
// space characters, except for NAME, which may contain spaces and
// continues to the end of the line.  The fields have the following
// meanings:
// 
// - TYPE: the name of the stabs symbol type; like SO or FUN.  These are
//   the names from /usr/include/bits/stab.def, without the leading N_.
//
// - OTHER, DESC, VALUE: numeric values for the n_other, n_desc, and
//   n_value fields of the stab.  These can be decimal or hex,
//   using C++ notation (10, 0x10)
//   
// - NAME: textual data for the entry.  STABS packs all kinds of
//   interesting data into entries' NAME fields, so calling it a NAME
//   is misleading, but that's how it is.  For SO, this may be a
//   filename; for FUN, this is the function name, plus type data; and
//   so on.

// I don't know if the whole parser/handler pattern is really worth
// the bureaucracy in this case.  But just writing it out as
// old-fashioned functions wasn't astonishingly clear either, so it
// seemed worth a try.

// A handler class for mock stabs data.
class MockStabsHandler {
 public:
  MockStabsHandler() { }
  virtual ~MockStabsHandler() { }
  // The mock stabs parser calls this member function for each entry
  // it parses, passing it the contents of the entry.  If this function
  // returns true, the parser continues; if it returns false, the parser
  // stops, and its Process member function returns false.
  virtual bool Entry(enum __stab_debug_code type, char other, short desc,
                     unsigned long value, const string &name) { return true; }
  // Report an error in parsing the mock stabs data.  If this returns true,
  // the parser continues; if it returns false, the parser stops and
  // its Process member function returns false.
  virtual bool Error(const char *format, ...) = 0;
};

// A class for parsing mock stabs files.
class MockStabsParser {
 public:
  // Create a parser reading input from STREAM and passing data to HANDLER.
  // Use FILENAME when reporting errors.
  MockStabsParser(const string &filename, istream *stream,
                  MockStabsHandler *handler);
  // Parse data from the STREAM, invoking HANDLER->Entry for each
  // entry we get.  Return true if we parsed all the data succesfully,
  // or false if we stopped early because Entry returned false, or if
  // there were any errors during parsing.
  bool Process();
 private:
  // A type for maps from stab type names ("SO", "SLINE", etc.) to
  // n_type values.
  typedef map<string, unsigned char> StabTypeNameTable;

  // Initialize the table mapping STAB type names to n_type values.
  void InitializeTypeNames();

  // Parse LINE, one line of input from a mock stabs file, and pass
  // its contents to handler_->Entry and return the boolean value that
  // returns.  If we encounter an error parsing the line, report it
  // using handler->Error.
  bool ParseLine(const string &line);

  const string &filename_;
  istream *stream_;
  MockStabsHandler *handler_;
  int line_number_;
  StabTypeNameTable type_names_;
};

MockStabsParser::MockStabsParser(const string &filename, istream *stream,
                                 MockStabsHandler *handler):
    filename_(filename), stream_(stream), handler_(handler),
    line_number_(0) {
  InitializeTypeNames();
}

bool MockStabsParser::Process() {
  // Iterate once per line, including a line at EOF without a
  // terminating newline.
  for(;;) {
    string line;
    std::getline(*stream_, line, '\n');
    if (line.empty() && stream_->eof())
      break;
    line_number_++;
    if (! ParseLine(line))
      return false;
  }
  return true;
}

void MockStabsParser::InitializeTypeNames() {
  // On GLIBC-based systems, <bits/stab.def> is a file containing a
  // call to an unspecified macro __define_stab for each stab type.
  // <stab.h> uses it to define the __stab_debug_code enum type.  We
  // use it here to initialize our mapping from type names to enum
  // values.
  //
  // This isn't portable to non-GLIBC systems.  Feel free to just
  // hard-code the values if this becomes a problem.
#  define __define_stab(name, code, str) type_names_[string(str)] = code;
#  include <bits/stab.def>
#  undef __define_stab
}

bool MockStabsParser::ParseLine(const string &line) {
  istringstream linestream(line);
  // Allow "0x" prefix for hex, and so on.
  linestream.unsetf(istringstream::basefield);
  // Parse and validate the stabs type.
  string typeName;
  linestream >> typeName;
  StabTypeNameTable::const_iterator typeIt = type_names_.find(typeName);
  if (typeIt == type_names_.end())
    return handler_->Error("%s:%d: unrecognized stab type: %s\n",
                           filename_.c_str(), line_number_, typeName.c_str());
  // These are int, not char and unsigned char, to ensure they're parsed
  // as decimal numbers, not characters.
  int otherInt, descInt;
  unsigned long value;
  linestream >> otherInt >> descInt >> value;
  if (linestream.fail())
    return handler_->Error("%s:%d: malformed mock stabs input line\n",
                           filename_.c_str(), line_number_);
  if (linestream.peek() == ' ')
    linestream.get();
  string name;
  getline(linestream, name, '\n');
  return handler_->Entry(static_cast<__stab_debug_code>(typeIt->second),
                         otherInt, descInt, value, name);
}

// A class for constructing .stab sections.
//
// A .stab section is an array of struct nlist entries.  These
// entries' n_un.n_strx fields are indices into an accompanying
// .stabstr section.
class StabSection {
 public:
  StabSection(): used_(0), size_(1) {
    entries_ = (struct nlist *) malloc(sizeof(*entries_) * size_);
  }
  ~StabSection() { free(entries_); }

  // Append a new 'struct nlist' entry to the end of the section, and
  // return a pointer to it.  This pointer is valid until the next
  // call to Append.  The caller should initialize the returned entry
  // as needed.
  struct nlist *Append();
  // Set SECTION to the contents of a .stab section holding the
  // accumulated list of entries added with Append.
  void GetSection(string *section);

 private:
  // The array of stabs entries,
  struct nlist *entries_;
  // The number of elements of entries_ that are used, and the allocated size
  // of the array.
  size_t used_, size_;
};

struct nlist *StabSection::Append() {
  if (used_ == size_) {
    size_ *= 2;
    entries_ = (struct nlist *) realloc(entries_, sizeof(*entries_) * size_);
  }
  assert(used_ < size_);
  return &entries_[used_++];
}

void StabSection::GetSection(string *section) {
  section->assign(reinterpret_cast<char *>(entries_),
                  sizeof(*entries_) * used_);
}

// A class for building .stabstr sections.
// 
// A .stabstr section is an array of characters containing a bunch of
// null-terminated strings.  A string is identified by the index of
// its initial character in the array.  The array always starts with a
// null byte, so that an index of zero refers to the empty string.
//
// This implementation also ensures that if two strings are equal, we
// assign them the same indices; most linkers do this, and some
// clients may rely upon it.  (Note that this is not quite the same as
// ensuring that a string only appears once in the section; you could
// share space when one string is a suffix of another, but we don't.)
class StabstrSection {
 public:
  StabstrSection(): next_byte_(1) { string_indices_[""] = 0; }
  // Ensure STR is present in the string section, and return its index.
  size_t Insert(const string &str);
  // Set SECTION to the contents of a .stabstr section in which the
  // strings passed to Insert appear at the indices we promised.
  void GetSection(string *section);
 private:
  // Maps from strings to .stabstr indices and back.
  typedef map<string, size_t> StringToIndex;
  typedef map<size_t, const string *> IndexToString;

  // A map from strings to the indices we've assigned them.
  StringToIndex string_indices_;

  // The next unused byte in the section.  The next string we add
  // will get this index.
  size_t next_byte_;
};

size_t StabstrSection::Insert(const string &str) {
  StringToIndex::iterator it = string_indices_.find(str);
  size_t index;
  if (it != string_indices_.end()) {
    index = it->second;
  } else {
    // This is the first time we've seen STR; add it to the table.
    string_indices_[str] = next_byte_;
    index = next_byte_;
    next_byte_ += str.size() + 1;
  }
  return index;
}

void StabstrSection::GetSection(string *section) {
  // First we have to invert the map.
  IndexToString byIndex;
  for (StringToIndex::const_iterator it = string_indices_.begin();
       it != string_indices_.end(); it++)
    byIndex[it->second] = &it->first;
  // Now we build the .stabstr section.
  section->clear();
  for (IndexToString::const_iterator it = byIndex.begin();
       it != byIndex.end(); it++) {
    // Make sure we're actually assigning it the index we claim to be.
    assert(it->first == section->size());
    *section += *(it->second);
    *section += '\0';
  }
}

// A mock stabs parser handler class that builds .stab and .stabstr
// sections.
class StabsSectionsBuilder: public MockStabsHandler {
 public:
  // Construct a handler that will receive data from a MockStabsParser
  // and construct .stab and .stabstr sections.  FILENAME should be
  // the name of the mock stabs input file; we use it in error
  // messages.
  StabsSectionsBuilder(const string &filename):
      filename_(filename), error_count_(0) { }

  // Overridden virtual member functions.
  bool Entry(enum __stab_debug_code type, char other, short desc,
             unsigned long value, const string &name);
  virtual bool Error(const char *format, ...);

  // Set SECTION to the contents of a .stab or .stabstr section
  // reflecting the entries that have been passed to us via Entry.
  void GetStab(string *section);
  void GetStabstr(string *section);

 private:
  StabSection stab_;                    // stabs entries we've seen
  StabstrSection stabstr_;              // and the strings they love
  const string &filename_;              // input filename, for error messages
  int error_count_;                     // number of errors we've seen so far
};

bool StabsSectionsBuilder::Entry(enum __stab_debug_code type, char other,
                                 short desc, unsigned long value,
                                 const string &name) {
  struct nlist *entry = stab_.Append();
  entry->n_type = type;
  entry->n_other = other;
  entry->n_desc = desc;
  entry->n_value = value;
  entry->n_un.n_strx = stabstr_.Insert(name);
  return true;
}

bool StabsSectionsBuilder::Error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  error_count_++;
  if (error_count_ >= 20) {
    fprintf(stderr,
            "%s: lots of errors; is this really a mock stabs file?\n",
            filename_.c_str());
    return false;
  }
  return true;
}

void StabsSectionsBuilder::GetStab(string *section) {
  stab_.GetSection(section);
}

void StabsSectionsBuilder::GetStabstr(string *section) {
  stabstr_.GetSection(section);
}

// A StabsHandler class that simply prints the reported data to a
// stream in a human-readable form.  FILENAME is the name of the input
// file, for use in error messages.
class StabsPrinter: public google_breakpad::StabsHandler {
 public:
  StabsPrinter(const string &filename, ostream &stream):
      filename_(filename), stream_(stream) { }

  bool StartCompilationUnit(const char *filename, uint64_t address,
                            const char *build_directory) {
    stream_ << "cu";
    stream_ << " filename=" << QuoteString(filename);
    stream_ << " addr=0x" << std::setbase(16) << address;
    if (build_directory)
      stream_ << " builddir=" << QuoteString(build_directory);
    stream_ << std::endl;
    return true;
  }

  bool EndCompilationUnit(uint64_t address) { 
    stream_ << "endcu addr=0x" << std::setbase(16) << address << std::endl;
    return true;
  }

  bool StartFunction(const std::string &name, uint64_t address) {
    stream_ << "fun";
    stream_ << " name=" << QuoteString(name);
    stream_ << " addr=0x" << std::setbase(16) << address;
    stream_ << std::endl;
    return true;
  }

  bool EndFunction(uint64_t address) {
    stream_ << "endfun addr=0x" << std::setbase(16) << address << std::endl;
    return true;
  }
  
  bool Line(uint64_t address, const char *filename, int number) {
    stream_ << "line";
    stream_ << " addr=0x" << std::setbase(16) << address;
    stream_ << " filename=" << QuoteString(filename);
    stream_ << " number=" << std::setbase(10) << number;
    stream_ << std::endl;
    return true;
  }

  void Warning(const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: warning: ", filename_.c_str());
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }

 private:
  // Return STR formatted as a C++ string literal: enclosed in double
  // quotes, with backslashes as necessary.
  string QuoteString(const string &str) {
    ostringstream quoted;
    quoted << "\"";
    for (string::const_iterator i = str.begin(); i != str.end(); i++) {
      char c = *i;
      if (c == 0)
        quoted << "\\0";
      else if (c < 32 || (c >= 127 && c < 256))
        quoted << "\\x" << std::setbase(16) << std::setw(2) << c;
      else if (c == '"' || c == '\\')
        quoted << "\\" << c;
      else
        quoted << c;
    }
    quoted << "\"";
    return quoted.str();
  }

  const string &filename_;
  ostream &stream_;
};

bool ProcessMockStabs(const string &filename) {
  // Open the input file.
  std::ifstream stream(filename.c_str());
  if (stream.fail()) {
    fprintf(stderr, "error opening mock stabs input file %s: %s\n",
            filename.c_str(), strerror(errno));
    return false;
  }
  // Parse the mock stabs data, and produce stabs sections to use as
  // test input to the reader.
  StabsSectionsBuilder builder(filename);
  MockStabsParser mock_parser(filename, &stream, &builder);
  if (! mock_parser.Process())
    return false;
  string stab, stabstr;
  builder.GetStab(&stab);
  builder.GetStabstr(&stabstr);

  // Run the parser on the test input, printing out whatever we find.
  StabsPrinter printer(filename, std::cout);
  google_breakpad::StabsReader reader(
      reinterpret_cast<const uint8_t *>(stab.data()),    stab.size(),
      reinterpret_cast<const uint8_t *>(stabstr.data()), stabstr.size(),
      &printer);
  return reader.Process();
}

} // anonymous namespace

int main(int argc, char **argv) {
  bool allOkay = true;
  if (argc <= 1) {
    fprintf(stderr, "usage: %s MOCKSTABFILE ...\n", argv[0]);
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    if (! ProcessMockStabs(argv[i]))
      allOkay = false;
  }
  return allOkay ? 0 : 1;
}

