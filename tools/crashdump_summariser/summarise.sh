#!/bin/sh

# Generates crash report summary lines for an entire directory of reports,
# then displays a ranking of crash reasons.

# martind@last.fm 2007-03-20


# ========
# = conf =
# ========

# for summary and error log
LOGDIR=/tmp

# these should not need changing
LOG_FILE=${LOGDIR}/summarise.log
ERR_FILE=${LOGDIR}/summarise.err

# to glob crash dump files in a directory
FILESPEC=*.zip

# find worker script
pushd `dirname $0`
INSTALL_ROOT=$PWD
popd
GET_SUMMARY=${INSTALL_ROOT}/get_summary.sh
if [ ! -e ${GET_SUMMARY} ]; then
  echo "Can't find ${GET_SUMMARY}."
  exit -1
fi


# ========
# = main =
# ========

# get input
if [ -z "$1" ]
then 
  echo "Please specify an input firectory that contains crash report zip files."
  exit -1
fi

input_dir="$1"

# prepare
if [ -e $LOG_FILE ]; then rm $LOG_FILE; fi
if [ -e $ERR_FILE ]; then rm $ERR_FILE; fi

# run
echo "Reading crash reports for ${input_dir}/${FILESPEC} ..."
for file in ${input_dir}/${FILESPEC}
do
  sh $GET_SUMMARY "${file}" >> $LOG_FILE 2>> $ERR_FILE || echo "${file} failed, check $ERR_FILE"
done

# report
echo 
echo "Version chart"
echo "-------------"
awk '{print $2}' < $LOG_FILE | sort | uniq -c | sort -rn
echo 
echo "Stack trace top ten"
echo "-------------------"
cut -d ' ' -f 3- < $LOG_FILE | sort | uniq -c | sort -rn | head -n 10
echo
echo "Check $LOG_FILE for the full list."
