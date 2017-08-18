# todo: Make this take from stdin
usage="\
$0 - Convert a number
Usage: $0 [OPTION] NUMBER

Options:
 --help    If you're seeing this, you already know what it does
 --version EG tool version

 -b        print binary number in 0bnnnn... format
 -d        print decimal number
 -x        print hexadecimal number in 0xnnnn.... format
 -o        print octal number in 0nnnn.... format

Meant as an easier way to manipulate in numbers than printf (1),
which can get ugly with special and escape characters when passing
through dc
"
decimal_only=no
binary_only=no
hex_only=no
octal_only=no
while test $# -ne 0; do
    case $1 in
        "-?" | --help)
            echo "$usage"
            exit
            ;;
        -b)
            binary_only=yes
            decimal_only=no
            hex_only=no
            octal_only=no
            ;;
        -d)
            decimal_only=yes
            binary_only=no
            hex_only=no
            octal_only=no
            ;;
        -x | -h)
            hex_only=yes
            decimal_only=no
            binary_only=no
            octal_only=no
            ;;
        -o)
            hex_only=no
            decimal_only=no
            binary_only=no
            octal_only=yes
            ;;
        -V | --version)
            echo "EGTOOLS Version $eg_version built on $eg_date"
            exit
            ;;
        -*)
            echo "Unexpected option '$1'" >&2
            exit
            ;;
        *)
            x=$1
            break;
            ;;
    esac
    shift
done

verbose=no
if test "$decimal_only$binary_only$hex_only$octal_only" = "nononono"
then
    verbose=yes
fi

quot=""
stripargs="${quot}s/^0*//g${quot}"
hex_striparg="${quot}s/^0x//g${quot}"
bin_striparg="${quot}s/^0b//g${quot}"
case $x in
    *.*)
        echo "Cannot handle non-integers"
        exit 1
        ;;
    0x* | *h | 0X* )
        type="hex"
        stripargs="-e $hex_striparg -e ${quot}s/h$//g${quot} -e $stripargs"
        ;;
    0b* | 0B*)
        type="bin"
        stripargs="-e $bin_striparg -e $stripargs"
        ;;
    0*)
        type="oct"
        ;;
    *)
        type="dec"
        ;;
esac

x=`echo $x | sed $stripargs`
if test -z $x; then
  echo 0
  exit 1
fi

octn=""
binn=""
hexn=""
decn=0
if test "$type" = "hex"; then
   hexn=$x
   decn=`printf "%d" 0x$x`
elif test "$type" = "oct"; then
   octn=$x
   decn=`printf "%d" 0$x`
elif test "$type" = "bin"; then
   binn=$x
   # printf (1) doesn't understand "0bxxxx", so do this
   # the Polish way...
   while test -n "$x"; do
       firstc=`echo $x | cut -b1`
       case $firstc in
           0|1);;
           *)
               echo "Not a valid number">&2
               exit
               ;;
       esac
       decn=`dc -e "$decn 2 * $firstc + p"`
       x=`echo $x | cut --complement -b1`
   done
else
   decn=$x
fi
if test -z $decn; then
   echo "Counld not parse" >&2
   exit
fi
if test -z $octn; then
   octn=`printf %o $decn`
fi
if test -z $hexn; then
   hexn=`printf %X $decn`
fi
if test -z $binn; then
   x=$hexn
   while test -n "$x"; do
       firstc=`echo $x | cut -b1`
       case $firstc in
           0)   binn="$binn 0000";;
           1)   binn="$binn 0001";;
           2)   binn="$binn 0010";;
           3)   binn="$binn 0011";;
           4)   binn="$binn 0100";;
           5)   binn="$binn 0101";;
           6)   binn="$binn 0110";;
           7)   binn="$binn 0111";;
           8)   binn="$binn 1000";;
           9)   binn="$binn 1001";;
           a|A) binn="$binn 1010";;
           b|B) binn="$binn 1011";;
           c|C) binn="$binn 1100";;
           d|D) binn="$binn 1101";;
           e|E) binn="$binn 1110";;
           f|F) binn="$binn 1111";;
           *)
               echo "Not a valid number">&2
               exit
               ;;
       esac
       x=`echo $x | cut --complement -b1`
   done
fi

if test "$verbose" = "yes"; then
    echo "Octal:   $octn"
    echo "Decimal: $decn"
    echo "Binary:  $binn"
    echo "Hex:     $hexn"
elif test "$decimal_only" = "yes"; then
    echo $decn
elif test "$binary_only" = "yes"; then
    echo 0b$binn | sed 's/ //g'
elif test "$hex_only" = "yes"; then
    echo 0x$hexn
elif test "$octal_only" = "yes"; then
    echo 0$octn
fi
