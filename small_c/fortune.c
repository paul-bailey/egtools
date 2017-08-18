/*
 * Ever-so-slightly modified fortune as stolen -ahem, taken - from
 * Version 7 Unix source.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define FORTUNE_PATH DATADIR "/fortunes"

char line[500];
char bline[500];
char sharefile[500];

int main(void)
{
	double p;
	register char *l;
	long t;
	FILE *f;

	f = fopen(FORTUNE_PATH, "r");
	if (f == NULL) {
                perror("Cannot open fortune file");
		exit(1);
	}
	time(&t);
	srand(getpid() + (int)((t>>16) + t));
	p = 1.;
	for(;;) {
		l = fgets(line, 500, f);
		if(l == NULL)
			break;
		if(fmod(rand(), 32768.) < 32768./p)
			strcpy(bline, line);
		p += 1.;
	}
	fputs(bline, stdout);
	return(0);
}
