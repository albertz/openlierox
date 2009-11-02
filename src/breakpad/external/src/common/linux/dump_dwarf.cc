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

#include <algorithm>
#include <cassert>
#include <cxxabi.h>

#include "common/linux/dump_dwarf.h"

#include "common/linux/module.h"
#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2reader.h"
#include "common/dwarf/dwarf2diehandler.h"

namespace {

using std::vector;
using google_breakpad::Module;
using dwarf2reader::DwarfTag;
using dwarf2reader::DwarfAttribute;
using dwarf2reader::DwarfForm;
using dwarf2reader::AttributeList;

// A handler for populating google_breakpad::Module objects with data
// parsed by dwarf2reader::LineInfo objects.
class DumpDwarfLineHandler: public dwarf2reader::LineInfoHandler {
 public:
  // Add source files to MODULE, and add all lines to the end of
  // LINES.  LINES need not be empty.  It's up to our client to sort
  // out which lines belong to which functions, so we don't add them
  // to MODULE ourselves.
  DumpDwarfLineHandler(Module *module, vector<Module::Line> *lines) :
      module_(module),
      lines_(lines),
      unsized_line_(-1),
      highest_file_number_(-1),
      warned_bad_file_number_(false) { }
  
  ~DumpDwarfLineHandler() { }

  void DefineDir(const std::string &name, uint32 dir_num);
  void DefineFile(const std::string &name, int32 file_num,
                  uint32 dir_num, uint64 mod_time,
                  uint64 length);
  void AddLine(uint64 address, uint32 file_num, uint32 line_num,
               uint32 column_num);
  void EndSequence(uint64 address);

 private:

  typedef std::map<uint32, std::string> DirectoryTable;
  typedef std::map<uint32, Module::File *> FileTable;

  // The module we're contributing debugging info to.
  Module *module_;

  // The vector of lines we're accumulating.  In a Module, as in a
  // breakpad symbol file, lines belong to specific functions, but
  // DWARF simply assigns lines to addresses; one must infer the
  // line/function relationship using the functions' beginning and
  // ending addresses.  So we can't add these to the appropriate
  // function from module_ until we've read the function info as well.
  // Instead, we accumulate them here, and let whoever constructed
  // this sort it all out.
  vector<Module::Line> *lines_;

  // Since AddLine only gives us an address, not a size, we leave each
  // line's size unset, and then fix it up when we get the next line's
  // address, or reach the end of the sequence.  This is the index in
  // lines_ of the line whose size we need to fix up, or -1 if no
  // fixup is needed (for example, at the start of the sequence, or
  // immediately after an EndSequence).
  int unsized_line_;

  // A table mapping directory numbers to paths.
  DirectoryTable directories_;

  // A table mapping file numbers to Module::File pointers.
  FileTable files_;

  // The highest file number we've seen so far, or -1 if we've seen
  // none.  Used for dynamically defined file numbers.
  int32 highest_file_number_;

