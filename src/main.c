#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_video.h"

#include "GL/glew.h"
#include "mesh.h"

#include <GL/gl.h>
#include <GL/glu.h>

#define EXIT_FAIL()                                                            \
	exit_status = EXIT_FAILURE;                                                \
	goto quit

#define WINDOW_TITLE "City Generator"
#define BACKGROUND_COLOR                                                       \
	51.0f / 255.0f, 51.0f / 255.0f, 51.0f / 255.0f, 51.0f / 255.0f

GLuint load_and_compile_shader(const char *path, GLenum shader_type) {
	GLuint shader;

	FILE *file = fopen(path, "r");
	if (file == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
					 "Failed to open shader file %s: %s", path,
					 strerror(errno));
		return 0;
	}

	fseek(file, 0, SEEK_END);
	unsigned long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = malloc(size + 1);
	buffer[size] = 0;
	if (buffer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
					 "Failed to allocate memory to read shader source");
		fclose(file);
		return 0;
	}

	if (fread(buffer, size, 1, file) < 0) {
		fprintf(stderr, "Failed to read from file %s", path);
		free(buffer);
		fclose(file);
		return 0;
	}
	fclose(file);

	shader = glCreateShader(shader_type);

	if (shader == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create shader: %s",
					 gluErrorString(glGetError()));
	}

	glShaderSource(shader, 1, (const GLchar *const *)&buffer, (int *)&size);
	glCompileShader(shader);
	free(buffer);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		fprintf(stderr, "Shader compilation failed in %s: %s", path, infoLog);
	}
	return shader;
}

char *read_file(const char *filepath) {
	FILE *file = fopen(filepath, "r");
	if (file == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open file %s: %s",
					 filepath, strerror(errno));
		return 0;
	}

	fseek(file, 0, SEEK_END);
	unsigned long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = malloc(size + 1);
	buffer[size] = 0;
	if (buffer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
					 "Failed to allocate memory to read file %s", filepath);
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

GLuint create_shader_program(const char *vert_path, const char *frag_path) {
	GLuint vert_shader =
		load_and_compile_shader("./shaders/shader.vert", GL_VERTEX_SHADER);
	GLuint frag_shader =
		load_and_compile_shader("./shaders/shader.frag", GL_FRAGMENT_SHADER);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vert_shader);
	glAttachShader(shader_program, frag_shader);
	glLinkProgram(shader_program);
	int success;
	char infoLog[512];
	glGetProgramiv(shader_program, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		fprintf(stderr, "Shader linking failed: %s", infoLog);
		glDeleteProgram(shader_program);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return 0;
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return shader_program;
}

void resize_window_event(int width, int height) {
	glViewport(0, 0, width, height);
}

void callback(GLenum source, GLenum type, GLuint id, GLenum severity,
			  GLsizei length, const GLchar *message, const void *userParam) {
	fprintf(stderr, "OpenGL ERROR(%d):(%d):%s", severity, id, message);
}

int main() {
	SDL_Window *window = NULL;
	void *gl_context = NULL;
	GLuint vbo = 0;
	GLuint vao = 0;
	GLuint ebo = 0;

	uint8_t exit_status = 0;
	printf("Starting...\n");

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL: %s",
					 SDL_GetError());
		EXIT_FAIL();
	}

	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, 1920 / 2, 1080 / 2,
							  SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window: %s",
					 SDL_GetError());
		EXIT_FAIL();
	}

	gl_context = SDL_GL_CreateContext(window);
	if (gl_context == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
					 "Failed to create OpenGL context: %s", SDL_GetError());
		EXIT_FAIL();
	}
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init glew: %s",
					 glewGetErrorString(glew_err));
		EXIT_FAIL();
	}

	glDebugMessageCallback(callback, NULL);

	GLuint shader_program =
		create_shader_program("./shaders/shader.vert", "./shaders/shader.frag");
	if (shader_program == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create shader");
		goto quit;
	}

	struct Mesh *mesh = mesh_create_from_file("./test.obj");
	// struct Mesh *mesh = mesh_create_empty(4, 2);
	// float verts[] = {
	// 	0.5f,  0.5f,  0.0f, // top right
	// 	0.5f,  -0.5f, 0.0f, // bottom right
	// 	-0.5f, -0.5f, 0.0f, // bottom left
	// 	-0.5f, 0.5f,  0.0f	// top left
	// };
	// memcpy(mesh->vertices, verts, sizeof(float) * 3 * 4);
	// unsigned int inds[] = {0, 1, 3, 1, 2, 3};
	// memcpy(mesh->indecies, inds, sizeof(unsigned int) * 3 * 2);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(unsigned int) * 3 * mesh->index_count, mesh->indecies,
				 GL_STATIC_DRAW);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->vertex_count,
				 mesh->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glUseProgram(shader_program);

	// SDL_GL_SetSwapInterval(0);
	bool running = true;
	SDL_Event event;
	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					resize_window_event(event.window.data1, event.window.data2);
				}
			default:
				break;
			}
		}

		glClearColor(BACKGROUND_COLOR);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader_program);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		SDL_GL_SwapWindow(window);
	}
quit:
	glDeleteBuffers(1, &vbo);
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return exit_status;
}
