#include "media_io_gate.h"
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
static void pause_ms(unsigned ms) { Sleep(ms); }
#else
#include <time.h>
static void pause_ms(unsigned ms) { struct timespec t={0,(long)ms*1000000};nanosleep(&t,NULL); }
#endif
static unsigned owners;
static bool held;
static void *worker(void *arg)
{
    (void)arg;
    for (unsigned i=0;i<10000;i++) {
        media_io_gate_begin();
        assert(__atomic_add_fetch(&owners,1u,__ATOMIC_SEQ_CST)==1);
        assert(__atomic_sub_fetch(&owners,1u,__ATOMIC_SEQ_CST)==0);
        media_io_gate_end();
    }
    return NULL;
}
static void *delayed_release(void *arg) { (void)arg;media_io_gate_begin();__atomic_store_n(&held,true,__ATOMIC_RELEASE);pause_ms(50);media_io_gate_end();return NULL; }
int main(void)
{
    assert(media_io_gate_init()==ESP_OK);
    pthread_t threads[4];
    for(unsigned i=0;i<4;i++) assert(!pthread_create(&threads[i],NULL,worker,NULL));
    for(unsigned i=0;i<4;i++) assert(!pthread_join(threads[i],NULL));
    media_io_gate_begin();
    assert(!media_io_gate_try_begin(0));
    assert(!media_io_gate_try_begin(10));
    media_io_gate_end();
    assert(!pthread_create(&threads[0],NULL,delayed_release,NULL));
    while(!__atomic_load_n(&held,__ATOMIC_ACQUIRE)) pause_ms(1);
    assert(!media_io_gate_try_begin(10));
    assert(media_io_gate_try_begin(1000));
    assert(!pthread_join(threads[0],NULL));media_io_gate_end();
    puts("Host media gate four-thread exclusion, timeout and delayed acquisition PASS");
}