  // True if we've warned about:
  bool warned_bad_file_number_; // bad file numbers
};

void DumpDwarfLineHandler::DefineDir(const string &name, uint32 dir_num) {
  directories_[dir_num] = name;
}

void DumpDwarfLineHandler::DefineFile(const string &name, int32 file_num,
                                      uint32 dir_num, uint64 mod_time,
                                      uint64 length) {
  if (file_num == -1)
    file_num = ++highest_file_number_;
  else if (file_num > highest_file_number_)
    highest_file_number_ = file_num;

  // Use the directory number to find the file's complete name.
  std::string full_name;
  DirectoryTable::const_iterator directory_it = directories_.find(dir_num);
  if (directory_it != directories_.end())
    full_name = directory_it->second + "/" + name;
  else
    full_name = name;

  // Find a Module::File object of the given name, and add it to the
  // file table.
  files_[file_num] = module_->FindFile(full_name);
}

void DumpDwarfLineHandler::AddLine(uint64 address, uint32 file_num,
                                   uint32 line_num, uint32 column_num) {
  // Set the size of the previous line.
  if (unsized_line_ != -1) {
    Module::Line *l = &(*lines_)[unsized_line_];
    l->size_ = address - l->address_;
  }
  // Find the source file being referred to.
  Module::File *file = files_[file_num];
  if (! file) {
    if (! warned_bad_file_number_) {
      fprintf(stderr, "warning: DWARF line number data refers to "
              "undefined file numbers.\n");
      warned_bad_file_number_ = true;
    }
    // We're not going to override it below, so clear it now.
    unsized_line_ = -1;
    return;
  }
  Module::Line line;
  line.address_ = address;
  // We set the size when we get the next line or the EndSequence call.
  line.size_ = 0;
  line.file_ = file;
  line.number_ = line_num;
  unsized_line_ = lines_->size();  // Remember to fix up this line's size.
  lines_->push_back(line);
}

void DumpDwarfLineHandler::EndSequence(uint64 last_address) {
  // Set the size for the previous line.
  if (unsized_line_ != -1) {
    Module::Line *l = &(*lines_)[unsized_line_];
    l->size_ = last_address - l->address_;
    unsized_line_ = -1;
  }
}

typedef map<uint64, std::string> FunctionMap;

// A handler class for DW_TAG_subprogram DIEs.
class DumpDwarfFuncHandler: public dwarf2reader::DIEHandler {
 public:
  DumpDwarfFuncHandler(uint64 cu_offset, uint64 offset, const string& prefixname, vector<Module::Function *> *functions, FunctionMap *offset_to_funcinfo) :
	  compilation_unit_offset_(cu_offset), offset_(offset), prefixname_(prefixname), low_pc_(0), high_pc_(0), functions_(functions), offset_to_funcinfo_(offset_to_funcinfo) { }
  void ProcessAttributeUnsigned(enum DwarfAttribute attr,
                                enum DwarfForm form,
                                uint64 data);
  void ProcessAttributeString(enum DwarfAttribute attr,
                              enum DwarfForm form,
                              const string& data);
  void Finish();

