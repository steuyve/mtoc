#include <stdio.h>
#include <stdlib.h>

void die(const char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("error: exactly one filename to process as an argument expected.");
		return 1;
	}

	printf("Opening file: %s...\n", argv[1]);
	// read in a markdown file as input.
	FILE *fp = fopen(argv[1], "r");
	if (!fp) die("fopen");

	// parse it, finding the headers and levels of depth, building a tree along the way.

	// output the table of contents.
	printf("Closing file: %s...\n", argv[1]);
	fclose(fp);
	return 0;
}
