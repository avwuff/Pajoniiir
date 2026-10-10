#include "web_crash_export.h"

size_t web_crash_sha_prefix(const uint8_t *src, size_t capacity, char out[65])
{
    out[0] = '\0';
    if (!src) return 0;
    size_t n = 0;
    while (n < capacity && n < 64u && src[n]) {
        unsigned char c = src[n];
        if (c >= 'A' && c <= 'F') c = (unsigned char)(c + ('a' - 'A'));
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            out[0] = '\0'; return 0;
        }
        out[n++] = (char)c;
    }
    if (n == capacity || (n == 64u && n < capacity && src[n])) {
        out[0] = '\0'; return 0;
    }
    out[n] = '\0';
    return n;
}

bool web_crash_export_copy(size_t size, size_t partition_size,
                           const web_crash_export_ops_t *ops)
{
    if (!ops || !ops->idle || !ops->now_ms || !ops->read || !ops->send ||
        size < sizeof(uint32_t) || size > partition_size ||
        size > WEB_CRASH_EXPORT_MAX_BYTES) return false;
    uint8_t buffer[WEB_CRASH_EXPORT_CHUNK_BYTES];
    const uint64_t start = ops->now_ms(ops->context);
    for (size_t offset = 0; offset < size;) {
        if (!ops->idle(ops->context) ||
            ops->now_ms(ops->context) - start >= WEB_CRASH_EXPORT_TIMEOUT_MS)
            return false;
        size_t bytes = size - offset;
        if (bytes > sizeof(buffer)) bytes = sizeof(buffer);
        if (!ops->read(ops->context, offset, buffer, bytes) ||
            !ops->idle(ops->context) ||
            ops->now_ms(ops->context) - start >= WEB_CRASH_EXPORT_TIMEOUT_MS ||
            !ops->send(ops->context, buffer, bytes)) return false;
        offset += bytes;
    }
    return ops->idle(ops->context) &&
           ops->now_ms(ops->context) - start < WEB_CRASH_EXPORT_TIMEOUT_MS;
}
