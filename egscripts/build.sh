#!/bin/sh
# Do not call this directly

env=$1
datestr="$2"
infile=$3
ver=$4

echo "#!${env}"
echo "# EG tools build ${datestr}"
echo "eg_version=${ver}"
echo "eg_date=\"${datestr}\""
cat ${infile}
