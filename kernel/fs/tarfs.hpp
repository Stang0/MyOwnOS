#ifndef TARFS_HPP
#define TARFS_HPP

#include "../lib/types.h"

bool tarfs_init(const uint8_t* archive, size_t size);
bool tarfs_is_ready();
void tarfs_ls();
bool tarfs_cat(const char* path);
void tarfs_self_test();

#endif
