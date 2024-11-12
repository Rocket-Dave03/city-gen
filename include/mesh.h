#pragma once

struct Mesh {
	float *vertices;
	int *indecies;

	int vertex_count;
	int index_count;
};

void mesh_delete(struct Mesh *self);

struct Mesh *mesh_create_empty(int vert_count, int index_count);
struct Mesh *mesh_create_from_file(const char *filepath);


