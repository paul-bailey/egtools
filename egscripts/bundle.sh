# A personal favorite!
#  This enables me to bundle files together but keep then editable
#  in a simple text editor.
case $1 in
   -[?hH]|-[Hh]elp|--[Hh]elp)
       echo "bundle -- put listed files into a bundle on stdout which sh will"
       echo "          unbundle"
       echo " From \"The UNIX Programming Environment,\" by Brian Kernighan and"
       echo "  Rob Pike, Prentice Hall, 1984."
       exit 0
       ;;
   -[Vv]|-[Vv]er*|--[Vv]er*)
       echo "EGTOOL version $eg_version, built $eg_date"
       exit 0
       ;;
   -*)
       echo "Unknown option: $1" >&2
       exit 1
       ;;
esac

echo "# To unbundle, sh this file"
for i
do
    echo "echo $i 1>&2"
    echo "cat >$i <<'End of $i'"
    cat $i
    echo "End of $i"
done
