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

void show_item(struct toc_item *item)
{
	printf("Depth: %d; Contents: %s", item->depth, item->contents);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("error incorrect number of arguments: exactly one filename to process expected.");
		return 1;
	}

	printf("Opening file: %s...\n", argv[1]);
	// read in a markdown file as input.
	FILE *fp = fopen(argv[1], "r");
	if (!fp) die("fopen");

	// parse it, finding the headers and levels of depth, building a tree along the way.
	size_t size = 32;
	struct toc_item headers[size];
	int num_headers = 0;
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		if (line[0] == '#') {
			struct toc_item *item = malloc(sizeof(struct toc_item));
			item->depth = get_depth(line);
			item->contents = malloc(sizeof(char) * linelen);
			strcpy(item->contents, line);
			headers[num_headers] = *item;
			num_headers++;
		}
	}
	free(line);

	for (int i = 0; i < num_headers; i++) {
		show_item(&headers[i]);
	}

	// output the table of contents.
	printf("Closing file: %s...\n", argv[1]);
	fclose(fp);
	return 0;
}
