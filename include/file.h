#pragma once

#include <stdio.h>
#include <stdbool.h>

unsigned long file_get_size(FILE *file);
bool file_read_line(FILE *file, char *buf, int max);
char *file_read_all(const char *filepath);
