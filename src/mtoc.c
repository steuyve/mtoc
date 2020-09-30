#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct toc_item {
	char *contents;
	char *anchor;
	int depth;
	int list_num;
};

void die(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

struct out_buf {
	char *buffer;
	size_t len;
};

void outbuf_append(struct out_buf *ob, const char *s, size_t length)
{
	char *new = realloc(ob->buffer, ob->len + length);
	if (!new) die("realloc");

	memcpy(&new[ob->len], s, length);
	ob->buffer = new;
	ob->len += length;
}

void outbuf_free(struct out_buf *ob)
{
	free(ob->buffer);
	ob->len = 0;
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
	char *heading = malloc((length - depth - 1) * sizeof(char));
	if (!heading) die("malloc");
	heading = strncpy(heading, line + depth + 2, length - depth - 2);
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
	for (int j = 0; j < length; j++) {
		anchor[j] = '\0';
	}
	int i = 0;
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

void output_toc(struct out_buf *ob, struct toc_item *headers, int num_headers, int lflag)
{
	for (int i = 0; i < num_headers; i++) {
		for (int j = 0; j < headers[i].depth; j++) {
			outbuf_append(ob, "\t", 1);
		}
		if (lflag == 0) {
			char out[2 * strlen(headers[i].contents)];
			size_t len = snprintf(out, sizeof(out), "%d. %s\n", headers[i].list_num, headers[i].contents);
			outbuf_append(ob, out, len);
		} else if (lflag == 1) {
			char out[4 * strlen(headers[i].contents)];
			size_t len = snprintf(out, sizeof(out), "%d. [%s](#%s)\n", headers[i].list_num, headers[i].contents, headers[i].anchor);
			outbuf_append(ob, out, len);
		}
		free(headers[i].contents);
		free(headers[i].anchor);
	}
}

void process_file(FILE *fp, struct out_buf *ob, int lflag, int dflag)
{
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
			int depth = get_depth(line);
			if (depth + 1 > dflag) continue;
			headers[num_headers].depth = depth;
			headers[num_headers].contents = get_heading(line, linelen, headers[num_headers].depth);
			headers[num_headers].anchor = gen_anchor(headers[num_headers].contents, linelen);
			num_headers++;
		}
	}
	free(line);

	// output table of contents
	set_list_nums(headers, num_headers);
	output_toc(ob, headers, num_headers, lflag);

	free(headers);

	return;
}

int main(int argc, char **argv)
{
	if (argc == 1) fprintf(stderr, "Usage: %s [-lwd] filename ...\n", argv[0]);
	// parse arguments.
	int lflag = 0;
	int dflag = 6;
	int wflag = 0;
	int opt;
	while ((opt = getopt(argc, argv, "lwd:")) != -1) {
		switch (opt) {
			case 'l':
				lflag = 1;
				break;
			case 'w':
				wflag = 1;
				break;
			case 'd':
				dflag = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-lwd] filename ...\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	for (int index = optind; index < argc; index++) {
		// read-in a file as input.
		FILE *fp = fopen(argv[index], "r");
		if (!fp) die("fopen");

		// process the file.
		struct out_buf ob = {NULL, 0};
		process_file(fp, &ob, lflag, dflag);
		if (wflag == 0) {
			write(STDOUT_FILENO, ob.buffer, ob.len);
		} else if (wflag == 1) {
			char name[strlen(argv[index]) + 5];
			snprintf(name, sizeof(name), "%s.toc", argv[index]);
			FILE *toc_file = fopen(name, "w");
			if (!toc_file) die("fopen");
			write(fileno(toc_file), ob.buffer, ob.len);
			fprintf(toc_file, "\n");
			rewind(fp);
			char c;
			while ((c = fgetc(fp)) != EOF) {
				fputc(c, toc_file);
			}
			fclose(toc_file);
		}
		outbuf_free(&ob);

		// close the file.
		fclose(fp);
	}

	exit(EXIT_SUCCESS);
}
