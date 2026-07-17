// POSIX (Linux, BSD, Apple, generic Unix) implementation of vmem.h interface

#include "platform.h"

#if defined(PLATFORM_POSIX)
#include <stdlib.h> // For NULL
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "vmem.h"

#define ALIGN_UP(x, bound) ((x) + ((bound) - ((x) % (bound))))

enum {
    // https://www.kernel.org/doc/Documentation/vm/soft-dirty.txt
    PAGE_FLAG_SOFT_DIRTY = 55,
};

#define GET_BIT(x, y) (x & ((uint64_t)1 << y)) >> y

void* vmem_reserve(uint64_t size) {
    // MAP_ANONYMOUS tells it not to try to map a file into memory
    // MAP_NORESERVE tells it not to reserve space in the page file
    // (allows for larger-than-physical-memory reserved regions, of which only
    // small parts are used)
    void *retval = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (retval == MAP_FAILED) {
        // We need all implementations to return NULL on error for consistency
        return NULL;
    }
    return retval;
}

int vmem_commit(void* addr, uint64_t size) {
    // The kernel will automatically commit physical memory as needed when we
    // write to the region. However, here we edit the existing mapping to let
    // it reserve space in the page file.
    void* retval = mmap(addr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (retval != MAP_FAILED) {
	    return 0;
    }
    return -1;
}

int vmem_free(void* addr, uint64_t size) {
    return munmap(addr, size);
}

void* vmem_map_file(const char* file) {
    const long page_size = sysconf(_SC_PAGE_SIZE);
    struct stat st;
    if (stat(file, &st) != 0) {
        return NULL;
    }
    const int64_t size = ALIGN_UP(st.st_size, page_size);
    const int prot = PROT_READ | PROT_WRITE;
    const int flags = MAP_PRIVATE;

    const int fd = open(file, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }

    void* result = mmap(NULL, size, prot, flags, fd, 0);
    if (result == MAP_FAILED) {
        printf("Failed to map file '%s' because '%s'\n", file, strerror(errno));
        return NULL;
    }

    return result;
}

void vmem_unmap_file(void* addr, uint64_t size) {
    munmap(addr, size);
}
#endif