 private:
  uint64 compilation_unit_offset_;
  uint64 offset_;
  string prefixname_;
  string name_;
  uint64 low_pc_, high_pc_;
  vector<Module::Function *> *functions_;
  FunctionMap* offset_to_funcinfo_;
};


// Given an offset value, its form, and the base offset of the
// compilation unit containing this value, return an absolute offset
// within the .debug_info section.
static uint64 GetAbsoluteOffset(uint64 offset,
                         enum DwarfForm form,
                         uint64 compilation_unit_base) {
  using namespace dwarf2reader;
  switch (form) {
    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref8:
    case DW_FORM_ref_udata:
      return offset + compilation_unit_base;
    case DW_FORM_ref_addr:
    default:
      return offset;
  }
}

void DumpDwarfFuncHandler::ProcessAttributeUnsigned(enum DwarfAttribute attr,
                                                    enum DwarfForm form,
                                                    uint64 data) {
  switch (attr) {
    case dwarf2reader::DW_AT_low_pc:  low_pc_  = data; break;
    case dwarf2reader::DW_AT_high_pc: high_pc_ = data; break;
    case dwarf2reader::DW_AT_specification: {
        // Some functions have a "specification" attribute
        // which means they were defined elsewhere. The name
        // attribute is not repeated, and must be taken from
        // the specification DIE. Here we'll assume that
        // any DIE referenced in this manner will already have
        // been seen, but that's not really required by the spec.
        uint64 abs_offset = GetAbsoluteOffset(data, form, compilation_unit_offset_);

        FunctionMap::iterator iter = offset_to_funcinfo_->find(abs_offset);
        if (iter != offset_to_funcinfo_->end()) {
          name_ = iter->second;
        } else {
          // If you hit this, this code probably needs to be rewritten.
          fprintf(stderr, "Error: DW_AT_specification, form %04x, Looking for DIE at offset %08llx, in DIE at offset %08llx, CU %08llx, data %08llx\n", form, abs_offset, offset_, compilation_unit_offset_, data);
        }
    }
    default: break;
  }
}

// Demangle using abi call.
// Older GCC may not support it.
static std::string Demangle(const std::string &mangled) {
	int status = 0;
	char *demangled = abi::__cxa_demangle(mangled.c_str(), NULL, NULL, &status);
	if (status == 0 && demangled != NULL) {
		std::string str(demangled);
		free(demangled);
		return str;
	}
	return std::string(mangled);
}

void DumpDwarfFuncHandler::ProcessAttributeString(enum DwarfAttribute attr,
                                                  enum DwarfForm form,
                                                  const string& data) {
  switch (attr) {
	case dwarf2reader::DW_AT_name: if(name_ == "") name_ = prefixname_ + data; break;
	case dwarf2reader::DW_AT_MIPS_linkage_name: name_ = Demangle(data); break;
    default: break;
  }
}

void DumpDwarfFuncHandler::Finish() {
  // Did we collect the information we need?  Not all DWARF function
  // entries have low and high addresses (for example, inlined
  // functions that were never used), but all the ones we're
  // interested in cover a non-empty range of bytes.
  if (low_pc_ < high_pc_) {
    // Create a Module::Function based on the data we've gathered, and
    // add it to the functions_ list.
    Module::Function *func = new Module::Function;
    func->name_ = name_;
    func->address_ = low_pc_;
    func->size_ = high_pc_ - low_pc_;
    func->parameter_size_ = 0;
    functions_->push_back(func);
  }

  offset_to_funcinfo_->insert( make_pair(offset_, name_) );
}

// A handler class for the root die of a DWARF compilation unit.
class DumpDwarfCURootHandler: public dwarf2reader::RootDIEHandler {
	friend struct DumpDwarfChildIterator;
 public:
  // Create a CU root DIE handler that deposits all the CU's data in
  // MODULE.  BYTE_READER and SECTION_MAP should be the same byte
  // reader and section map we're using for the CompilationUnit.
  DumpDwarfCURootHandler(const string &dwarf_filename, Module *module,
                         dwarf2reader::ByteReader *byte_reader,
                         const dwarf2reader::SectionMap *section_map) :
      dwarf_filename_(dwarf_filename),
      module_(module),
      byte_reader_(byte_reader),
      section_map_(section_map),
      has_source_line_info_(false),
      language_(dwarf2reader::DW_LANG_none),
      report_bad_line_details_(true),
      cu_offset_(0) { }
  ~DumpDwarfCURootHandler();

  void ProcessAttributeSigned(enum DwarfAttribute attr,
                              enum DwarfForm form,
                              int64 data);
  void ProcessAttributeUnsigned(enum DwarfAttribute attr,
                                enum DwarfForm form,
                                uint64 data);
  void ProcessAttributeString(enum DwarfAttribute attr,
                              enum DwarfForm form,
                              const string &data);
  DIEHandler *FindChildHandler(uint64 offset, enum DwarfTag tag,
                               const AttributeList &attrs);
  void Finish();
  bool StartCompilationUnit(uint64 offset, uint8 address_size,
                            uint8 offset_size, uint64 cu_length,
                            uint8 dwarf_version);
  bool StartRootDIE(uint64 offset, enum DwarfTag tag,
                    const AttributeList& attrs);

 private:
  
  // Read source line information at OFFSET in the .debug_line
  // section.  Record source files in module_, but record source lines
  // in lines_; we apportion them to functions in
  // AssignLinesToFunctions.
  void ReadSourceLines(uint64 offset);

  // Assign the lines in lines_ to the individual line lists of the
  // functions in functions_.  (DWARF line information maps an entire
  // compilation unit at a time, and gives no indication of which
  // lines belong to which functions, beyond their addresses.)
  void AssignLinesToFunctions();

