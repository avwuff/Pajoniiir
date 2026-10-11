#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WEB_CRASH_EXPORT_MAX_BYTES (256u * 1024u)
#define WEB_CRASH_EXPORT_CHUNK_BYTES 512u
#define WEB_CRASH_EXPORT_TIMEOUT_MS 5000u

/* A dump identity is a bounded hexadecimal prefix, never the running SHA. */
size_t web_crash_sha_prefix(const uint8_t *src, size_t capacity, char out[65]);

typedef struct {
    void *context;
    bool (*idle)(void *context);
    uint64_t (*now_ms)(void *context);
    bool (*read)(void *context, size_t offset, void *buffer, size_t bytes);
    bool (*send)(void *context, const void *buffer, size_t bytes);
} web_crash_export_ops_t;

/* The sink must treat false as a truncated download, never a complete dump.
 * No playback/USB changes; an input that starts playback aborts this export. */
bool web_crash_export_copy(size_t image_size, size_t partition_size,
                           const web_crash_export_ops_t *ops);
