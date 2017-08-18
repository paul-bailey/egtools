# XXX: Find out why the EOF markers have to be unique?

tmp_file=tmp_$$
cat>$tmp_file<<'End of tmp_file'
usage="\
$0 GREP_PATTERN
  fwrd: List files
  gfwrd: Show lines
Option is -c for .c files, -h, for .h files, -vhd for .vhd files, etc
Default is for c, h, and s files
"
ext="[chs]"
while : ; do
    case "$1" in
            -py | --py)  ext="py"   ;;
            -mk | --mk)  ext="mk"   ;;
              -c | --c)  ext="c"    ;;
          -cpp | --cpp)  ext="cpp"  ;;
              -h | --h)  ext="h"    ;;
          -txt | --txt)  ext="txt"  ;;
            -sh | --sh)  ext="sh"   ;;
        -bash | --bash)  ext="bash" ;;
            -am | --am)  ext="am"   ;;
            -in | --in)  ext="in"   ;;
          -asm | --asm)  ext="asm"  ;;
        -[sS] | --[sS])  ext="[sS]" ;;
              -v | --v)  ext="v"    ;;
          -vhd | --vhd)  ext="vhd"  ;;

        "-?" | -[Hh]elp | --[Hh]elp)
            echo "${usage}"
            exit 0;
            ;;

        -V | --V | -[Vv]er* | --[Vv]er*)
            echo "EGTOOOL version $eg_version, built $eg_date"
            exit 0;
            ;;

        -*)
            echo "Unknown option '$1'">&2
            exit 1;
            ;;

        *) break ;;
    esac
    shift
done
End of tmp_file

cat $tmp_file>gfwrd.sh
cat>>gfwrd.sh<<'End of gfwrd.sh'
grep "$1" `grep "$1" \`find . -name "*.$ext"\` | sed 's/:.*$//g' | uniq`
End of gfwrd.sh

cat $tmp_file>fwrd.sh
cat>>fwrd.sh<<'End of fwrd.sh'
grep "$1" `find . -name "*.$ext"` | sed 's/:.*$//g' | uniq
End of fwrd.sh

rm -rf $tmp_file
