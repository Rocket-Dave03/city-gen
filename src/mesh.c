#include "mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

void mesh_delete(struct Mesh *self) {
	if (self != NULL) {
		free(self->vertices);
		free(self->indecies);
		free(self);
	}
}

struct Mesh *mesh_create_empty(int vert_count, int index_count) {
	struct Mesh *self = malloc(sizeof(struct Mesh));

	if (self == NULL) {
		fprintf(stderr, "Failed to allocate memory for Mesh");
		return NULL;
	}

	self->vertices = malloc(3 * sizeof(float) * vert_count);
	if (self->vertices == NULL) {
		fprintf(stderr, "Failed to allocate memory for Mesh");
		mesh_delete(self);
		return NULL;
	}

	self->indecies = malloc(3 * sizeof(float) * index_count);
	if (self->indecies == NULL) {
		fprintf(stderr, "Failed to allocate memory for Mesh");
		mesh_delete(self);
		return NULL;
	}
	self->vertex_count = vert_count;
	self->index_count = index_count;

	return self;
}

struct Mesh *mesh_create_from_file(const char *filepath) {
	int vert_count = 0;
	int index_count = 0;

	char read_buf[128] = {};
	FILE *file = fopen(filepath, "r");
	if (file == NULL) {
		fprintf(stderr, "Failed to open file: %s", filepath);
		return NULL;
	}
	while (file_read_line(file, (char *)&read_buf, 128)) {
		if (read_buf[0] == 'v') {
			vert_count++;
		} else if (read_buf[0] == 'f') {
			index_count += 2;
		}
	}
	struct Mesh *self = mesh_create_empty(vert_count, index_count);
	if (self == NULL) {
		fprintf(stderr, "Faield to create mesh struct");
	}
	fseek(file, 0, SEEK_SET);

	int cur_vert_count = 0;
	int cur_index_count = 0;

	while (file_read_line(file, (char *)&read_buf, 128)) {
		if (read_buf[0] == 'v') {
			float x;
			float y;
			float z;

			sscanf(read_buf, "v %f %f %f", &x, &y, &z);
			self->vertices[cur_vert_count * 3 + 0] = x / 2.0;
			self->vertices[cur_vert_count * 3 + 1] = y / 2.0;
			self->vertices[cur_vert_count * 3 + 2] = z / 2.0;
			cur_vert_count++;
		} else if (read_buf[0] == 'f') {
			if (read_buf[8] != 0) {
				int a, b, c, d;
				sscanf(read_buf, "f %u %u %u %u", &a, &b, &c, &d);
				// Tri 1/2
				self->indecies[cur_index_count * 3 + 0] = a - 1;
				self->indecies[cur_index_count * 3 + 1] = b - 1;
				self->indecies[cur_index_count * 3 + 2] = c - 1;
				// Tri 2/2
				self->indecies[cur_index_count * 3 + 3] = c - 1;
				self->indecies[cur_index_count * 3 + 4] = d - 1;
				self->indecies[cur_index_count * 3 + 5] = a - 1;
				cur_index_count += 2;
			} else {
				int a, b, c;
				sscanf(read_buf, "f %u %u %u", &a, &b, &c);
				self->indecies[cur_index_count * 3 + 0] = a - 1;
				self->indecies[cur_index_count * 3 + 1] = b - 1;
				self->indecies[cur_index_count * 3 + 2] = c - 1;
				cur_index_count += 1;
			}
		}
	}

	self->index_count = cur_index_count;
	self->vertex_count = cur_vert_count;

	return self;
}
