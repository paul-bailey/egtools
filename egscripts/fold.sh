# fold: fold long lines
sed 's/	/        /g' $* | awk '
BEGIN {
      N = 73  # folds at column N
}
{
        if ((n = length($0)) <= N)
                printf "%s\n\n", $0
        else {
                i = 1;
                N1 = N;
                extra = 0;
                while( n > N ) {
                        N1 = N;
                        # Do not split word up
                        while((substr($0,i+N1,1) != " ") && (N1 > 1))
                        {
                                N1--;
                        }
                        # The "+ 1" and "- 1" skip the space on the LF.
                        if(extra == 1)
                                printf "%s\n", substr($0, i + 1, N1 - 1)
                        else
                                printf "%s\n", substr($0, i, N1)
                        n -= N1;
                        i += N1;
                        extra = 1;
                }
                if(extra == 1)
                        printf "%s\n\n", substr($0, i + 1, n - 1)
                else
                        printf "%s\n\n", substr($0, i, n)
        }
}'
