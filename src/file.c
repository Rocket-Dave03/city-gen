#include "file.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


unsigned long file_get_size(FILE *file) {
	unsigned long pos = ftell(file);
	fseek(file, 0, SEEK_END);
	unsigned long size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
}


/**
 * Read one line from the FILE * into buf upto max bytes
 */
bool file_read_line(FILE *file, char *buf, int max) {
	if (file == NULL) {
		fprintf(stderr, "file_read_line called with NULL file");
		return false;
	}
	int idx = 0;
	memset(buf, 0, max);
	while (idx < max-1) {
		char cur;
		int read_ret = fread(&cur, 1, 1, file);
		// printf("read: %*s\n", 1, &cur);
		if (read_ret == 0) {
			if (feof(file)) {
				return false;
			} else if (ferror(file)){
				fprintf(stderr,"Failed to read from file");
				return false;
			}
		}
		if (cur == '\n') {
			return true;
		} else {
			buf[idx] = cur;
		}
		idx++;
	}
	return true;
}

char *file_read_all(const char *filepath) {
	FILE *file = fopen(filepath, "r");
	if (file == NULL) {
		fprintf(stderr, "Failed to open file %s: %s",
					 filepath, strerror(errno));
		return 0;
	}
	unsigned long size = file_get_size(file);
	char *buffer = malloc(size + 1);

	buffer[size] = 0;
	if (buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory to read file %s", filepath);
		fclose(file);
		return 0;
	}

	if (fread(buffer, size, 1, file) < 0) {
		fprintf(stderr, "Failed to read from file %s", filepath);
		free(buffer);
		fclose(file);
		return 0;
	}
	fclose(file);
	return buffer;
}
