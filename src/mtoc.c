#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct toc_item {
	char *contents;
	int depth;
};

void die(const char *s)
{
	perror(s);
	exit(1);
}

int get_depth(char *line)
{
	int depth = 0;
	while (line[depth] != '\0' && line[depth] != ' ') {
		depth++;
	}

	return depth - 1;
}

char *get_heading(char *line, ssize_t length, int depth)
{
	char *heading;
	heading = malloc((length - depth - 2) * sizeof(char));
	if (!heading) die("malloc");
	heading = strcpy(heading, line + depth + 2);
	length = length - depth - 3;
	while (heading[length] == '\n' || heading[length] == ' ' || heading[length] == '\t') {
		heading[length] = '\0';
		length--;
	}
	return heading;
}

void show_item(struct toc_item *item)
{
	printf("Depth: %d; Contents: %s", item->depth, item->contents);
}

void output_toc(struct toc_item *headers, int num_headers)
{
	printf("# Table of Contents\n");
	for (int i = 0; i < num_headers; i++) {
		for (int j = 0; j < headers[i].depth; j++) {
			printf("\t");
		}
		printf("%d. %s\n", (i + 1), headers[i].contents);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("error incorrect number of arguments: exactly one filename to process expected.");
		return 1;
	}

	printf("Opening file: %s...\n", argv[1]);
	// read-in a file as input.
	FILE *fp = fopen(argv[1], "r");
	if (!fp) die("fopen");

	// finding the headers and levels of depth.
	size_t num_items = 32;
	struct toc_item *headers = malloc(num_items * sizeof(struct toc_item));
	if (!headers) die("malloc");
	int num_headers = 0;
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		if (line[0] == '#') {
			if (num_headers == num_items) {
				num_items *= 2;
				headers = realloc(headers, num_items * sizeof(struct toc_item));
				if (!headers) die("realloc");
			}
			struct toc_item *item = malloc(sizeof(struct toc_item));
			if (!item) die("malloc");
			item->depth = get_depth(line);
			item->contents = malloc(linelen * sizeof(char));
			if (!(item->contents)) die("malloc");
			item->contents = get_heading(line, linelen, item->depth);
			headers[num_headers] = *item;
			num_headers++;
		}
	}
	free(line);

	printf("Found the following header lines:\n");
	for (int i = 0; i < num_headers; i++) {
		show_item(&headers[i]);
		printf("\n");
	}

	// output table of contents
	output_toc(headers, num_headers);

	// close the file.
	printf("Closing file: %s...\n", argv[1]);
	fclose(fp);
	free(headers);
	return 0;
}
