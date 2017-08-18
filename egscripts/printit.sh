USAGE="Usage: printit FILE1 [FILE2 [...]]"

case $1 in
    "-?" | [-]-[hH] | [-]-[Hh]elp)
        echo $USAGE
        exit
        ;;
esac

pr -f -l 56 $*
