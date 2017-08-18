#!/bin/sh
# -*- shell-script -*-
stamp=`date +%s`
while test $# -ne 0; do
    cp "$1" "${1}.bak"
    sed -e 's/( /(/g' \
        -e 's/ )/)/g' \
        -e 's/\[ /[/g' \
        -e 's/ \]/]/g' \
        -e 's/\<if(/if (/g' \
        -e 's/\<while(/while (/g' \
        -e 's/\<for(/for (/g' \
        -e 's/\<switch(/switch (/g' \
        "$1" > "${1}.${stamp}"
    mv "${1}.${stamp}" "$1"
    shift
done