  const string &dwarf_filename_;
  Module *module_;
  dwarf2reader::ByteReader *byte_reader_;
  const dwarf2reader::SectionMap *section_map_;

  // True if this compilation unit has source line information.
  bool has_source_line_info_;

  // The offset of this compilation unit's line number information in
  // the .debug_line section.
  uint64 source_line_offset_;

  // The source language of this compilation unit.
  dwarf2reader::DwarfLanguage language_;

  // The functions defined in this compilation unit.  We accumulate
  // them here during parsing.  Then, in Finish, we assign them lines
  // and add them to module_.
  //
  // Destroying this destroys all the functions this vector points to.
  vector<Module::Function *> functions_;

  FunctionMap offset_to_funcinfo_;

  // The line numbers we have seen thus far.  We accumulate these here
  // during parsing.  Then, in Finish, we call AssignLinesToFunctions
  // to dole them out to the appropriate functions.
  vector<Module::Line> lines_;

  // True if we should report details of line number data outside all
  // functions, and functions whose code isn't covered by line number
  // data.
  bool report_bad_line_details_;

  // For printing error messages.
  uint64 cu_offset_;
  string cu_name_;
};

DumpDwarfCURootHandler::~DumpDwarfCURootHandler() {
  for (vector<Module::Function *>::iterator it = functions_.begin();
       it != functions_.end(); it++)
    delete *it;
}

void DumpDwarfCURootHandler::ProcessAttributeSigned(enum DwarfAttribute attr,
                                                    enum DwarfForm form,
                                                    int64 data) {
  switch (attr) {
    case dwarf2reader::DW_AT_language: // source language of this CU

      break;
    default:
      break;
  }
}

void DumpDwarfCURootHandler::ProcessAttributeUnsigned(enum DwarfAttribute attr,
                                                      enum DwarfForm form,
                                                      uint64 data) {
  switch (attr) {
    case dwarf2reader::DW_AT_stmt_list: // Line number information.
      has_source_line_info_ = true;
      source_line_offset_ = data;
      break;
    case dwarf2reader::DW_AT_language: // source language of this CU
      language_ = static_cast<dwarf2reader::DwarfLanguage>(data);
      break;
    default:
      break;
  }
}

void DumpDwarfCURootHandler::ProcessAttributeString(enum DwarfAttribute attr,
                                                    enum DwarfForm form,
                                                    const string &data) {
  if (attr == dwarf2reader::DW_AT_name)
    cu_name_ = data;
}

struct DumpDwarfChildIterator : dwarf2reader::DIEHandler {
	DumpDwarfChildIterator(DumpDwarfCURootHandler* root, const string& prefixname = "")
	: root_(root), prefixname_(prefixname), fullname_(prefixname) {}

	dwarf2reader::DIEHandler* FindChildHandler(uint64 offset, enum DwarfTag tag, const AttributeList &attrs) {
		std::string childprefix = fullname_.empty() ? "" : (fullname_ + "::");
		switch (tag) {
			case dwarf2reader::DW_TAG_subprogram:
			case dwarf2reader::DW_TAG_inlined_subroutine:
				return new DumpDwarfFuncHandler(root_->cu_offset_, offset, childprefix, &root_->functions_, &root_->offset_to_funcinfo_);
			default:
				return new DumpDwarfChildIterator(root_, childprefix);
		}
	}

	void ProcessAttributeString(enum DwarfAttribute attr, enum DwarfForm form, const string &data) {
	   if (attr == dwarf2reader::DW_AT_name)
		   fullname_ = prefixname_ + data;
   }

