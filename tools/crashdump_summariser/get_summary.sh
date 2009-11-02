#!/bin/sh

# Uses breakpad's stackwalk tool to create a crash report summary line with stack trace
# for the specified crash report zip file.

# martind@last.fm 2007-03-20


# ========
# = conf =
# ========

# location of stackwalk binary
MINIDUMP_STACKWALK=/Users/mongo/src/breakpad/src/processor/minidump_stackwalk

# directory for temp files
TMPDIR=/tmp/crashdump_summarise

# these should not need changing
ZIP_TMPDIR=${TMPDIR}/zip_temp
STACKWALK_OUTFILE=${TMPDIR}/stackwalk.out


# ==========
# = helper =
# ==========

# returns a one-line summary for a crashdump zip file
function getZipfileSummary() 
{
  FILE=$1
  DIR=$2
  
  unzip -d $DIR -qq -o $FILE || return -1

  # note: windows file, has \r\n line endings
  VERSION=`grep 'App version:' ${DIR}/container.log | tail -n 1 | perl -ne 'm/.*: ([^s]+).*\r/; print $1'` || return -1
  
  $MINIDUMP_STACKWALK ${DIR}/minidump.dmp > $STACKWALK_OUTFILE || return -1
  STACKTRACE=`perl -ne '
    if ($t){ 
      if (m/^Thread/){
        # 4. end of stack trace
        undef $t
      } 
      elsif (m/\s+\d+\s+(.*)/) {
        # 3. stack frame
        print " | $1"
      }
    } 
    else {
      # 1. crash reason
      if (m/^Crash reason:\s+([^\s]+)/) { print "$1 in " }
      # 2. start of stack trace
      else {
        $t=m/(Thread \d+) \(crashed\)/; print $1
      }
    }' < $STACKWALK_OUTFILE` || return -1
    
  rm ${DIR}/* || return -1

  echo "$FILE $VERSION $STACKTRACE"
}


# ========
# = main =
# ========

# get input
if [ -z "$1" ]
then 
  echo "Please specify a crash report zip file."
  exit -1
fi

file="$1"

# prepare
if [ ! -d $ZIP_TMPDIR ]; then mkdir -p $ZIP_TMPDIR; fi

# run
getZipfileSummary $file $ZIP_TMPDIR || exit -1

# cleanup
rm -rf $TMPDIR
