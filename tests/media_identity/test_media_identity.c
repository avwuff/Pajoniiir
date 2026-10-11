#include "media_identity.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void hex(const uint8_t digest[32], char out[65])
{
    static const char digits[] = "0123456789abcdef";
    for (unsigned i=0;i<32;i++) { out[i*2]=digits[digest[i]>>4]; out[i*2+1]=digits[digest[i]&15u]; }
    out[64]='\0';
}

int main(void)
{
    uint8_t digest[32]; char actual[65];
    media_sha256("abc",3u,digest); hex(digest,actual);
    assert(strcmp(actual,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);
    media_persistent_id_t a={0}, b={0}, c={0}, d={0};
    assert(media_persistent_id_derive(digest,"/Contents/song.wav",1234u,5678,&a));
    assert(media_persistent_id_derive(digest,"/Contents/song.wav",1234u,5678,&b));
    assert(media_persistent_id_derive(digest,"/Contents/other.wav",1234u,5678,&c));
    uint8_t other_export_digest[32];
    memcpy(other_export_digest, digest, sizeof(other_export_digest));
    other_export_digest[0] ^= 0x80u;
    assert(media_persistent_id_derive(other_export_digest,"/Contents/song.wav",1234u,5678,&d));
    assert(media_persistent_id_equal(&a,&b));
    assert(!media_persistent_id_equal(&a,&c));
    /* A raw Rekordbox track ID is deliberately absent from the derivation.
     * The export digest must isolate two media even when path/stat and the raw
     * numeric track ID presented by their catalogs are otherwise identical. */
    assert(!media_persistent_id_equal(&a,&d));
    media_persistent_id_clear(&b);
    assert(!media_persistent_id_equal(&a,&b));
    char *oversized = malloc((size_t)UINT16_MAX + 2u);
    assert(oversized);
    memset(oversized, 'x', (size_t)UINT16_MAX + 1u);
    oversized[(size_t)UINT16_MAX + 1u] = '\0';
    const char *invalid_paths[] = {NULL, "", oversized};
    const uint8_t zeros[32] = {0};
    for (size_t i=0; i<3; i++) {
        b=a;
        assert(!media_persistent_id_derive(digest,invalid_paths[i],1234u,5678,&b));
        assert(!b.valid && memcmp(b.bytes,zeros,32)==0);
    }
    b=a;
    assert(!media_persistent_id_derive(NULL,"song",1234u,5678,&b));
    assert(!b.valid && memcmp(b.bytes,zeros,32)==0);
    assert(!media_persistent_id_derive(digest,"song",1234u,5678,NULL));
    /* Preserve valid derivation when its input digest aliases the output. */
    b=a;
    assert(media_persistent_id_derive(a.bytes,"song",1234u,5678,&c));
    assert(media_persistent_id_derive(b.bytes,"song",1234u,5678,&b));
    assert(media_persistent_id_equal(&b,&c));
    free(oversized);
    puts("media_identity tests passed");
    return 0;
}