	DumpDwarfCURootHandler* root_;
	std::string prefixname_;
	std::string fullname_;
};

dwarf2reader::DIEHandler *DumpDwarfCURootHandler::FindChildHandler(
    uint64 offset,
    enum DwarfTag tag,
    const AttributeList &attrs) {
  switch (tag) {
    case dwarf2reader::DW_TAG_subprogram:
    case dwarf2reader::DW_TAG_inlined_subroutine:
      return new DumpDwarfFuncHandler(cu_offset_, offset, "", &functions_, &offset_to_funcinfo_);
    default:
      return new DumpDwarfChildIterator(this);
  }
}

void DumpDwarfCURootHandler::ReadSourceLines(uint64 offset) {
  dwarf2reader::SectionMap::const_iterator map_entry
      = section_map_->find(".debug_line");
  if (map_entry == section_map_->end()) {
    fprintf(stderr, "warning: couldn't find DWARF line number info"
            " (\".debug_line\" section)\n");
    return;
  }
  const char *section_start = map_entry->second.first;
  uint64 section_length = map_entry->second.second;
  if (offset >= section_length) {
    fprintf(stderr, "warning: DWARF debug info claims line info starts"
            " beyond end of line section\n");
    return;
  }
  DumpDwarfLineHandler handler(module_, &lines_);
  dwarf2reader::LineInfo reader(section_start + offset,
                                section_length - offset,
                                byte_reader_, &handler);
  reader.Start();
}

// A local class for printing warnings.  This takes care of not
// repeating header lines and formatting the messages.
class Warnings {
 public:
  Warnings(const string &dwarf_filename, const string &cu_name,
           uint64 cu_offset, bool enabled) :
      dwarf_filename_(dwarf_filename), cu_name_(cu_name), cu_offset_(cu_offset),
      enabled_(enabled), printed_(false) { }
  void PrintOnce() { 
    if (! printed_) {
      fprintf(stderr, "%s: warning: in DWARF compilation unit \"%s\", at offset 0x%llx:\n",
              dwarf_filename_.c_str(), cu_name_.c_str(), cu_offset_);
      fprintf(stderr, "%s: warning: skipping unpaired lines/functions:\n",
              dwarf_filename_.c_str());
      printed_ = true;
    }
  }
  void Warn(const Module::Function *func) {
    if (! enabled_) return;
    PrintOnce();
    fprintf(stderr, "    function%s: %s\n", 
            (func->size_ == 0 ? " (zero-length)" : ""), func->name_.c_str());
  }
  void Warn(const Module::Line *line) {
    if (! enabled_) return;
    PrintOnce();
    fprintf(stderr, "    line%s: %s:%d at 0x%llx\n",
            (line->size_ == 0 ? " (zero-length)" : ""),
            line->file_->name_.c_str(), line->number_, line->address_);
  }
 private:
  const string &dwarf_filename_; // name of the DWARF input file
  const string &cu_name_;        // name of compilation unit within input file
  uint64 cu_offset_;             // offset of compilation unit in .debug_info
  bool enabled_;                 // Is this warning enabled?
  bool printed_;                 // Have we printed this warning?
};

void DumpDwarfCURootHandler::AssignLinesToFunctions() {
  Warnings warnings(dwarf_filename_, cu_name_, cu_offset_,
                    report_bad_line_details_);

  // Put both our functions and lines in order by address.
  sort(functions_.begin(), functions_.end(),
       Module::Function::CompareByAddress);
  sort(lines_.begin(), lines_.end(), Module::Line::CompareByAddress);

  // Make a single pass through both vectors from lower to higher
  // addresses, populating each Function's lines_ vector with lines
  // from our lines_ vector that fall within the function's address
  // range.
  vector<Module::Function *>::iterator func_it = functions_.begin();
  vector<Module::Line>::const_iterator line_it = lines_.begin();

  // The last line that we used any piece of.  We use this only for
  // generating warnings.
  const Module::Line *last_line_used = NULL;

  // This would be simpler if we assumed that source line entries
  // don't cross function boundaries.  However, there's no real reason
  // to assume that (say) a series of function definitions on the same
  // line wouldn't get coalesced into one line number entry.  The
  // DWARF spec certainly makes no such promises.
  //
  // So treat the functions and lines as peers, and take the trouble
  // to compute their ranges' intersections precisely.  In any case,
  // the hair here is a constant factor for performance; the
  // complexity from here on out is linear.
  while (func_it != functions_.end() && line_it != lines_.end()) {
    // Grab a pointer to the current function and line here; the
    // process of computing their intersection also tells us which
    // iterator(s) to advance, so we can't just use the iterators to
    // refer to the objects throughout.
    Module::Function *func = *func_it;
    const Module::Line *line = &*line_it;
    // The offset from the start of the function to the start of the line.
    // This subtraction may well wrap around, but it still works out.
    Module::Address offset = line->address_ - func->address_;
    // The intersection of the line's address range and the function's.
    Module::Address start, size;

    // The comparisons below are valid even if OFFSET is "negative" or
    // the function/line abuts the end of the address space, because
    // (we assume) the functions' and lines' ranges don't *overrun*
    // the end of the address space.  Be careful if you rearrange the
    // expressions here.

    // Does the start of the line fall within the function?  Note
    // that, since the comparison is between unsigned values, and the
    // lower bound is zero, there's no need to actually do that test;
    // the one comparison bounds both ends.
    if (offset < func->size_) {
      start = line->address_;
      Module::Address func_after_start = func->size_ - offset;
      if (line->size_ < func_after_start) {
        size = line->size_;
        line_it++;
      } else if (line->size_ > func_after_start) {
        size = func_after_start;
        func_it++;
      } else {
        size = func_after_start;
        line_it++, func_it++;
      }
    } else if (-offset < line->size_) {
      // The start of the function falls within the line.
      Module::Address line_after_start = line->size_ + offset;
      start = func->address_;
      if (func->size_ < line_after_start) {
        size = func->size_;
        func_it++;
      } else if (func->size_ > line_after_start) {
        size = line_after_start;
        line_it++;
      } else {
        size = line_after_start;
        line_it++, func_it++;
      }
    } else if (func->address_ < line->address_) {
      // The function falls entirely before the line.
      warnings.Warn(func);
      func_it++;
      continue;
    } else if (func->address_ > line->address_) {
      // The line falls entirely before the function.
      //
      // If GCC emits padding after one function to align the start of
      // the next, then it will attribute the padding instructions to
      // the last source line of function (to reduce the size of the
      // line number info), but omit it from the DW_AT_{low,high}_pc
      // range given in .debug_info (since it costs nothing to be
      // precise there).  If we did use at least some of the line
      // we're about to skip, then assume this is what happened, and
      // don't warn.
      if (line != last_line_used)
        warnings.Warn(line);
      line_it++;
      continue;
    } else {
      // The line and function fall at the same address, but there is
      // no intersection.  One or both must both be zero-length.  Bad
      // input, but we should tolerate that.
      if (func->size_ == 0) {
        warnings.Warn(func);
        func_it++;
      }
      if (line->size_ == 0) {
        warnings.Warn(line);
        line_it++;
      }
      continue;
    }

    // Add the intersection to the function's line list.
    Module::Line new_line = *line;
    new_line.address_ = start;
    new_line.size_ = size;
    func->lines_.push_back(new_line);
    last_line_used = line;
  }

  if (report_bad_line_details_) {
    while (line_it != lines_.end()) {
      warnings.Warn(&*line_it);
      line_it++;
    }
    while (func_it != functions_.end()) {
      warnings.Warn(*func_it);
      func_it++;
    }
  }
}

void DumpDwarfCURootHandler::Finish() {
  // Assembly language files have no function data, and that gives us
  // no place to store our line numbers (even though the GNU toolchain
  // will happily produce source line info for assembly language
  // files).  To avoid spurious warnings about lines we can't assign
  // to functions, skip assembly language CUs.
  //
  // DWARF has no generic language code for assembly language; this is
  // what the GNU toolchain uses.
  if (language_ == dwarf2reader::DW_LANG_Mips_Assembler)
    return;

  // Read source line info, if we have any.
  if (has_source_line_info_)
    ReadSourceLines(source_line_offset_);

  // Dole out lines to the appropriate functions.
  AssignLinesToFunctions();

  // Add our now-populated function list to module_.
  module_->AddFunctions(functions_.begin(), functions_.end());

  // Since functions_ owns the function objects it points to, but
  // we've handed the functions off to module_, we'd better correct
  // that misapprehension.
  functions_.clear();
}

bool DumpDwarfCURootHandler::StartCompilationUnit(uint64 offset,
                                                  uint8 address_size,
                                                  uint8 offset_size,
                                                  uint64 cu_length,
                                                  uint8 dwarf_version) {
  cu_offset_ = offset;
  return dwarf_version >= 2;
}

bool DumpDwarfCURootHandler::StartRootDIE(uint64 offset, enum DwarfTag tag,
                                          const AttributeList& attrs) {
  // We don't deal with partial compilation units (the only other tag
  // likely to be used for root DIE).
  return tag == dwarf2reader::DW_TAG_compile_unit;
}

} // anonymous namespace

