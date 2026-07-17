// Windows implementation of vmem.h interface

#include "platform.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <stdint.h>
#include <stdlib.h> // For NULL
#include "vmem.h"

// This API mimics VirtualAlloc() so it's a thin wrapper, not much to say here.
void* vmem_reserve(uint64_t size) {
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

int vmem_commit(void* addr, uint64_t size) {
    if (VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE) == NULL) {
	    return -1; // Failure :(
    }
    return 0;
}

int vmem_free(void* addr, uint64_t size) {
    if (VirtualFree(addr, 0, MEM_RELEASE)) {
        return 0;
    }
    return -1;
}

void* vmem_map_file(const char* file) {
    const DWORD access = GENERIC_READ;
    const DWORD share = FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE;
    HANDLE h = CreateFileA(file, access, share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    const DWORD prot = PAGE_WRITECOPY | SEC_RESERVE;
    HANDLE mapping = CreateFileMapping(h, NULL, prot, 0, 0, NULL);
    if (mapping == NULL) {
        CloseHandle(h);
        return NULL;
    }

    // Map the entire file into memory
    // Return value is NULL on failure, which matches our API
    void* buf = MapViewOfFile(mapping, FILE_MAP_COPY, 0, 0, 0);

    // Close the mapping. On Windows' side, our mapped view maintains an
    // internal reference to the mapping, so it's automatically destroyed once
    // we unmap our view (and there are no longer any references to it).
    CloseHandle(mapping);
    CloseHandle(h);

    return buf;
}

void vmem_unmap_file(void* addr, uint64_t size) {
    UnmapViewOfFile(addr);
}

#endif
