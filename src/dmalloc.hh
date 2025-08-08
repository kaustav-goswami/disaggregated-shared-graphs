// So, this is an allocator file that initializes the graph using mmap
// I am targeting a C-like header, which should be compatible with both C/C++.
//

#ifndef __DMALLOC_HH__
#define __DMALLOC_HH__

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>

int *dmalloc(size_t size, int host_id);
int *hmalloc(size_t size, int host_id);
int *shmalloc(size_t size, int host_id);

#endif