namespace google_breakpad {

bool LoadDwarf(const string &dwarf_filename, const ElfW(Ehdr) *elf_header,
               Module *module) {
  // Figure out what endianness this file is.
  dwarf2reader::Endianness endianness;
  if (elf_header->e_ident[EI_DATA] == ELFDATA2LSB)
    endianness = dwarf2reader::ENDIANNESS_LITTLE;
  else if (elf_header->e_ident[EI_DATA] == ELFDATA2MSB)
    endianness = dwarf2reader::ENDIANNESS_BIG;
  else {
    fprintf(stderr, "bad data encoding in ELF header: %d\n",
            elf_header->e_ident[EI_DATA]);
    return false;
  }
  // Build an appropriate bytereader for that endianness.
  dwarf2reader::ByteReader byte_reader(endianness); 
  // Build a map of the ELF file's sections.
  dwarf2reader::SectionMap section_map;
  const ElfW(Shdr) *sections
      = reinterpret_cast<ElfW(Shdr) *>(elf_header->e_shoff);
  int num_sections = elf_header->e_shnum;
  const ElfW(Shdr) *shstrtab = sections + elf_header->e_shstrndx;
  for (int i = 0; i < num_sections; i++) {
    const ElfW(Shdr) *section = &sections[i];
    string name = reinterpret_cast<const char *>(shstrtab->sh_offset
                                                 + section->sh_name);
    const char *contents = reinterpret_cast<const char *>(section->sh_offset);
    uint64 length = section->sh_size;
    section_map[name] = std::make_pair(contents, length);
  }
  // Find the length of the .debug_info section.
  std::pair<const char *, uint64> debug_info_section
      = section_map[".debug_info"];
  // We should never have been called if the file doesn't have a
  // .debug_info section.
  assert(debug_info_section.first);
  uint64 debug_info_length = debug_info_section.second;

  // Parse all the compilation units in the .debug_info section.
  for (uint64 offset = 0; offset < debug_info_length;) {
    // Make a handler for the root DIE that populates MODULE with the
    // data we find.
    DumpDwarfCURootHandler root_handler(dwarf_filename, module, &byte_reader,
                                        &section_map);
    // Make a Dwarf2Handler that drives our DIEHandler.
    dwarf2reader::DIEDispatcher die_dispatcher(&root_handler);
    // Make a DWARF parser for the compilation unit at OFFSET.
    dwarf2reader::CompilationUnit reader(section_map, offset, &byte_reader,
                                         &die_dispatcher);
    // Process the entire compilation unit; get the offset of the next.
    offset += reader.Start();
  }
  return true;
}

} // namespace google_breakpad
