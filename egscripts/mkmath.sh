#!bin/sh

parser="\
usage=\"\\
egadd, egsub, egmul
  dc wrappers that can read 0xn, 0n, 0bn, etc., in addition
  to decimal values.

Usage: \$0 [VAL1 [VAL2...]]

Options:
  -?, --help
     Show this help
  -V, --version
     Show version info
\"
case \$1 in
  \"-?\" | --help)
    echo \"\$usage\"
    exit
    ;;
  -V | --version)
    echo \"EGTOOLS version \$eg_version built on \$eg_date\"
    exit
    ;;
  -*)
    echo \"Unkown option\"
    exit 1
    ;;
  *) ;;
esac
"

do_math="\
res=@START@
for i
do
   idec=\`egnum -d \${i}\`
   res=\`dc -e \"\${res} \${idec} @OPERAND@ p\"\`
done
echo \$res
"

case $1 in
    # For recursive calls, to forgo the need for bash-like functions.
    --subst_str)
        echo "$do_math" | sed -e "s!@OPERAND@!$2!g" -e "s/@START@/$3/g"
        exit 0
        ;;
    *)
        ;;
esac

sub_div_start='\`egnum \-d \$(1)\`;shift'
mkoutp="sh $0 --subst_str"

for i in egsub.sh egadd.sh egmul.sh egdiv.sh
do
    echo "$parser">$i
done

$mkoutp - "$sub_div_start" >> egsub.sh
$mkoutp + 0 >> egadd.sh
$mkoutp "*" 1 >> egmul.sh
$mkoutp "/" "$sub_div_start" >> egdiv.sh
