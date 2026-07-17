#pragma once
#include "util.h"
EXTERN_C_BEGIN
#include <stdint.h>
/// @file vmem.h
/// @brief Cross-platform virtual memory utilities
/// @author Greenlord/S41L0R (Nintendo Switch implementation)
/// @author Torphedo (Windows & Unix implementations)
///
/// Unified API wrapping platform-specific virtual memory tricks, normally not
/// available to portable code.
/// Contains implementations for Windows, Unix, and Nintendo Switch. This
/// should also work on other Unix-based systems like macOS and FreeBSD.

enum {
    // TODO: Add a function to get the page size and/or convert a size in bytes
    // to a number of pages. Everything's 4KB pages right now, but we really
    // ought to be asking the OS at runtime.
    VMEM_PAGE_SIZE = 4 * 1024,
    VMEM_ALLOC_GRANULARITY = 64 * 1024,
};

/// @brief Reserve a virtual memory region without committing any physical RAM.
///
/// Only address space is reserved, and no space is reserved in the page file.
/// Before using any part of the memory, commit all or part of the region with
/// vmem_commit(). You may be able to use the region without errors on some
/// systems, but this isn't portable or guaranteed.
/// @return Pointer to reserved region, or NULL on failure
// TODO: Add an option to make the entire region immediately usable, at the cost of reserving page file space
// TODO: Look into writing a page fault handler on Windows to auto-commit reserved pages as needed
void* vmem_reserve(uint64_t size);

/// @brief Commit physical memory to a virtual memory region.
///
/// If successful, memory in the region becomes usable and space is reserved in
/// the page file. Actual physical pages are only allocated as needed when
/// parts of the committed region are accessed.
/// @return 0 on success, -1 on failure.
int vmem_commit(void* addr, uint64_t size);

/// @brief Free a virtual memory region reserved with @ref vmem_reserve().
///
/// This also frees physical memory committed to that region.
/// @note Something I noticed in testing is that it can crash if you
/// accidentally free a region larger than you reserved. When run under a
/// debugger, it was fine. If you see this happen, make sure your sizes match!
/// @return 0 on success, -1 on failure.
int vmem_free(void* addr, uint64_t size);

/// @brief Map a file into virtual memory without reading the entire thing
/// @param file File path
/// @return Pointer to mapped buffer
void* vmem_map_file(const char* file);

/// @brief Free an existing file mapping
/// @param addr Mapped buffer to free
/// @param size Size of the mapped buffer
void vmem_unmap_file(void* addr, uint64_t size);

EXTERN_C_END
