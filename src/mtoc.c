#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int links = 0;

struct toc_item {
	char *contents;
	char *anchor;
	int depth;
	int list_num;
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
	char *heading = malloc((length - depth - 2) * sizeof(char));
	if (!heading) die("malloc");
	heading = strcpy(heading, line + depth + 2);
	length = length - depth - 3;
	while (heading[length] == '\n' || heading[length] == ' ' || heading[length] == '\t') {
		heading[length] = '\0';
		length--;
	}
	return heading;
}

char *gen_anchor(char *heading, ssize_t length)
{
	char *anchor = malloc(length * sizeof(char));
	if (!anchor) die("malloc");
	int i;
	while (heading[i] != '\0') {
		if (heading[i] == ' ') {
			anchor[i] = '-';
		} else if (heading[i] >= 'A' && heading[i] <= 'Z') {
			anchor[i] = heading[i] + 32;
		}else {
			anchor[i] = heading[i];
		}
		i++;
	}
	return anchor;
}

void show_item(struct toc_item *item)
{
	printf("Depth: %d; Contents: %s", item->depth, item->contents);
}

void set_list_nums(struct toc_item *headers, int num_headers)
{
	int levels_last[10];
	int num = 0;
	int prev_depth = headers[0].depth;
	for (int i = 0; i < num_headers; i++) {
		if (headers[i].depth > prev_depth) {
			levels_last[prev_depth] = num;
			num = 0;
			prev_depth = headers[i].depth;
		} else if (headers[i].depth < prev_depth) {
			num = levels_last[headers[i].depth];
			prev_depth = headers[i].depth;
		}
		num++;
		headers[i].list_num = num;
		levels_last[headers[i].depth] = num;
	}
}

void output_toc(struct toc_item *headers, int num_headers)
{
	printf("# Table of Contents\n");
	for (int i = 0; i < num_headers; i++) {
		for (int j = 0; j < headers[i].depth; j++) {
			printf("\t");
		}
		printf("%d. [%s](#%s)\n", headers[i].list_num, headers[i].contents, headers[i].anchor);
	}
}

int main(int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
			case 'l':
				links = 1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-l] filename\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	/*
	if (argc != 2) {
		printf("error incorrect number of arguments: exactly one filename to process expected.");
		return 1;
	}
	*/

	printf("Opening file: %s...\n", argv[optind]);
	// read-in a file as input.
	FILE *fp = fopen(argv[optind], "r");
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
			headers[num_headers].depth = get_depth(line);
			headers[num_headers].contents = get_heading(line, linelen, headers[num_headers].depth);
			headers[num_headers].anchor = gen_anchor(headers[num_headers].contents, linelen);
			num_headers++;
		}
	}
	free(line);

	//printf("Found the following header lines:\n");
	//for (int i = 0; i < num_headers; i++) {
	//	show_item(&headers[i]);
	//	printf("\n");
	//}

	// output table of contents
	set_list_nums(headers, num_headers);
	output_toc(headers, num_headers);

	// close the file.
	printf("Closing file: %s...\n", argv[optind]);
	fclose(fp);
	free(headers);
	exit(EXIT_SUCCESS);
}
